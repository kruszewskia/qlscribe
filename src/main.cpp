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
#include <QMap>
#include "mainwindow.h"
#include "qcdscene.h"
#include "qlightscribe.h"
#include "qconsoleprintprogress.h"

#include <iostream>

std::ostream &operator<<( std::ostream &os, const QString &str )
{
   return os << str.toAscii().data();
}

void usage()
{
   std::cout << "Usage: \n"
         << "To run GUI: qlscribe [filename] [filename] ...\n"
         << "To print label in console mode: qlscribe --print [--drive Index] [--replace KEY=VALUE] filename\n"
         << "\tParameters:\n"
         << "\t\t--help | -h               - print this message\n"
         << "\t\t--print | -p              - switch to print mode\n"
         << "\t\t--drive | -d NUMBER       - use this drive, starts from 0, 0 is default\n"
         << "\t\t--replace | -r KEY=VALUE  - replace text from KEY to VALUE\n"
         << std::endl;
}

int main( int argc, char **argv )
{
   QApplication app( argc, argv );
   app.addLibraryPath( "/usr/lib32/qt4/plugins" );

   bool doPrint = false;
   QStringList arguments = app.arguments();
   QStringList files;
   QCDScene::QString2String replacements;
   int driveIndex = 0;
   PrintParameters params;
   bool labelModeOverriden = false;

   for( int i = 1; i < arguments.size(); ++i ) {
      QString arg = arguments[i];
      if( arg == "--print" || arg == "-p" ) {
         doPrint = true;
         continue;
      }
      if( arg == "--help" || arg == "-h" ) {
         usage();
         return 0;
      }
      if( arg == "--replace" || arg == "-r" ) {
         if( i == arguments.size() - 1 ) {
            std::cerr << "Error: arg#" << i
                  << " argument expected for \"" << arg << "\" not enough parameters" << std::endl;
            return 1;
         }
         QString param = arguments[ ++i ];
         int pos = param.indexOf( '=' );
         if( pos == -1 ) {
            std::cerr << "Error: arg#" << i
                  << " invalid argument for replacement, KEY=VALUE expected, missing =" << std::endl;
            return 2;
         }
         replacements[ param.left( pos ) ] = param.right( param.size() - pos - 1 );
         continue;
      }
      if( arg == "--drive" || arg == "-d" ) {
         if( i == arguments.size() - 1 ) {
            std::cerr << "Error: arg#" << i
                  << " argument expected for \"" << arg << "\" not enough parameters" << std::endl;
            return 1;
         }
         QString param = arguments[ ++i ];
         driveIndex = param.toInt();
      }
      if( arg[0] != '-' )
         files.append( arg );
   }

   if( doPrint && files.size() != 1 ) {
      if( files.size() )
         std::cerr << "Error: only one filename expected to print, got " << files.size() << " instead" << std::endl;
      else
         std::cerr << "Error: filename required to print" << std::endl;
      return 3;
   }

   int rez = 0;

   if( doPrint ) {
      // print here
      QCDScene scene;
      QString err;
      if( !scene.load( files.front(), &err ) ) {
         std::cerr << "Error: cannot read \"" << files.front() << "\" - " << err << std::endl;
         return 5;
      }
      scene.replace( replacements );
      if( !labelModeOverriden )
         params.m_labelMode = scene.labelMode();

      QLightScribe *scribe = QLightScribe::instance();
      QList< QLightDrive * > drives = scribe->getDrives();
      if( driveIndex >= drives.size() ) {
         std::cerr << "Error: drive " << driveIndex << " specified, but there are only " << drives.size() << " drives" << std::endl;
         return 6;
      }
      std::cout << "Printing label " << files.front() << std::endl;

      QLightDrive *drive = drives[driveIndex];
      QConsolePrintProgress progress( drive );
      drive->print( params, &scene );
      rez = app.exec();
   } else {
      MainWindow mwindow( true );
      mwindow.show();
      mwindow.open( files );
      rez = app.exec();
   }
   return rez;
}
