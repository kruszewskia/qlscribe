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

DBusHandlerResult processManagerMessage( DBusConnection *connPtr, DBusMessage *messagePtr, void *data );
DBusHandlerResult processDriveMessage( DBusConnection *connPtr, DBusMessage *messagePtr, void *data );

const char *introspectHeader =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\
                      \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">";

void putIntrospectInterface( std::string &str )
{
   str += "<interface name=\"org.freedesktop.DBus.Introspectable\">"
          "  <method name=\"Introspect\">"
          "    <arg name=\"data\" direction=\"out\" type=\"s\"/>"
          "  </method>"
          "</interface>";
}

DBusHandlerResult processIntrospectMessage( DBusConnection *connPtr, DBusMessage *messagePtr, void *data )
{
   using namespace DBusCpp;

   Connection conn( connPtr );
   Message message = Message::parameter( messagePtr );

   if( message.isMethodCall( "org.freedesktop.DBus.Introspectable", "Introspect" ) ) {
      const char *path = message.path();
      if( !path )
         return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

      int pathlen = strlen( path );

      std::string introspect = introspectHeader;
      introspect += "<node>";
      putIntrospectInterface( introspect );

      if( strncmp( path, DBusManagerPath, pathlen ) == 0 ) {
         if( pathlen != 1 ) ++pathlen;
         const char *end = strchr( DBusManagerPath + pathlen, '/' );
         if( end ) {
            char sub[128];
            const int sublen = end - DBusManagerPath - pathlen;
            strncpy( sub, DBusManagerPath + pathlen, sublen );
            sub[ sublen ] = 0;
            introspect += std::string( "<node name=\"" ) + sub + "\"/>";
         } else
            introspect += "<node name=\"Manager\"/><node name=\"drives\"/>";
      }
      introspect += "</node>";

      Message reply = message.newMethodReturn();
      reply.append( introspect );
      conn.send( reply );
      return DBUS_HANDLER_RESULT_HANDLED;
   }
   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int main( int argc, char **argv )
{
   DBusError err;
   dbus_error_init( &err );

   DBusConnection *conn = dbus_bus_get( DBUS_BUS_SYSTEM, &err );
   if( dbus_error_is_set( &err ) ) {
      std::cerr << "dbus_bus_get() error: " << err.message << std::endl;
      dbus_error_free( &err );
      return 1;
   }
   int ret = dbus_bus_request_name( conn, DBusServiceName, DBUS_NAME_FLAG_REPLACE_EXISTING, &err );
   if( dbus_error_is_set( &err ) ) {
      std::cerr << "dbus_bus_request_name error: " << err.message << std::endl;
      dbus_error_free( &err );
      return 2;
   }
   if( ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER ) {
      std::cerr << "could not get ownership of \"" << DBusServiceName << "\" terminating" << std::endl;
      return 3;
   }
   DBusObjectPathVTable vtable;
   ::memset( &vtable, 0, sizeof( vtable ) );

   vtable.message_function = processIntrospectMessage;
   dbus_connection_register_fallback( conn, "/", &vtable, 0 );

   vtable.message_function = processManagerMessage;
   dbus_connection_register_object_path( conn, DBusManagerPath, &vtable, 0 );

   vtable.message_function = processDriveMessage;
   dbus_connection_register_fallback( conn, DBusDrivesPath, &vtable, 0 );

   drives.insert( std::make_pair( "drive0", "DRIVE 0" ) );
   drives.insert( std::make_pair( "drive1", "DRIVE 1" ) );

   while( dbus_connection_read_write_dispatch( conn, -1 ) );

   return 0;
}
