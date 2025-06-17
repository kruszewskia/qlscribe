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

FIND_PATH( LSCRIBE_INCLUDE_DIR lightscribe.h /usr/local/include /usr/include /usr/include/lightscribe $ENV{LIGHTSCRIBEDIR}/include )

FIND_LIBRARY( LSCRIBE_LIBRARY liblightscribe.so /usr/lib /opt/lightscribe/lib /opt/lightscribe/lib32 $ENV{LIGHTSCRIBEDIR}/lib )

if( LSCRIBE_INCLUDE_DIR AND LSCRIBE_LIBRARY )
    SET( LSCRIBE_FOUND TRUE )
    SET( LSCRIBE_LIBRARIES ${LSCRIBE_LIBRARY} )
    INCLUDE_DIRECTORIES(${LSCRIBE_INCLUDE_DIR})
ENDIF( LSCRIBE_INCLUDE_DIR AND LSCRIBE_LIBRARY )

IF( LSCRIBE_FOUND )
    IF( NOT LightScribe_FIND_QUIETLY )
        MESSAGE( STATUS "Found lighstcribe include: ${LSCRIBE_INCLUDE_DIR}" )
        MESSAGE( STATUS "Found lighstcribe libraries: ${LSCRIBE_LIBRARIES}" )
    ENDIF( NOT LightScribe_FIND_QUIETLY )
ELSE( LSCRIBE_FOUND )
    IF( LightScribe_FIND_REQUIRED )
        MESSAGE( FATAL_ERROR "Could not find LightScribe" )
    ENDIF( LightScribe_FIND_REQUIRED )
ENDIF( LSCRIBE_FOUND )

MARK_AS_ADVANCED( LSCRIBE_INCLUDE_DIR LSCRIBE_LIBRARY LSCRIBE_FOUND )
