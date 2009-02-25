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

#include "managerhandler.h"
#include "lscribed.h"

using namespace DBusCpp;

#include <stdio.h>

static const char *strManagerIntrospect =
"<interface name=\"org.lightscribe.printManager\">\
    <method name=\"getDrives\">\
      <arg type=\"a{ss}\" direction=\"out\"/>\
    </method>\
  </interface>";


DBusHandlerResult ManagerHandler::processMessage( const Message &msg )
{
   if( msg.isMethodCall( "org.freedesktop.DBus.Introspectable", "Introspect" ) ) {
      Message reply = msg.newMethodReturn();
      std::string introspect = generateInrospectHeader();
      introspect += strManagerIntrospect;
      introspect += "</node>";
      reply.append( introspect );

      connection()->send( reply );
      return DBUS_HANDLER_RESULT_HANDLED;
   }

   if( msg.isMethodCall( "org.lightscribe.printManager", "getDrives" ) ) {
      Message reply = msg.newMethodReturn();

      MessageIter args = reply.appendIter();
      {
         MessageIter sub = args.openContainer( DBUS_TYPE_ARRAY, "{ss}" );

         for( Drives::const_iterator it = drives.begin(); it != drives.end(); ++it ) {
            MessageIter itemIterator = sub.openContainer( DBUS_TYPE_DICT_ENTRY, 0 );

            itemIterator.append( std::string( DBusDrivesPath ) + "/" + it->first );
            itemIterator.append( it->second );
         }
      }

      connection()->send( reply );
      return DBUS_HANDLER_RESULT_HANDLED;
   }

   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


