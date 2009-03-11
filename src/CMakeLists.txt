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

FIND_PACKAGE( Qt4 REQUIRED )
SET( QT_USE_QTDBUS 1 )

SET( QLSCRIBE_SRCS main.cpp mainwindow.cpp qcdscene.cpp qcdview.cpp
     qdialogpixmap.cpp qdialogroundtext.cpp qdialogtext.cpp qlightroundtextitem.cpp 
     qlightscribe.cpp qshapefactory.cpp qlighttextitem.cpp qlightpixmapitem.cpp 
     qdialogprint.cpp qdialogprogress.cpp qdialogcdproperties.cpp qconsoleprintprogress.cpp)

SET( QLSCRIBE_UIS qdialogtext.ui qdialogroundtext.ui qdialogpixmap.ui
     qdialogprint.ui qdialogprogress.ui qdialogcdproperties.ui)

SET( QLSCRIBE_MOC_HDRS mainwindow.h qcdscene.h qdialogpixmap.h qdialogroundtext.h
     qdialogtext.h qlightscribe.h qdialogprint.h qdialogprogress.h
     qdialogcdproperties.h qconsoleprintprogress.h)

ADD_DEFINITIONS( -Wall )

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/qlscribe.h.in ${CMAKE_CURRENT_BINARY_DIR}/qlscribe.h @ONLY )

INCLUDE( ${QT_USE_FILE} )

QT4_WRAP_UI( QLSCRIBE_UI_HDRS ${QLSCRIBE_UIS} )

QT4_WRAP_CPP( QLSCRIBE_MOC_SRCS ${QLSCRIBE_MOC_HDRS} )

QT4_ADD_RESOURCES( QLSCRIBE_RCC_SRCS ../resources/qlscribe.qrc )

MACRO(ADD_DBUS_INTERFACE _sources _interface _basename)
         GET_FILENAME_COMPONENT(_infile ${_interface} ABSOLUTE)
         SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
         SET(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
         SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

         ADD_CUSTOM_COMMAND(OUTPUT ${_impl} ${_header}
           COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -i qlightscribe.h -p ${_basename} ${_infile}
           DEPENDS ${_infile})

         SET_SOURCE_FILES_PROPERTIES(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

         QT4_GENERATE_MOC(${_header} ${_moc})

         SET(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
         MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})
ENDMACRO(ADD_DBUS_INTERFACE)

ADD_DBUS_INTERFACE( QLSCRIBE_DBUS_SRCS lightscribe.xml lightscribe_interface )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_EXECUTABLE( qlscribe ${QLSCRIBE_SRCS} ${QLSCRIBE_MOC_SRCS}
                         ${QLSCRIBE_RCC_SRCS} ${QLSCRIBE_UI_HDRS} 
                         ${QLSCRIBE_DBUS_SRCS} )

TARGET_LINK_LIBRARIES( qlscribe ${QT_LIBRARIES} )

INSTALL( TARGETS qlscribe 
         DESTINATION bin )

