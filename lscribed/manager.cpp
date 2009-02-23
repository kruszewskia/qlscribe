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

#include "dbuscpp.h"
#include "lscribed.h"

#include <stdio.h>

static const char *strManagerIntrospect =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\
                      \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\
<node>\
  <interface name=\"org.freedesktop.DBus.Introspectable\">\
    <method name=\"Introspect\">\
      <arg name=\"data\" direction=\"out\" type=\"s\"/>\
    </method>\
  </interface>\
  <interface name=\"org.lightscribe.printManager\">\
    <method name=\"getDrives\">\
      <arg type=\"a{ss}\" direction=\"out\"/>\
    </method>\
  </interface>\
</node>";


DBusHandlerResult processManagerMessage( DBusConnection *connPtr, DBusMessage *messagePtr, void *data )
{
   using namespace DBusCpp;

   Connection conn( connPtr );
   Message message = Message::parameter( messagePtr );

   /*std::cout << "message received"
             << "type" << dbus_message_get_type (message)
             << " path "<< ( dbus_message_get_path (message) ?  dbus_message_get_path (message) : "no path" )
             << " interface " << ( dbus_message_get_interface (message) ?  dbus_message_get_interface (message) : "no interface" )
             << " member " << ( dbus_message_get_member (message) ? dbus_message_get_member (message) : "no member" )
             << " destination " << ( dbus_message_get_destination (message) ? dbus_message_get_destination (message) : "null" )<< std::endl;*/


   if( message.isMethodCall( "org.freedesktop.DBus.Introspectable", "Introspect" ) ) {
      Message reply = message.newMethodReturn();
      reply.append( strManagerIntrospect );
      conn.send( reply );
      return DBUS_HANDLER_RESULT_HANDLED;
   }

   if( message.isMethodCall( "org.lightscribe.printManager", "getDrives" ) ) {
      Message reply = message.newMethodReturn();

      MessageIter args = reply.appendIter();
      {
         MessageIter sub = args.openContainer( DBUS_TYPE_ARRAY, "{ss}" );

         for( Drives::const_iterator it = drives.begin(); it != drives.end(); ++it ) {
            MessageIter itemIterator = sub.openContainer( DBUS_TYPE_DICT_ENTRY, 0 );

            itemIterator.append( std::string( DBusDrivesPath ) + "/" + it->first );
            itemIterator.append( it->second );
         }
      }

      conn.send( reply );
      return DBUS_HANDLER_RESULT_HANDLED;
   }

   //Message reply = message.newError( DBUS_ERROR_NOT_SUPPORTED, "method is not supported" );
   //conn.send( reply );
   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


