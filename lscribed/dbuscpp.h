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
   Connection( const Connection &conn ) : m_connection( conn.m_connection ) {}
   ~Connection();

   dbus_uint32_t send( const Message &msg );
   void registerHandler( const std::string &path, MessageHandler *handler, bool fallback );
   void flush() { dbus_connection_flush( m_connection ); }

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

class MessageConstIter {
public:
   MessageConstIter recurse() const;

   std::string signature() const;
   bool next() { return dbus_message_iter_next( &m_iter ); }
   bool hasNext() const { return dbus_message_iter_has_next( const_cast< DBusMessageIter *>( &m_iter ) ); }

   void *getFixedArray( int &elements ) const;
   template <typename T> T getValue() const
   {
      T rez = T();
      dbus_message_iter_get_basic( const_cast< DBusMessageIter *>( &m_iter ), &rez );
      return rez;
   }

   friend class Message;
private:
   DBusMessageIter  m_iter;
};

class Message {
public:
   explicit Message( int type );
   Message( const Message &msg );
   ~Message();

   static Message parameter( DBusMessage *msg ) { return Message( msg, false ); }

   Message newMethodReturn() const;
   Message newError( const char *error, const char *message ) const;

   static Message newSignal( const std::string &path,
                             const std::string &interface,
                             const std::string &name );

   bool isMethodCall( const char *interface, const char *method ) const
   { return dbus_message_is_method_call( m_message, interface, method ); }

   MessageConstIter constIter() const;
   MessageIter appendIter();
   void append( const std::string &str ) { append( str.c_str() ); }
   void append( const char *str );
   void append( int32_t i );

   const char *path() const;
   const char *interface() const;
   const char *member() const;

   friend class Connection;
private:
   Message( DBusMessage *msg, bool ownership );

   DBusMessage *m_message;
};


}

#endif // DBUSCPP_H
