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

#include <iostream>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "lscribed.h"
#include "dbuscpp.h"
#include "introspecthandler.h"
#include "managerhandler.h"
#include "drivehandler.h"
#include "drives.h"

unsigned int lastActivity = 0;
unsigned int activityTimeout = 300; // 5 min by default
bool terminateOnTimeout = false;
int alarmInterval = 1;
bool debug = false;

void onAlarm( int )
{
   alarm( alarmInterval );
   unsigned int curr = time( 0 );

   if( lastActivity == 0 ) {
      lastActivity = curr;
      return;
   }
   if( curr - lastActivity < activityTimeout )
      return;

   if( DrivesManager::instance().active() ) {
      lastActivity = curr;
      return;
   }
   syslog( LOG_INFO, "terminating due to inactivity timeout" );
   terminateOnTimeout = true;
}

void usage()
{
   std::cerr << "usage: lscribed [--help] [--timeout t] [--debug]" << std::endl;
   std::cerr << "\t--help     print this message" << std::endl;
   std::cerr << "\t--timeout  interval (seconds) to terminate after inactivity 0 - disable, default 300" << std::endl;
   std::cerr << "\t--debug    to enable virtual drive and print debug statements" << std::endl;
}

int main( int argc, char **argv )
{
   openlog( "lscribed", LOG_PID, LOG_DAEMON );
   for( int i = 1; i < argc; ++i ) {
      if( strcmp( argv[i], "--help" ) == 0 ) {
         usage();
         return 1;
      }

      if( strcmp( argv[i], "--timeout" ) == 0 ) {
         if( i == argc - 1 ) {
            std::cerr << "Error: agrument for --timeout expected" << std::endl;
            usage();
            return 2;
         }
         activityTimeout = atoi( argv[ ++ i ] );
         continue;
      }
      if( strcmp( argv[i], "--debug" ) == 0 ) {
         debug = true;
         continue;
      }
      std::cerr << "Error: unknown parameter " << argv[ i ] << std::endl;
      usage();
      return 3;
   }
   dbus_threads_init_default();

   DBusError err;
   dbus_error_init( &err );

   DBusCpp::Connection conn ( dbus_bus_get_private( DBUS_BUS_SYSTEM, &err ) );
   if( dbus_error_is_set( &err ) ) {
      std::cerr << "dbus_bus_get() error: " << err.message << std::endl;
      dbus_error_free( &err );
      return 1;
   }
   int ret = dbus_bus_request_name( conn.ptr(), DBusServiceName, DBUS_NAME_FLAG_REPLACE_EXISTING, &err );
   if( dbus_error_is_set( &err ) ) {
      std::cerr << "dbus_bus_request_name error: " << err.message << std::endl;
      syslog( LOG_ERR, "dbus_bus_request_name error: %s", err.message );
      dbus_error_free( &err );
      return 2;
   }
   if( ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER ) {
      std::cerr << "could not get ownership of \"" << DBusServiceName << "\" terminating" << std::endl;
      syslog( LOG_ERR, "could not get ownership of \"%s\" terminating", DBusServiceName );
      return 3;
   }

   conn.registerHandler( "/", new DBusCpp::IntrospectHandler, true );
   conn.registerHandler( DBusManagerPath, new DBusCpp::ManagerHandler, false );
   conn.registerHandler( DBusDrivesPath, new DBusCpp::DriveHandler, true );

   DrivesManager::instance().init( conn, debug );
   if( activityTimeout ) {
      signal( SIGALRM, onAlarm );
      alarm( alarmInterval );
   }

   while( dbus_connection_read_write_dispatch( conn.ptr(), 10 ) ) {
      if( terminateOnTimeout )
         dbus_connection_close( conn.ptr() );
   }

   return 0;
}
