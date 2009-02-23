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

#include "dbuscpp.h"
#include "lscribed.h"

Drives drives;

static const char *strDriveIntrospect =
"<node>\
  <interface name=\"org.freedesktop.DBus.Introspectable\">\
    <method name=\"Introspect\">\
      <arg name=\"data\" direction=\"out\" type=\"s\"/>\
    </method>\
  </interface>\
  <interface name=\"org.lightscribe.drive\">\
    <signal name=\"prepareProgress\">\
      <arg name=\"current\" type=\"i\" direction=\"out\"/>\
      <arg name=\"final\" type=\"i\" direction=\"out\"/>\
    </signal>\
    <signal name=\"labelProgress\">\
      <arg name=\"current\" type=\"i\" direction=\"out\"/>\
      <arg name=\"final\" type=\"i\" direction=\"out\"/>\
    </signal>\
    <signal name=\"timeEstimate\">\
      <arg name=\"time\" type=\"i\" direction=\"out\"/>\
    </signal>\
    <signal name=\"finished\">\
      <arg name=\"status\" type=\"i\" direction=\"out\"/>\
    </signal>\
    <method name=\"preview\">\
      <arg type=\"s\" direction=\"out\"/>\
      <arg name=\"params\" type=\"(iiii)\" direction=\"in\"/>\
      <arg name=\"image\" type=\"ay\" direction=\"in\"/>\
      <arg name=\"previewSize\" type=\"(ii)\" direction=\"in\"/>\
    </method>\
    <method name=\"print\">\
      <arg name=\"params\" type=\"(iiii)\" direction=\"in\"/>\
      <arg name=\"image\" type=\"ay\" direction=\"in\"/>\
    </method>\
    <method name=\"abort\">\
    </method>\
    <method name=\"name\">\
      <arg direction=\"out\" type=\"s\"/>\
    </method>\
  </interface>\
</node>";


DBusHandlerResult processDriveMessage( DBusConnection *connPtr, DBusMessage *messagePtr, void *data )
{
   DBusHandlerResult rez = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
   using namespace DBusCpp;

   Connection conn( connPtr );
   Message message = Message::parameter( messagePtr );

   const char *path = message.path();
   if( !path )
      return rez;

   const char *item = strrchr( path, '/' );
   if( !item )
      return rez;

   if( item[ 0 ] == '/' ) ++item;

   if( message.isMethodCall( "org.freedesktop.DBus.Introspectable", "Introspect" ) ) {


      std::string introspect = introspectHeader;
      if( strcmp( item, "drives" ) == 0 ) {
         introspect += "<node>";
         putIntrospectInterface( introspect );
         for( Drives::const_iterator it = drives.begin(); it != drives.end(); ++it )
            introspect += "<node name=\"" + it->first + "\"/>";
         introspect += "</node>";
      }
      else
         introspect += strDriveIntrospect;

      Message reply = message.newMethodReturn();
      reply.append( introspect );
      conn.send( reply );
      return rez = DBUS_HANDLER_RESULT_HANDLED;
   }

   Drives::const_iterator f = drives.find( item );
   if( f == drives.end() )
      return rez;

   std::cout << "calling method on item " << item << std::endl;
   if( message.isMethodCall( "org.lightscribe.drive", "name" ) ) {
      Message reply = message.newMethodReturn();
      reply.append( f->second );
      conn.send( reply );
      return rez = DBUS_HANDLER_RESULT_HANDLED;
   }
   return rez;
}


