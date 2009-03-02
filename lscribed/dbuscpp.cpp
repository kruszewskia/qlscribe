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

using namespace DBusCpp;

MessageHandler::~MessageHandler()
{
}

std::string MessageHandler::generateInrospectHeader( const std::string &nodename )
{
   std::string rez = "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
                     "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">";

   rez += " <node";
   if( !nodename.empty() )
      rez += "=\"" + nodename + "\"";
   rez += "> <interface name=\"org.freedesktop.DBus.Introspectable\">"
          "  <method name=\"Introspect\">"
          "    <arg name=\"data\" direction=\"out\" type=\"s\"/>"
          "  </method>"
          "</interface>";
   return rez;
}

namespace {
   DBusObjectPathVTable vtable;
   DBusHandlerResult processMessage( DBusConnection *connPtr,
                                     DBusMessage *messagePtr,
                                     void *data )
   {
      Message message = Message::parameter( messagePtr );
      return reinterpret_cast< MessageHandler * >( data )->processMessage( message );
   }
 }

Connection::Connection( DBusConnection *conn )
   : m_connection( conn )
{
   if( !vtable.message_function )
      vtable.message_function = processMessage;
}

Connection::~Connection()
{
   for( Handlers::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it ) {
      dbus_connection_unregister_object_path( m_connection, it->first.c_str() );
      delete it->second;
   }
}

dbus_uint32_t Connection::send( const Message &msg )
{
   dbus_uint32_t rez;
   dbus_connection_send( m_connection, msg.m_message, &rez );
   return rez;
}

void Connection::registerHandler( const std::string &path, MessageHandler *handler, bool fallback )
{
   handler->m_connection = this;
   m_handlers.insert( std::make_pair( path, handler ) );
   if( fallback )
      dbus_connection_register_fallback( m_connection, path.c_str(), &vtable, handler );
   else
      dbus_connection_register_object_path( m_connection, path.c_str(), &vtable, handler );
}


MessageIter::MessageIter( const MessageIter &an )
   : m_iter( an.m_iter ), m_container( an.m_container )
{
   if( m_container )
      const_cast< MessageIter & >( an ).m_container = 0;
}

MessageIter::~MessageIter()
{
   if( m_container )
      dbus_message_iter_close_container( m_container, &m_iter );
}

MessageIter MessageIter::openContainer( int elemType, const char *signature )
{
   MessageIter rez;
   dbus_message_iter_open_container( &m_iter, elemType, signature, &rez.m_iter );
   rez.m_container = &m_iter;

   return rez;
}

void MessageIter::append( const char *str )
{
   dbus_message_iter_append_basic( &m_iter, DBUS_TYPE_STRING, &str );
}

std::string MessageConstIter::signature() const
{
   char *ptr = dbus_message_iter_get_signature( const_cast< DBusMessageIter *>( &m_iter ) );
   std::string rez( ptr );
   dbus_free( ptr );
   return rez;
}

MessageConstIter MessageConstIter::recurse() const
{
   MessageConstIter rez;
   dbus_message_iter_recurse( const_cast< DBusMessageIter *>( &m_iter ), &rez.m_iter );
   return rez;
}

void *MessageConstIter::getFixedArray( int &elements ) const
{
   void *data =0;
   dbus_message_iter_get_fixed_array( const_cast< DBusMessageIter *>( &m_iter ), &data, &elements );
   return data;
}

Message::Message( int type )
{
   m_message = dbus_message_new( type );
}

Message::Message( const Message &msg )
   : m_message( msg.m_message )
{
   dbus_message_ref( m_message );
}

Message::Message( DBusMessage *msg, bool ownership )
   : m_message( msg )
{
   if( !ownership )
      dbus_message_ref( m_message );
}

Message::~Message()
{
   dbus_message_unref( m_message );
}

Message Message::newMethodReturn() const
{
   return Message( dbus_message_new_method_return( m_message ), true );
}

Message Message::newError( const char *error, const char *message ) const
{
   return Message( dbus_message_new_error( m_message, error, message ), true );
}

MessageConstIter Message::constIter() const
{
   MessageConstIter rez;
   dbus_message_iter_init( m_message, &rez.m_iter );
   return rez;
}

MessageIter Message::appendIter()
{
   MessageIter rez;
   dbus_message_iter_init_append( m_message, &rez.m_iter );
   return rez;
}

void Message::append( const char *str )
{
   dbus_message_append_args( m_message, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID );
}

const char *Message::path() const
{
   return dbus_message_get_path( m_message );
}

const char *Message::interface() const
{
   return dbus_message_get_interface( m_message );
}

const char *Message::member() const
{
   return dbus_message_get_member( m_message );
}

