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
   PrintParameters        m_parameters;
   QImage                *m_image;
   QList< QLightDrive * > m_drives;
   QTemporaryFile         m_tmpFile;
   QSize                  m_size;

   Task( Action action ) : m_action( action ), m_done( false ), m_image( 0 ) {}
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

QPixmap QLightScribe::preview( QLightDrive *drive, const PrintParameters &params, QCDScene *scene, const QSize &size )
{
   QMutexLocker lock( m_mutex );
   if( m_task )  // what should we do?
      return QPixmap();

   m_task = new Task( Task::preview );
   m_task->m_parameters = params;
   m_task->m_size = size;
   m_task->m_image = new QImage( 2772, 2772, QImage::Format_RGB888 );
   m_task->m_image->fill( 0xFFFFFFFF );

   {
      QPainter painter( m_task->m_image );
      scene->render( &painter, m_task->m_image->rect() );
   }

   m_task->m_image->setDotsPerMeterX( 23622 );
   m_task->m_image->setDotsPerMeterY( 23622 );

   m_waitQueue->wakeOne();

   while( !m_task->m_done )
      m_waitDone->wait( m_mutex );

   QPixmap pixmap( m_task->m_tmpFile.fileName(), "bmp" );
   delete m_task;
   m_task = 0;

   return pixmap;
}

void QLightScribe::print( QLightDrive *drive, const PrintParameters &params, QCDScene *scene )
{
   QMutexLocker lock( m_mutex );
   if( m_task )  // what should we do?
      return;

   m_task = new Task( Task::print );
   m_task->m_parameters = params;
   m_task->m_image = new QImage( 2772, 2772, QImage::Format_RGB888 );
   m_task->m_image->fill( 0xFFFFFFFF );

   {
      QPainter painter( m_task->m_image );
      scene->render( &painter, m_task->m_image->rect() );
   }

   m_task->m_image->setDotsPerMeterX( 23622 );
   m_task->m_image->setDotsPerMeterY( 23622 );

   m_waitQueue->wakeOne();

   while( !m_task->m_done )
      m_waitDone->wait( m_mutex );

   delete m_task;
   m_task = 0;
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

const size_t bitmapHeaderSize = 54;

void QLightScribe::run()
{
   QMutexLocker lock( m_mutex );
   while( true ) {
      while( m_task == 0 || m_task->m_done )
         m_waitQueue->wait( m_mutex );

      if( m_task->m_action == Task::stop )
         break;

      DiscPrintMgr manager;
      EnumDiscPrinters printers = manager.EnumDiscPrinters();

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

         DiscPrinter printer = printers.Item( 0 );
         DiscPrintSession session = printer.OpenPrintSession();

         if( m_task->m_action == Task::preview ) {

            LS_Size size;
            size.x = m_task->m_size.width();
            size.y = m_task->m_size.height();

            m_task->m_tmpFile.open();

            session.PrintPreview( LS_windows_bitmap,
                                  LS_label_mode_full,
                                  LS_draw_default,
                                  LS_quality_best,
                                  LS_media_recognized,
                                  ba.data() + 14,
                                  bitmapHeaderSize - 14,
                                  ba.data() + bitmapHeaderSize,
                                  ba.size() - bitmapHeaderSize,
                                  m_task->m_tmpFile.fileName().toAscii().data(),
                                  LS_windows_bitmap,
                                  size,
                                  true );
         }  else {
            // print here
         }
      }
      m_task->m_done = true;
      m_waitDone->wakeAll();
   }
}

