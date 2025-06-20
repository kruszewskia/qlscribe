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
#include "lightscribe_interface.h"

#include <QApplication>
#include <QPainter>
#include <QBuffer>
#include <QTemporaryFile>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QMessageBox>

//#include <lightscribe_cxx.h>

//using namespace LightScribe;
// Marshall the PrintParameters data into a D-BUS argument
 QDBusArgument &operator<<( QDBusArgument &argument, const PrintParameters &p )
 {
     argument.beginStructure();
     argument << p.m_labelMode << p.m_drawOptions << p.m_printQuality << p.m_mediaOptimizationLevel << (int) p.m_ejectAfterComplete;
     argument.endStructure();
     return argument;
 }

 // Retrieve the PrintProperties data from the D-BUS argument
 const QDBusArgument &operator>>( const QDBusArgument &argument, PrintParameters &p )
 {
     argument.beginStructure();
     int drawOptions = 0, labelMode = 0, mediaOptimizationLevel = 0, printQuality = 0, ejectAfterComplete = 0;
     argument >> labelMode >> drawOptions >> printQuality >> mediaOptimizationLevel >> ejectAfterComplete;
     argument.endStructure();
     p = PrintParameters( LabelMode( labelMode ),
                          DrawOptions( drawOptions ),
                          PrintQuality( printQuality ),
                          MediaOptimizationLevel( mediaOptimizationLevel ),
                          ejectAfterComplete );
     return argument;
 }

QLightScribe *QLightScribe::instance()
{
   static QLightScribe inst;
   return &inst;
}

QLightScribe::QLightScribe()
   : m_managerPrx( new OrgLightscribePrintManagerInterface( DBusServiceName,
                                                            DBusManagerPath,
                                                            QDBusConnection::systemBus(),
                                                            this ) )
{
   qDBusRegisterMetaType<PrintParameters>();
   qDBusRegisterMetaType<QObject2StringMap>();
}

QLightScribe::~QLightScribe()
{
}

QList< QLightDrive * > QLightScribe::getDrives()
{
   if( m_drives.empty() ) {
      qApp->setOverrideCursor( Qt::WaitCursor );
      QDBusReply<QObject2StringMap> reply = m_managerPrx->getDrives();
      qApp->restoreOverrideCursor();
      if( !reply.isValid() ) {
         QMessageBox::critical( 0, tr( "DBus Error" ), tr( "Error on request: " ) + reply.error().message() );
      } else {
         const QObject2StringMap &map = reply.value();
         for( QObject2StringMap::const_iterator i = map.begin(); i != map.end(); ++i )
            m_drives.push_back( new QLightDrive( this, i.key(), i.value() ) );
      }
   }
   return m_drives;
}

QLightDrive::QLightDrive( QObject *parent, const QString &path, const QString &name )
   : QObject( parent ),
     m_drivePrx( new OrgLightscribeDriveInterface( DBusServiceName, path, QDBusConnection::systemBus(), this ) ),
     m_displayName( name ),
     m_path( path )
{
   connect( m_drivePrx, SIGNAL(finished(int)), this, SIGNAL(finished(int)) );
   connect( m_drivePrx, SIGNAL(prepareProgress( int, int )), this, SIGNAL(prepareProgress( int, int )) );
   connect( m_drivePrx, SIGNAL(labelProgress( int , int )), this, SIGNAL(labelProgress( int, int )) );
   connect( m_drivePrx, SIGNAL(timeEstimate( int )), this, SIGNAL(timeEstimate( int )) );
}

static
void printScene( QCDScene *scene, QByteArray &array )
{
    const int resolution = 2772;
    QImage image( resolution, resolution, QImage::Format_RGB888 );
    image.fill( 0xFFFFFFFF );

    scene->clearSelection();
    {
        QPainter painter( &image );
        scene->render( &painter, image.rect() );
    }
    image.setDotsPerMeterX( resolution * 8 );
    image.setDotsPerMeterY( resolution * 8 );

    array.clear();
    QBuffer buffer( &array );
    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer, "bmp", 100 );
}

QPixmap QLightDrive::preview( const PrintParameters &params, QCDScene *scene, const QSize &size ) noexcept
{
   qApp->setOverrideCursor( Qt::WaitCursor );
   QByteArray array;
   printScene( scene, array );

   /*QList<QVariant> argumentList;
   argumentList << qVariantFromValue( params ) << qVariantFromValue( array )
                << qVariantFromValue( size );
   QDBusReply<QString> reply = m_drivePrx->callWithArgumentList( QDBus::BlockWithGui,
                                                                 QLatin1String( "preview" ),
                                                                 argumentList);*/
   QDBusReply<QString> reply = m_drivePrx->preview( params, array, size );
   qApp->restoreOverrideCursor();
   if( !reply.isValid() )
      throw QString( "DBus error: (%1) %2" ).arg( reply.error().name(), reply.error().message() );

   QString fname = reply.value();
   QPixmap pixmap( fname );
   QFile( fname ).remove();
   return pixmap;
}

void QLightDrive::print( const PrintParameters &params, QCDScene *scene ) noexcept
{
   qApp->setOverrideCursor( Qt::WaitCursor );
   QByteArray array;
   printScene( scene, array );
   /*QList<QVariant> argumentList;
   argumentList << qVariantFromValue( params ) << qVariantFromValue( array );
   QDBusReply<void> reply =  callWithArgumentList( QDBus::BlockWithGui,
                                                   QLatin1String( "print" ),
                                                   argumentList);*/
   QDBusReply<void> reply = m_drivePrx->print( params, array );
   qApp->restoreOverrideCursor();
   if( !reply.isValid() )
      throw QString( "DBus error: (%1) %2" ).arg( reply.error().name(), reply.error().message() );
}

void QLightDrive::abort()
{
   m_drivePrx->abort();
}
