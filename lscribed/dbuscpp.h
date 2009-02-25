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

#ifndef DBUSCPP_H
#define DBUSCPP_H

#include <dbus/dbus.h>
#include <string>
#include <map>

namespace DBusCpp {

class Message;
class Connection;

class MessageHandler {
public:
   MessageHandler() : m_connection( 0 ) {}
   virtual ~MessageHandler();

   virtual DBusHandlerResult processMessage( const Message &msg ) = 0;

   Connection *connection() const { return m_connection; }

   static std::string generateInrospectHeader( const std::string &nodename = std::string() );

   friend class Connection;

private:
   Connection *m_connection;
};

class Connection {
public:
   Connection( DBusConnection *conn );
   ~Connection();

   dbus_uint32_t send( const Message &msg );
   void registerHandler( const std::string &path, MessageHandler *handler, bool fallback );

   DBusConnection *ptr() const { return m_connection; }
private:
   typedef std::map< std::string, MessageHandler * > Handlers;

   DBusConnection *m_connection;
   Handlers        m_handlers;
};

class MessageIter {
public:
   MessageIter( const MessageIter &an );
   ~MessageIter();

   MessageIter openContainer( int eemType, const char *signature );

   void append( const std::string &str ) { append( str.c_str() ); }
   void append( const char *str );
   friend class Message;
private:
   MessageIter() : m_container( 0 ) {}

   DBusMessageIter  m_iter;
   DBusMessageIter *m_container;
};

class Message {
public:
   explicit Message( int type );
   Message( const Message &msg );
   ~Message();

   static Message parameter( DBusMessage *msg ) { return Message( msg, false ); }

   Message newMethodReturn() const;
   Message newError( const char *error, const char *message ) const;

   bool isMethodCall( const char *interface, const char *method ) const
   { return dbus_message_is_method_call( m_message, interface, method ); }

   MessageIter appendIter();
   void append( const std::string &str ) { append( str.c_str() ); }
   void append( const char *str );

   const char *path() const;

   friend class Connection;
private:
   Message( DBusMessage *msg, bool ownership );

   DBusMessage *m_message;
};


}

#endif // DBUSCPP_H
