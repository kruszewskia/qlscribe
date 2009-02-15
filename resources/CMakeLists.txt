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

SET( DESKTOP_DIR share/applications )
SET( MIME_DIR share/mime/packages )
SET( ICON_DIR share/icons/hicolor )

INSTALL( FILES qlscribe.desktop DESTINATION ${DESKTOP_DIR} )

INSTALL( FILES qlscribe.xml DESTINATION ${MIME_DIR} )

MACRO( INSTALL_ICON SIZE )
        INSTALL( FILES qlscribe${SIZE}.png DESTINATION ${ICON_DIR}/${SIZE}x${SIZE}/apps RENAME qlscribe.png )
ENDMACRO( INSTALL_ICON )

INSTALL_ICON( 64 )
INSTALL_ICON( 48 )
INSTALL_ICON( 32 )

