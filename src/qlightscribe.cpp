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
//#include "lightscribe_interface.h"

#include <QPainter>
#include <QBuffer>
#include <QTemporaryFile>

//#include <lightscribe_cxx.h>

//using namespace LightScribe;

QLightScribe *QLightScribe::instance()
{
   static QLightScribe inst;
   return &inst;
}

QLightScribe::QLightScribe()
{
}

QLightScribe::~QLightScribe()
{
}

QList< QLightDrive * > QLightScribe::getDrives()
{
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

QPixmap QLightDrive::preview( const PrintParameters &params, QCDScene *scene, const QSize &size ) throw( QString )
{
   return QPixmap();
}

void QLightDrive::print( const PrintParameters &params, QCDScene *scene )
{
}

void QLightDrive::abort()
{
}




/*namespace {
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
}*/
