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
#include "drivehandler.h"
#include "drives.h"

using namespace DBusCpp;

static const char *strDriveIntrospect =
"<interface name=\"org.lightscribe.drive\">\
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
      <arg name=\"params\" type=\"(iiiii)\" direction=\"in\"/>\
      <arg name=\"image\" type=\"ay\" direction=\"in\"/>\
      <arg name=\"previewSize\" type=\"(ii)\" direction=\"in\"/>\
    </method>\
    <method name=\"print\">\
      <arg name=\"params\" type=\"(iiiii)\" direction=\"in\"/>\
      <arg name=\"image\" type=\"ay\" direction=\"in\"/>\
    </method>\
    <method name=\"abort\">\
    </method>\
    <method name=\"name\">\
      <arg direction=\"out\" type=\"s\"/>\
    </method>\
  </interface>";


DBusHandlerResult DriveHandler::processMessage( const Message &msg )
{
   DBusHandlerResult rez = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

   const char *path = msg.path();
   if( !path )
      return rez;

   const char *item = strrchr( path, '/' );
   if( !item )
      return rez;

   if( item[ 0 ] == '/' ) ++item;

   DrivesManager &manager = DrivesManager::instance();

   if( msg.isMethodCall( "org.freedesktop.DBus.Introspectable", "Introspect" ) ) {

      std::string introspect = generateInrospectHeader();
      if( strcmp( item, "drives" ) == 0 ) {
         for( DrivesManager::const_iterator it = manager.begin(); it != manager.end(); ++it )
            introspect += "<node name=\"" + (*it)->path() + "\"/>";
      }
      else
         introspect += strDriveIntrospect;
      introspect += "</node>";

      Message reply = msg.newMethodReturn();
      reply.append( introspect );
      connection()->send( reply );
      return rez = DBUS_HANDLER_RESULT_HANDLED;
   }

   const char *interface = msg.interface();
   if( !interface || strcmp( interface, "org.lightscribe.drive" ) )
      return rez;

   const char *member = msg.member();
   if( !member )
      return rez;

   DrivesManager::const_iterator f = manager.find( item );
   if( f == manager.end() )
      return rez;

   rez = DBUS_HANDLER_RESULT_HANDLED;
   std::cout << "calling method " << member << " on item " << item << std::endl;
   (*f)->invoke( member, msg );
   return rez;
}


