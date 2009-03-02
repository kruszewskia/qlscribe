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

#include <lightscribe_cxx.h>
#include <fstream>
#include <iostream>

using namespace LightScribe;

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

      LS_LabelMode mode;
      LS_DrawOptions options;
      LS_PrintQuality quality;
      LS_MediaOptimizationLevel level;

      char *image = 0;
      int imageSize = 0;
      try {
         DBusCpp::MessageConstIter iter = m_message->constIter();
         if( iter.signature() != "(iiii)" )
            throw std::string( "invalid argument for params: " ) + iter.signature() + ", (iiii) expected";

         DBusCpp::MessageConstIter sub = iter.recurse();

         mode    = LS_LabelMode( sub.getValue<int32_t>() ); sub.next();
         options = LS_DrawOptions( sub.getValue<int32_t>() ); sub.next();
         quality = LS_PrintQuality( sub.getValue<int32_t>() ); sub.next();
         level   = LS_MediaOptimizationLevel( sub.getValue<int32_t>() ); sub.next();

         std::cout << "got parameters " << std::endl;

         if( !iter.next() )
            throw std::string( "image argument missing" );

         if( iter.signature() != "ay" )
            throw std::string( "invalid argument for image: " ) + iter.signature() + ", ay expected";

         sub = iter.recurse();

         image = reinterpret_cast< char * >( sub.getFixedArray( imageSize ) );
      }
      catch( const std::string &str ) {
         std::cout << "exception " << str << std::endl;
         DBusCpp::Message reply = m_message->newError( DBUS_ERROR_INVALID_ARGS, str.c_str() );
         man.connection().send( reply );
         continue;
      }
      {
         std::ofstream out( "/tmp/image.bmp" );
         out.write( image, imageSize );
      }
      DBusCpp::Message reply = m_message->newMethodReturn();
      reply.append( "/tmp/image.bmp" );
      man.connection().send( reply );
   }
}

DrivesManager::DrivesManager()
   : m_connection( 0 )
{
}

void DrivesManager::init( DBusCpp::Connection conn, bool debug )
{
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

DrivesManager::const_iterator DrivesManager::find( const std::string &path ) const
{
   const_iterator f;
   for( f = begin(); f != end(); ++f )
      if( (*f)->path() == path )
         break;

   return f;
}



