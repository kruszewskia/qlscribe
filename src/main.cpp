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

#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"

uid_t realUserId;

int main(int argc, char **argv)
{
   QApplication app(argc, argv);
   app.addLibraryPath( "/usr/lib32/qt4/plugins" );

   bool enablePrint = false;
   if( geteuid() ) {
      if( QMessageBox::question( 0,
                                 QObject::tr( "Confirmation" ),
                                 QObject::tr( "Print functionality requires setuid (sticky) flag set on the application\n"
                                              "This program does not seem to have it set, print functiionality will be disabled\n"
                                              "You still will be able to do print preview and edit documents\n"
                                              "Do you want to continue?" ),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No )
         == QMessageBox::No )
         return 1;
   } else {
      realUserId = getuid();
      setreuid( 0, realUserId );
      enablePrint = true;
   }
   MainWindow mwindow( enablePrint );
   mwindow.show();

   return app.exec();
}
