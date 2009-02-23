#    qlscribe - Qt based application to print lightScribe discs
#
#    Copyright (C) 2009 Vyacheslav Kononenko <vyacheslav@kononenko.net>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
#    $Id$ 

FIND_PATH( DBUS_INCLUDE_DIR dbus/dbus.h
           PATH_SUFFIXES dbus-1.0 )

IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
        FIND_LIBRARY( DBUS_LIBRARY dbus-1 
                      PATHS /lib32 /usr/lib32 /usr/local/lib32
                      NO_DEFAULT_PATH )
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

FIND_LIBRARY( DBUS_LIBRARY dbus-1 )

get_filename_component( _dbusLibPath ${DBUS_LIBRARY} PATH )

FIND_PATH( DBUS_LIBINCLUDE_DIR dbus/dbus-arch-deps.h
           PATHS /lib /usr/lib /usr/local/lib ${_dbusLibPath}
           PATH_SUFFIXES dbus-1.0/include )

if( DBUS_INCLUDE_DIR AND DBUS_LIBINCLUDE_DIR AND DBUS_LIBRARY )
    SET( Dbus32_FOUND TRUE )
    SET( DBUS_LIBRARIES ${DBUS_LIBRARY} )
    INCLUDE_DIRECTORIES(${DBUS_INCLUDE_DIR} ${DBUS_LIBINCLUDE_DIR} )
ENDIF( DBUS_INCLUDE_DIR AND DBUS_LIBINCLUDE_DIR AND DBUS_LIBRARY )

IF( Dbus32_FOUND )
    IF( NOT Dbus32_FIND_QUIETLY )
        MESSAGE( STATUS "Found dbus include: ${LSCRIBE_INCLUDE_DIR}" )
        MESSAGE( STATUS "Found dbus libraries: ${LSCRIBE_LIBRARIES}" )
    ENDIF( NOT Dbus32_FIND_QUIETLY )
ELSE( Dbus32_FOUND )
    IF( Dbus32_FIND_REQUIRED )
        MESSAGE( FATAL_ERROR "Could not find dbus" )
    ENDIF( Dbus32_FIND_REQUIRED )
ENDIF( Dbus32_FOUND )

MARK_AS_ADVANCED( Dbus32_FOUND DBUS_INCLUDE_DIR DBUS_LIBRARY DBUS_LIBINCLUDE_DIR )
