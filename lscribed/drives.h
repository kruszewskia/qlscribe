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

#ifndef DRIVES_H
#define DRIVES_H

#include <vector>
#include <string>

#include "dbuscpp.h"

class Drive {
public:
   const std::string &path() const { return m_path; }
   const std::string &name() const { return m_name; }

   void invoke( const std::string &method, const DBusCpp::Message &msg );

   friend class DrivesManager;
   friend void *routine( void * );

private:
   Drive( int index, const std::string &path, const std::string &name );
   ~Drive();

   void routine();

   int               m_index;
   std::string       m_path;
   std::string       m_name;
   bool              m_threadStarted;
   pthread_t         m_thread;
   pthread_cond_t    m_cond;
   DBusCpp::Message *m_message;
};

class DrivesManager {
public:
   typedef std::vector< Drive * > Drives;
   typedef Drives::const_iterator const_iterator;

   static DrivesManager &instance();

   void init( DBusCpp::Connection conn );

   const_iterator begin() const { return m_drives.begin(); }
   const_iterator end() const { return m_drives.end(); }
   const_iterator find( const std::string &path ) const;

   DBusCpp::Connection connection() const { return m_connection; }

private:
   DrivesManager();
   ~DrivesManager();

   Drives              m_drives;
   DBusCpp::Connection m_connection;
};

#endif // DRIVES_H
