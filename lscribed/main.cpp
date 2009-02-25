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

#include "lscribed.h"
#include "dbuscpp.h"
#include "introspecthandler.h"
#include "managerhandler.h"
#include "drivehandler.h"

int main( int argc, char **argv )
{
   DBusError err;
   dbus_error_init( &err );

   DBusCpp::Connection conn ( dbus_bus_get( DBUS_BUS_SYSTEM, &err ) );
   if( dbus_error_is_set( &err ) ) {
      std::cerr << "dbus_bus_get() error: " << err.message << std::endl;
      dbus_error_free( &err );
      return 1;
   }
   int ret = dbus_bus_request_name( conn.ptr(), DBusServiceName, DBUS_NAME_FLAG_REPLACE_EXISTING, &err );
   if( dbus_error_is_set( &err ) ) {
      std::cerr << "dbus_bus_request_name error: " << err.message << std::endl;
      dbus_error_free( &err );
      return 2;
   }
   if( ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER ) {
      std::cerr << "could not get ownership of \"" << DBusServiceName << "\" terminating" << std::endl;
      return 3;
   }

   conn.registerHandler( "/", new DBusCpp::IntrospectHandler, true );
   conn.registerHandler( DBusManagerPath, new DBusCpp::ManagerHandler, false );
   conn.registerHandler( DBusDrivesPath, new DBusCpp::DriveHandler, true );

   drives.insert( std::make_pair( "drive0", "DRIVE 0" ) );
   drives.insert( std::make_pair( "drive1", "DRIVE 1" ) );

   while( dbus_connection_read_write_dispatch( conn.ptr(), -1 ) );

   return 0;
}
