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

FIND_PACKAGE( Dbus32 REQUIRED )
FIND_PACKAGE( LightScribe REQUIRED )

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/lscribed.h.in ${CMAKE_CURRENT_BINARY_DIR}/lscribed.h @ONLY )
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/lightscribe.conf.in ${CMAKE_CURRENT_BINARY_DIR}/lightscribe.conf @ONLY )

# lightSribe provides library only for m32 (yet?)
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCXX )
     ADD_DEFINITIONS( -m32 )
     SET( CMAKE_EXE_LINKER_FLAGS "-m32 -pthread ${CMAKE_EXE_LINKER_FLAGS}" )
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCXX )

SET( LSCRIBED_SRCS main.cpp dbuscpp.cpp )

ADD_DEFINITIONS( -Wall )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

ADD_EXECUTABLE( lscribed ${LSCRIBED_SRCS} )

TARGET_LINK_LIBRARIES( lscribed ${LSCRIBE_LIBRARIES} ${DBUS_LIBRARIES} )

#INSTALL( TARGETS qlscribe DESTINATION sbin )

