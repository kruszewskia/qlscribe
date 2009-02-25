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

#include "introspecthandler.h"
#include "lscribed.h"

using namespace DBusCpp;

DBusHandlerResult IntrospectHandler::processMessage( const Message &msg )
{
   if( msg.isMethodCall( "org.freedesktop.DBus.Introspectable", "Introspect" ) ) {
      const char *path = msg.path();
      if( !path )
         return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

      int pathlen = strlen( path );

      std::string introspect = generateInrospectHeader();

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

      Message reply = msg.newMethodReturn();
      reply.append( introspect );
      connection()->send( reply );
      return DBUS_HANDLER_RESULT_HANDLED;
   }
   return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
