/*  qlscribe - Qt based application to print lightScribe discs

    Copyright (C) 2009 Vyacheslav Kononenko <vyacheslav@kononenko.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    $Id$ */

#include "qlightscribe.h"
#include "qcdscene.h"

#include <QMutex>
#include <QWaitCondition>
#include <QPainter>
#include <QBuffer>
#include <QTemporaryFile>

#include <lightscribe_cxx.h>

using namespace LightScribe;

struct QLightScribe::Task {
   enum Action { getDrives, preview, print, stop };
   Action                 m_action;
   bool                   m_done;
   QLightDrive           *m_selectedDrive;
   PrintParameters        m_parameters;
   QSize                  m_size;
   QImage                *m_image;

   QList< QLightDrive * > m_drives;
   QTemporaryFile         m_tmpFile;
   QString                m_error;

   Task( Action action ) : m_action( action ), m_done( false ), m_selectedDrive( 0 ), m_image( 0 ) {}
   ~Task() { delete m_image; }
};

QLightScribe *QLightScribe::instance()
{
   static QLightScribe inst;
   return &inst;
}

QLightScribe::QLightScribe()
   : m_task( 0 ),
   m_aborted( false ),
   m_mutex( new QMutex ),
   m_waitQueue( new QWaitCondition ),
   m_waitDone( new QWaitCondition )
{
   start();
}

QLightScribe::~QLightScribe()
{
   delete m_waitDone;
   delete m_waitQueue;
   delete m_mutex;
}

QList< QLightDrive * > QLightScribe::getDrives( bool refresh )
{
   if( m_drives.empty() || refresh ) {
      QMutexLocker lock( m_mutex );
      if( m_task )  // what should we do?
         return m_drives;

      m_task = new Task( Task::getDrives );
      m_waitQueue->wakeOne();

      while( !m_task->m_done )
         m_waitDone->wait( m_mutex );

      while( m_drives.count() ) {
         delete m_drives.front();
         m_drives.erase( m_drives.begin() );
      }
      m_drives = m_task->m_drives;

      delete m_task;
      m_task = 0;
   }
   return m_drives;
}

static
QImage *printScene( QCDScene *scene )
{
   QImage *image = new QImage( 2772, 2772, QImage::Format_RGB888 );
   image->fill( 0xFFFFFFFF );

   scene->clearSelection();
   {
      QPainter painter( image );
      scene->render( &painter, image->rect() );
   }
   image->setDotsPerMeterX( 23622 );
   image->setDotsPerMeterY( 23622 );

   return image;
}

QPixmap QLightScribe::preview( QLightDrive *drive, const PrintParameters &params, QCDScene *scene, const QSize &size ) throw( QString )
{
   QMutexLocker lock( m_mutex );
   if( m_task )  // what should we do?
      return QPixmap();

   m_task = new Task( Task::preview );
   m_task->m_selectedDrive = drive;
   m_task->m_parameters = params;
   m_task->m_size = size;
   m_task->m_image = printScene( scene );

   m_aborted = false;

   m_waitQueue->wakeOne();

   while( !m_task->m_done )
      m_waitDone->wait( m_mutex );

   QString err = m_task->m_error;
   QPixmap pixmap;
   if( err.isEmpty() )
      pixmap.load( m_task->m_tmpFile.fileName(), "bmp" );

   delete m_task;
   m_task = 0;

   if( !err.isEmpty() )
      throw err;

   return pixmap;
}

void QLightScribe::print( QLightDrive *drive, const PrintParameters &params, QCDScene *scene )
{
   QMutexLocker lock( m_mutex );
   if( m_task )  // what should we do?
      return;

   m_task = new Task( Task::print );
   m_task->m_selectedDrive = drive;
   m_task->m_parameters = params;
   m_task->m_image = printScene( scene );

   m_aborted = false;

   m_waitQueue->wakeOne();
}

void QLightScribe::abort()
{
   m_aborted = true;
}

void QLightScribe::stopThread()
{
   QMutexLocker lock( m_mutex );
   delete m_task;
   m_task = new Task( Task::stop );
   m_waitQueue->wakeOne();
}

bool QLightScribe::clAbortLabel()
{
   return QLightScribe::instance()->m_aborted;
}

void QLightScribe::clReportPrepareProgress(long current, long final)
{
   emit instance()->prepareProgress( current, final );
}

void QLightScribe::clReportLabelProgress(long current, long final)
{
   emit instance()->labelProgress( current, final );
}

void QLightScribe::clReportFinished(LSError status)
{
   emit instance()->finished( status );
}

bool QLightScribe::clReportLabelTimeEstimate(long time)
{
   emit instance()->timeEstimate( time );
   return false;
}



namespace {
   class AutoUidSetter {
   public:
      AutoUidSetter() : m_saved( 0 ) {}
      AutoUidSetter( uid_t userId ) : m_saved( 0 ) { set( userId ); }
      ~AutoUidSetter() { unset(); }

      void set( uid_t userId )
      {
         if( userId ) {
            setreuid( userId, 0 );
            m_saved = userId;
         }
      }
      void unset()
      {
         if( m_saved ) {
            setreuid( 0, m_saved );
            m_saved = 0;
         }
      }

   private:
      uid_t m_saved;
   };

   typedef void (LightScribe::DiscPrinter::*DiscPrinterOp)();

   template<DiscPrinterOp dlock, DiscPrinterOp dunlock >
         class AutoOp {
   public:
      AutoOp( LightScribe::DiscPrinter &printer ) : m_printer( &printer ), m_locked( false ) {}
      AutoOp() { unlock(); }

      void lock()
      {
         if( !m_locked ) {
            (m_printer->*dlock)();
            m_locked = true;
         }
      }

      void unlock()
      {
         if( m_locked ) {
            try {
               (m_printer->*dunlock)();
            }
            catch(...){}
            m_locked = false;
         }
      }

   private:
      LightScribe::DiscPrinter *m_printer;
      bool                      m_locked;
   };
}

extern uid_t realUserId;

void QLightScribe::run()
{
   QMutexLocker lock( m_mutex );
   while( true ) {
      while( m_task == 0 || m_task->m_done )
         m_waitQueue->wait( m_mutex );

      if( m_task->m_action == Task::stop )
         break;

      QString function;
      try {
         function = "LS_DiscPrintMgr_Create";
         DiscPrintMgr manager;

         function = "LS_DiscPrintMgr_EnumDiscPrinters";
         EnumDiscPrinters printers = manager.EnumDiscPrinters();

         function = "";

         if( m_task->m_action == Task::getDrives ) {
            const unsigned count = printers.Count();

            for( unsigned i = 0; i < count; ++i ) {
               DiscPrinter printer = printers.Item( i );

               QLightDrive *drive = new QLightDrive;
               drive->m_displayName = QString::fromStdString( printer.GetPrinterDisplayName() );
               drive->m_productName = QString::fromStdString( printer.GetPrinterProductName() );
               drive->m_vendorName  = QString::fromStdString( printer.GetPrinterVendorName() );
               drive->m_path        = QString::fromStdString( printer.GetPrinterPath() );
               drive->m_innerRadius = printer.GetDriveInnerRadius() / 1000.0;
               drive->m_outerRadius = printer.GetDriveOuterRadius() / 1000.0;

               m_task->m_drives.append( drive );
            }
         } else {

            QByteArray ba;
            QBuffer buffer( &ba );
            buffer.open( QIODevice::WriteOnly );
            m_task->m_image->save( &buffer, "bmp", 100 );

            int found = -1;
            for( size_t i = 0; i < printers.Count(); ++i )
               if( printers.Item( i ).GetPrinterDisplayName() == m_task->m_selectedDrive->displayName().toStdString() ) {
               found = i;
               break;
            }
            if( found == -1 )
               throw tr( "Cannot find drive: \"" ) + m_task->m_selectedDrive->displayName() + "\"";

            DiscPrinter printer = printers.Item( found );

            AutoUidSetter autoUid;
            AutoOp< &DiscPrinter::AddExclusiveUse, &DiscPrinter::ReleaseExclusiveUse > autoExclusive( printer );
            AutoOp< &DiscPrinter::LockDriveTray, &DiscPrinter::UnlockDriveTray > autoLock( printer );

            if( m_task->m_action == Task::print ) {
               autoUid.set( realUserId );
               function = "LS_DiscPrinter_AddExclusiveUse";
               autoExclusive.lock();
               function = "LS_DiscPrinter_LockDriveTray";
               autoLock.lock();
            }

            {
               function = "LS_DiscPrinter_OpenPrintSession";
               DiscPrintSession session = printer.OpenPrintSession();

               LS_PrintCallbacks callbacks;
               callbacks.AbortLabel = clAbortLabel;
               callbacks.ReportPrepareProgress = clReportPrepareProgress;
               callbacks.ReportLabelProgress = clReportLabelProgress;
               callbacks.ReportFinished = clReportFinished;
               callbacks.ReportLabelTimeEstimate = clReportLabelTimeEstimate;

               session.SetProgressCallback( &callbacks );

               const size_t bitmapHeaderSize = 54;

               if( m_task->m_action == Task::preview ) {

                  LS_Size size;
                  size.x = m_task->m_size.width();
                  size.y = m_task->m_size.height();

                  m_task->m_tmpFile.open();

                  function = "LS_DiscPrintSession_PrintPreview";
                  session.PrintPreview( LS_windows_bitmap,
                                        LS_LabelMode( m_task->m_parameters.m_labelMode ),
                                        LS_DrawOptions( m_task->m_parameters.m_drawOptions ),
                                        LS_PrintQuality( m_task->m_parameters.m_printQuality ),
                                        LS_MediaOptimizationLevel( m_task->m_parameters.m_mediaOptimizationLevel ),
                                        ba.data() + 14,
                                        bitmapHeaderSize - 14,
                                        ba.data() + bitmapHeaderSize,
                                        ba.size() - bitmapHeaderSize,
                                        m_task->m_tmpFile.fileName().toAscii().data(),
                                        LS_windows_bitmap,
                                        size,
                                        true );
               }  else {
                  PrintParameters params = m_task->m_parameters;
                  delete m_task;
                  m_task = 0;
                  lock.unlock();
                  // print here
                  session.PrintDisc( LS_windows_bitmap,
                                     LS_LabelMode( params.m_labelMode ),
                                     LS_DrawOptions( params.m_drawOptions ),
                                     LS_PrintQuality( params.m_printQuality ),
                                     LS_MediaOptimizationLevel( params.m_mediaOptimizationLevel ),
                                     ba.data() + 14,
                                     bitmapHeaderSize - 14,
                                     ba.data() + bitmapHeaderSize,
                                     ba.size() - bitmapHeaderSize );
               }
            }

            function = "";
         }
      }
      catch( LightScribe::LSException &ex ) {
         if( m_task )
            m_task->m_error = function + tr( " failed with code 0x" ) + QString::number( ex.GetCode(), 16 );
      }
      catch( const QString &err ) {
         if( m_task )
            m_task->m_error = err;
      }
      if( m_task ) {
         m_task->m_done = true;
         m_waitDone->wakeAll();
      }
   }
}
