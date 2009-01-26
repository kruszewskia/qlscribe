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

SET( QLSCRIBE_SRCS main.cpp mainwindow.cpp qcdscene.cpp qcdview.cpp qdialogpixmap.cpp qdialogroundtext.cpp
     qdialogtext.cpp qgraphicsroundtextitem.cpp qlightscribe.cpp qshapecontrollers.cpp qshapefactory.cpp 
     qdialogprint.cpp )

SET( QLSCRIBE_UIS qdialogtext.ui qdialogroundtext.ui qdialogpixmap.ui qdialogprint.ui )

SET( QLSCRIBE_MOC_HDRS mainwindow.h qcdscene.h qdialogpixmap.h qdialogroundtext.h
     qdialogtext.h qlightscribe.h qdialogprint.h )

ADD_DEFINITIONS( -Wall )

INCLUDE( ${QT_USE_FILE} )

QT4_WRAP_UI( QLSCRIBE_UI_HDRS ${QLSCRIBE_UIS} )

QT4_WRAP_CPP( QLSCRIBE_MOC_SRCS ${QLSCRIBE_MOC_HDRS} )

INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR}/src ${CMAKE_SOURCE_DIR}/src )

ADD_EXECUTABLE( qlscribe ${QLSCRIBE_SRCS} ${QLSCRIBE_MOC_SRCS} ${QLSCRIBE_UI_HDRS} )

TARGET_LINK_LIBRARIES( qlscribe ${QT_LIBRARIES} ${LSCRIBE_LIBRARIES} )

INSTALL( TARGETS qlscribe DESTINATION bin )
