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

#include "drives.h"
#include "lscribed.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <lightscribe_cxx.h>
#include <fstream>
#include <iostream>


using namespace LightScribe;

static pthread_key_t drivesKey;

static Drive *getDriveForThread()
{
   return reinterpret_cast<Drive *>( pthread_getspecific( drivesKey ) );
}

static bool clAbortLabel()
{
   Drive *drive = getDriveForThread();
   return drive ? drive->isAborted() : true;
}

static void clReportPrepareProgress( long current, long final )
{
   Drive *drive = getDriveForThread();
   if( drive ) {
      DBusCpp::Message msg = DBusCpp::Message::newSignal( drive->fullPath(),
                                                          "org.lightscribe.drive",
                                                          "prepareProgress" );
      msg.append( current );
      msg.append( final );
      DrivesManager::instance().connection().send( msg );
   }
}

static void clReportLabelProgress( long current, long final )
{
   Drive *drive = getDriveForThread();
   if( drive ) {
      DBusCpp::Message msg = DBusCpp::Message::newSignal( drive->fullPath(),
                                                          "org.lightscribe.drive",
                                                          "labelProgress" );
      msg.append( current );
      msg.append( final );
      DrivesManager::instance().connection().send( msg );
   }
}

void clReportFinished( LSError status )
{
   Drive *drive = getDriveForThread();
   if( drive ) {
      DBusCpp::Message msg = DBusCpp::Message::newSignal( drive->fullPath(),
                                                          "org.lightscribe.drive",
                                                          "finished" );
      msg.append( status );
      DrivesManager::instance().connection().send( msg );
   }
}

static bool clReportLabelTimeEstimate( long time )
{
   Drive *drive = getDriveForThread();
   if( drive ) {
      DBusCpp::Message msg = DBusCpp::Message::newSignal( drive->fullPath(),
                                                          "org.lightscribe.drive",
                                                          "timeEstimate" );
      msg.append( time );
      DrivesManager::instance().connection().send( msg );
   }
   return false;
}

static std::string generateTmp()
{
   char *temp = tempnam( 0, ".bmp" );
   std::string rez( temp );
   free( temp );
   return rez;
}

Drive::Drive( int index, const std::string &path, const std::string &name )
   : m_index( index ), m_path( path ), m_name( name ), m_threadStarted( false ), m_message( 0 )
{
}

Drive::~Drive()
{
   if( m_threadStarted ) {
      pthread_cancel( m_thread );
      pthread_join( m_thread, 0 );
      pthread_cond_destroy( &m_cond );
   }
}

std::string Drive::fullPath() const
{
   return std::string( DBusDrivesPath ) + "/" + path();
}

bool Drive::active() const
{
   return m_message != 0;
}

void *routine( void *ptr )
{
   reinterpret_cast< Drive * >( ptr )->routine();
   return ptr;
}


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void Drive::invoke( const std::string &method, const DBusCpp::Message &msg )
{
   DrivesManager &man = DrivesManager::instance();
   if( method == "name" ) {
      DBusCpp::Message reply = msg.newMethodReturn();
      reply.append( name() );
      man.connection().send( reply );
      return;
   }
   if( method == "abort" ) {
      m_aborted = true;
      DBusCpp::Message reply = msg.newMethodReturn();
      man.connection().send( reply );
      return;
   }
   if( method == "preview" || method == "print" ) {
      if( !m_threadStarted ) {
         m_threadStarted = true;
         pthread_cond_init( &m_cond, 0 );
         pthread_create( &m_thread, 0, ::routine, this );
      }
      pthread_mutex_lock( &mutex );
      if( m_message ) {
         DBusCpp::Message reply = msg.newError( DBUS_ERROR_FAILED, "drive is busy, try again later" );
         man.connection().send( reply );
      }
      m_aborted = false;
      m_message = new DBusCpp::Message( msg );
      pthread_cond_signal( &m_cond );
      pthread_mutex_unlock( &mutex );
      return;
   }
   DBusCpp::Message reply = msg.newError( DBUS_ERROR_UNKNOWN_METHOD, "method is not supported" );
   man.connection().send( reply );
}

void Drive::routine()
{
   pthread_setspecific( drivesKey, this );
   DrivesManager &man = DrivesManager::instance();

   bool first = true;
   while( true ) {
      pthread_mutex_lock( &mutex );

      if( first )
         first = false;
      else {
         delete m_message;
         m_message = 0;
      }
      while( !m_message )
         pthread_cond_wait( &m_cond, &mutex );

      pthread_mutex_unlock( &mutex );

      bool preview = strcmp( m_message->member(), "preview" ) == 0;

      LS_LabelMode mode;
      LS_DrawOptions options;
      LS_PrintQuality quality;
      LS_MediaOptimizationLevel level;
      bool eject= false;

      char *image = 0;
      int imageSize = 0;

      LS_Size size;

      try {
         DBusCpp::MessageConstIter iter = m_message->constIter();
         if( iter.signature() != "(iiiii)" )
            throw std::string( "invalid argument for params: " ) + iter.signature() + ", (iiiii) expected";

         DBusCpp::MessageConstIter sub = iter.recurse();

         mode    = LS_LabelMode( sub.getValue<int32_t>() ); sub.next();
         options = LS_DrawOptions( sub.getValue<int32_t>() ); sub.next();
         quality = LS_PrintQuality( sub.getValue<int32_t>() ); sub.next();
         level   = LS_MediaOptimizationLevel( sub.getValue<int32_t>() ); sub.next();
         eject   = sub.getValue<int32_t>();

         if( !iter.next() )
            throw std::string( "image argument missing" );

         if( iter.signature() != "ay" )
            throw std::string( "invalid argument for image: " ) + iter.signature() + ", ay expected";

         sub = iter.recurse();

         image = reinterpret_cast< char * >( sub.getFixedArray( imageSize ) );

         if( preview ) {
            if( !iter.next() )
               throw std::string( "size argument missing" );
            if( iter.signature() != "(ii)" )
               throw std::string( "invalid argument for size: " ) + iter.signature() + ", (ii) expected";

            sub = iter.recurse();
            size.x = sub.getValue<int32_t>(); sub.next();
            size.y = sub.getValue<int32_t>();
         }
      }
      catch( const std::string &str ) {
         std::cout << "exception " << str << std::endl;
         DBusCpp::Message reply = m_message->newError( DBUS_ERROR_INVALID_ARGS, str.c_str() );
         man.connection().send( reply );
         syslog( LOG_ERR, "error in parsing request: %s", str.c_str() );
         continue;
      }

      std::string tmpFile;
      if( preview )
         tmpFile = generateTmp();

      if( m_index == -1 ) {
         DBusCpp::Message reply = m_message->newMethodReturn();

         if( preview ) { // this is debug drive, so just write input into output
            std::ofstream out( tmpFile.c_str() );
            out.write( image, imageSize );
            out.close();

            reply.append( tmpFile );
         }
         man.connection().send( reply );
         continue;
      }

      const char *function = 0;
      try {
         const size_t bitmapHeaderSize = 54;

         if( size_t( imageSize ) <= bitmapHeaderSize )
            throw "Invalid image";

         function = "LS_DiscPrintMgr_Create";
         DiscPrintMgr manager;

         function = "LS_DiscPrintMgr_EnumDiscPrinters";
         EnumDiscPrinters printers = manager.EnumDiscPrinters();
         if( m_index >= int( printers.Count() ) )
            throw std::string( "internal error: invalid index" );

         DiscPrinter printer = printers.Item( m_index );

         {
            function = "LS_DiscPrinter_OpenPrintSession";
            DiscPrintSession session = printer.OpenPrintSession();

            DBusCpp::Message reply = m_message->newMethodReturn();

            if( preview ) {

               syslog( LOG_INFO, "generating preview into temp file %s", tmpFile.c_str() );
               function = "LS_DiscPrintSession_PrintPreview";
               session.PrintPreview( LS_windows_bitmap,
                                     mode,
                                     options,
                                     quality,
                                     level,
                                     image + 14,
                                     bitmapHeaderSize - 14,
                                     image + bitmapHeaderSize,
                                     imageSize - bitmapHeaderSize,
                                     const_cast< char *>( tmpFile.c_str() ),
                                     LS_windows_bitmap,
                                     size,
                                     true );
               reply.append( tmpFile );
               man.connection().send( reply );

            } else {

               syslog( LOG_INFO, "printing label with parameters: mode %d, options %d, quality %d, level %d, eject %d",
                       mode, options, quality, level, eject );

               LS_PrintCallbacks callbacks;
               callbacks.AbortLabel = clAbortLabel;
               callbacks.ReportPrepareProgress = clReportPrepareProgress;
               callbacks.ReportLabelProgress = clReportLabelProgress;
               callbacks.ReportFinished = clReportFinished;
               callbacks.ReportLabelTimeEstimate = clReportLabelTimeEstimate;
               session.SetProgressCallback( &callbacks );

               man.connection().send( reply );

               function = 0;
               session.PrintDisc( LS_windows_bitmap,
                                  mode,
                                  options,
                                  quality,
                                  level,
                                  image + 14,
                                  bitmapHeaderSize - 14,
                                  image + bitmapHeaderSize,
                                  imageSize - bitmapHeaderSize );
            }
         }
         if( !preview && eject )
            printer.OpenDriveTray();
      }
      catch( LightScribe::LSException &ex ) {
         if( function ) {
            char message[ 255 ];
            sprintf( message, "\"%s()\" failed with code 0x%X", function, ex.GetCode() );
            DBusCpp::Message reply = m_message->newError( DBUS_ERROR_FAILED, message );
            man.connection().send( reply );
            syslog( LOG_ERR, "%s", message );
         } else
            clReportFinished( ex.GetCode() );
         continue;
      }
      catch( const std::string &str ) {
         DBusCpp::Message reply = m_message->newError( DBUS_ERROR_FAILED, str.c_str() );
         man.connection().send( reply );
         syslog( LOG_ERR, "operation failed: \"%s\"", str.c_str() );
         continue;
      }
   }
}

DrivesManager::DrivesManager()
   : m_connection( 0 )
{
}

void DrivesManager::init( DBusCpp::Connection conn, bool debug )
{
   pthread_key_create( &drivesKey, 0 );
   m_connection = conn;

   if( debug )
      m_drives.push_back( new Drive( -1, "drived", "Debug printer" ) );

   DiscPrintMgr manager;
   EnumDiscPrinters printers = manager.EnumDiscPrinters();
   const unsigned count = printers.Count();
   for( unsigned i = 0; i < count; ++i ) {
      DiscPrinter printer = printers.Item( i );

      char path[ 10 ];
      sprintf( path, "drive%d", i );
      m_drives.push_back( new Drive( i, path, printer.GetPrinterDisplayName() ) );
   }
}


DrivesManager::~DrivesManager()
{
   for( const_iterator f = begin(); f != end(); ++f )
      delete *f;
}

DrivesManager &DrivesManager::instance()
{
   static DrivesManager theManager;
   return theManager;

}

bool DrivesManager::active() const
{
   for( const_iterator f = begin(); f != end(); ++f )
      if( (*f)->active() )
         return true;

   return false;
}

DrivesManager::const_iterator DrivesManager::find( const std::string &path ) const
{
   const_iterator f;
   for( f = begin(); f != end(); ++f )
      if( (*f)->path() == path )
         break;

   return f;
}



