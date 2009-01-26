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

#include "mainwindow.h"
#include "qgraphicsroundtextitem.h"
#include "qcdview.h"
#include "qcdscene.h"
#include "qshapefactory.h"
#include "qlightscribe.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSignalMapper>
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <QLabel>

/*#include <QImage>
#include <QBuffer>
#include <QGraphicsScene>
#include <QFontDialog>
#include <QPainter>*/

//#include <lightscribe.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_mdiArea( new QMdiArea( this ) ),
    m_menuFile( 0 ), m_menuInsert( 0 ),
    m_insertMapper( new QSignalMapper( this ) )
{
   setCentralWidget( m_mdiArea );

   m_menuFile   = menuBar()->addMenu( tr( "File",   "Menu item \"File\"" ) );

   m_menuFile->addAction( tr( "New", "Menu item \"New\"" ),
                          this,
                          SLOT(onMenuNew()) );

   m_menuFile->addAction( tr( "Open...", "Menu item \"Open\"" ),
                          this,
                          SLOT(onMenuOpen()) );

   m_menuFile->addAction( tr( "Save", "Menu item \"Save\"" ),
                          this,
                          SLOT(onMenuSave()) );


   m_menuFile->addAction( tr( "Save as...", "Menu item \"Save as\"" ),
                          this,
                          SLOT(onMenuSaveAs()) );

   m_menuFile->addSeparator();

   m_menuFile->addAction( tr( "Exit", "Menu item \"Exit\"" ),
                          this,
                          SLOT(close()));

   m_menuInsert = menuBar()->addMenu( tr( "Insert", "Menu item \"Insert\"" ) );

   m_menuHelp = menuBar()->addMenu( tr( "Help", "Menu item \"Help\"" ) );

   m_menuHelp->addAction( tr( "About...", "Menu item \"About\"" ),
                          this,
                          SLOT(onMenuAbout()) );

   m_menuHelp->addAction( tr( "About Qt...", "Menu item \"About Qt\"" ),
                          this,
                          SLOT(onMenuQtAbout()) );

    QShapeFactory &sfactory = QShapeFactory::instance();

    for( QShapeFactory::iterator i = sfactory.begin(); i != sfactory.end(); ++i ) {
       QAction *action = new QAction( i->second->menuName(), this );
       m_menuInsert->addAction( action );
       m_insertMapper->setMapping( action, i->first );
       connect( action, SIGNAL(triggered()), m_insertMapper, SLOT(map()) );
    }
    connect( m_insertMapper, SIGNAL(mapped(int)), this, SLOT(onMenuInsert(int)) );
}

MainWindow::~MainWindow()
{
   QLightScribe *lscribe = QLightScribe::instance();
   lscribe->stopThread();
   lscribe->wait( 1000 );
}

void MainWindow::onMenuNew()
{
   QCDView *newView = new QCDView;
   newView->setScene( new QCDScene( newView ) );
   //newView->setWindowTitle( "Baba" );
   QMdiSubWindow *subWindow = m_mdiArea->addSubWindow( newView );
   subWindow->show();
}

inline
QCDScene *getScene( QMdiArea *area )
{
   QMdiSubWindow *subWindow =  area->activeSubWindow();
   if( !subWindow )
      return 0;

   QCDView *cdview = dynamic_cast< QCDView * >( subWindow->widget() );
   if( !cdview )
      return 0;

   return cdview->scene();
}

void MainWindow::onMenuInsert( int id )
{
   QCDScene *cdscene = getScene( m_mdiArea );
   if( !cdscene )
      return;

   QShapeFactory &sfactory = QShapeFactory::instance();
   QGraphicsItem *item = sfactory.create( id );
   if( !item ) {
      QMessageBox::warning( this,
                            "Warning",
                            tr( "Creator for type id %n returned zero pointer", 0, id ) );
      return;
   }
   if( !sfactory.edit( item, this ) ) {
      delete item;
      return;
   }

   item->setFlag( QGraphicsItem::ItemIsMovable, true );
   item->setFlag( QGraphicsItem::ItemIsSelectable, true );
   cdscene->addItem( item );
}

void MainWindow::onMenuOpen()
{
   QString fileName = QFileDialog::getOpenFileName( this, 
                                                    tr( "Open:" ), 
                                                    QString(), 
                                                    tr("qlscribe document (*.qlx)") );
   
   if( fileName.isNull() )
      return;

   QFile file( fileName );
   if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
      QMessageBox::critical( this, tr( "Error" ), tr( "Cannot open file for reading\n" ) + fileName );
      return;
   }
   
   QCDView *newView = new QCDView;
   QCDScene *scene = new QCDScene( newView );
   newView->setScene( scene );
   
   QXmlStreamReader reader( &file );
   try {
      scene->read( reader );
   }
   catch( const QString &err ) {
      QMessageBox::critical( this, tr( "Error" ), tr( "Cannot read file " ) + fileName + "\n" + err );
      delete newView;
      return;
   }      
   
   //newView->setWindowTitle( "Baba" );
   QMdiSubWindow *subWindow = m_mdiArea->addSubWindow( newView );
   subWindow->show();
}

void MainWindow::onMenuSave()
{
}

void MainWindow::onMenuSaveAs()
{
   QCDScene *cdscene = getScene( m_mdiArea );
   if( !cdscene )
      return;

   QString fileName =
         QFileDialog::getSaveFileName( this,
                                       tr( "Save as:" ),
                                       QString(),
                                       tr("qlscribe document (*.qlx)") );
   if( fileName.isNull() )
      return;

   if( !fileName.contains( '.' ) )
      fileName += ".qlx";

   QFile file( fileName );
   if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) ) {
      QMessageBox::warning( this, tr( "Warning" ), tr( "Cannot open file for writing: " ) + fileName );
      return;
   }
   QXmlStreamWriter writer( &file );
   writer.setAutoFormatting( true );
   cdscene->write( writer );
}


void MainWindow::onMenuAbout()
{
   QMessageBox::about( this,
                       tr( "About" ),
                       tr( "<h3>qlscribe - Qt lisghtScribe</h3>"
                           "<p>$Revision$</p>"
                           "<p>visit project at home page "
                           "<a href=\"http://qlscribe.sourceforge.net/\">qlscribe.sourceforge.net</a></p>" ) );
}

void MainWindow::onMenuQtAbout()
{
   QMessageBox::aboutQt( this );
}
