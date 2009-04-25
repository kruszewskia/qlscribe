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

SET( LSCRIBED_CONF_FILE ${CMAKE_CURRENT_BINARY_DIR}/lightscribe.conf )
SET( LSCRIBED_SERVICE_FILE ${CMAKE_CURRENT_BINARY_DIR}/${QLSCRIBE_DBUS_SERVICE}.printManager.service )

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/lscribed.h.in ${CMAKE_CURRENT_BINARY_DIR}/lscribed.h @ONLY )
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/lightscribe.conf.in ${LSCRIBED_CONF_FILE}  @ONLY )
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/service.in ${LSCRIBED_SERVICE_FILE} @ONLY )

# lightSribe provides library only for m32 (yet?)
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCXX )
     ADD_DEFINITIONS( -m32 )
     SET( CMAKE_EXE_LINKER_FLAGS "-m32 -pthread ${CMAKE_EXE_LINKER_FLAGS}" )
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCXX )

IF (DBUS_SYSTEM_POLICY_DIR)
     INSTALL( FILES ${LSCRIBED_CONF_FILE} DESTINATION ${DBUS_SYSTEM_POLICY_DIR})
ELSE (DBUS_SYSTEM_POLICY_DIR)
     MESSAGE(STATUS "It is necessary to specify the directory where the client policy file for Qt Lightscribe is installed as DBUS_SYSTEM_POLICY_DIR.")
     MESSAGE(STATUS "Otherwise it will be installed into share/qlscribe")
     MESSAGE(STATUS "Normally this is /etc/dbus-1/system.d on a Linux system")
     INSTALL( FILES ${LSCRIBED_CONF_FILE} DESTINATION share/qlscribe )
ENDIF (DBUS_SYSTEM_POLICY_DIR)

IF (DBUS_SYSTEM_SERVICE_DIR)
     INSTALL( FILES ${LSCRIBED_SERVICE_FILE} DESTINATION ${DBUS_SYSTEM_SERVICE_DIR})
ELSE (DBUS_SYSTEM_SERVICE_DIR)
     MESSAGE(STATUS "It is necessary to specify the directory where the client service file for Qt Lightscribe is installed as DBUS_SYSTEM_SERVICE_DIR.")
     MESSAGE(STATUS "Otherwise it will be installed into share/qlscribe")
     MESSAGE(STATUS "Normally this is /usr/share/dbus-1/services on a Linux system")
     INSTALL( FILES ${LSCRIBED_SERVICE_FILE} DESTINATION share/qlscribe )
ENDIF (DBUS_SYSTEM_SERVICE_DIR)

SET( LSCRIBED_SRCS main.cpp dbuscpp.cpp drives.cpp managerhandler.cpp drivehandler.cpp
                   introspecthandler.cpp )

ADD_DEFINITIONS( -Wall )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

ADD_EXECUTABLE( lscribed ${LSCRIBED_SRCS} )

TARGET_LINK_LIBRARIES( lscribed ${LSCRIBE_LIBRARIES} ${DBUS_LIBRARIES} pthread )

INSTALL( TARGETS lscribed DESTINATION sbin )

