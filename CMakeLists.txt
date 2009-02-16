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

PROJECT( qlscribe )

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

# lightSribe provides library only for m32 (yet?)
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCXX )
    ADD_DEFINITIONS( -m32 ) 
    SET( CMAKE_EXE_LINKER_FLAGS "-m32 ${CMAKE_EXE_LINKER_FLAGS}" )
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCXX )

SET( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
FIND_PACKAGE( Qt4 REQUIRED )
FIND_PACKAGE( LightScribe REQUIRED )

SUBDIRS( src )
SUBDIRS( resources )
