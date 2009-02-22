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
#include "qlightroundtextitem.h"
#include "qcdview.h"
#include "qcdscene.h"
#include "qshapefactory.h"
#include "qlightscribe.h"
#include "qdialogprint.h"
#include "qdialogprogress.h"
#include "qdialogcdproperties.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSignalMapper>
#include <QFileDialog>
#include <QLabel>
#include <QCloseEvent>


MainWindow::MainWindow( bool enablePrint )
    : QMainWindow( 0 ),
    m_mdiArea( new QMdiArea( this ) ),
    m_menuFile( 0 ), m_menuInsert( 0 ),
    m_insertMapper( new QSignalMapper( this ) ),
    m_newLabelMapper( new QSignalMapper( this ) )
{
   setWindowTitle( tr( "Qt lightScribe", "Main window title \"Qt lightScribe\"" ) );
   setCentralWidget( m_mdiArea );

   m_menuFile   = menuBar()->addMenu( tr( "File",   "Menu item \"File\"" ) );

   {
      QMenu *newSubMenu = m_menuFile->addMenu( tr( "New label", "Menu item \"New lable\"" ) );
      QAction *action = newSubMenu->addAction( tr( "Full", "Menu item \"Full\"" ),
                                               m_newLabelMapper,
                                               SLOT(map()) );
      m_newLabelMapper->setMapping( action, modeFull );

      action = newSubMenu->addAction( tr( "Content", "Menu item \"Content\"" ),
                                      m_newLabelMapper,
                                      SLOT(map()) );
      m_newLabelMapper->setMapping( action, modeContent );

      action = newSubMenu->addAction( tr( "Title", "Menu item \"Title\"" ),
                                      m_newLabelMapper,
                                      SLOT(map()) );
      m_newLabelMapper->setMapping( action, modeTitle );
   }
   connect( m_newLabelMapper, SIGNAL(mapped(int)), this, SLOT(onMenuNewLabel(int)) );

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

   m_menuFile->addAction( tr( "Label properties...", "Menu item \"Label propeties...\"" ),
                          this,
                          SLOT(onMenuProperties()) );

   m_menuFile->addSeparator();

   m_menuFile->addAction( tr( "Print preview...", "Menu item \"Print preview\"" ),
                          this,
                          SLOT(onMenuPrintPreview()) );

   QAction *printAction =
         m_menuFile->addAction( tr( "Print...", "Menu item \"Print\"" ),
                                this,
                                SLOT(onMenuPrint()) );
   if( !enablePrint )
      printAction->setEnabled( false );

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
}

void MainWindow::open( const QStringList &files )
{
   foreach( QString fileName, files ) {
      QCDView *newView = new QCDView;
      QCDScene *scene = new QCDScene( newView );
      newView->setScene( scene );

      if( !scene->load( fileName ) ) {
         delete newView;
         continue;
      }

      QMdiSubWindow *subWindow = m_mdiArea->addSubWindow( newView );
      subWindow->show();
   }
}

bool MainWindow::saveSceneAs( QCDScene *scene )
{
   QString fileName =
         QFileDialog::getSaveFileName( this,
                                       scene->name() + tr( ": save as" ),
                                       QString(),
                                       tr("qlscribe document (*.qlx)") );
   if( fileName.isNull() )
      return false;

   if( !fileName.contains( '.' ) )
      fileName += ".qlx";

   return scene->saveAs( fileName );
}

bool MainWindow::saveScene( QCDScene *scene )
{
   return scene->isUnnamed() ? saveSceneAs( scene ) : scene->save();
}


 void MainWindow::closeEvent(QCloseEvent *event)
 {
    bool saveAll = false;
    QList< QMdiSubWindow * > subs = m_mdiArea->subWindowList();
    foreach( QMdiSubWindow *sub, subs ) {
       QCDView *cdview = dynamic_cast< QCDView * >( sub->widget() );
       if( !cdview )
          continue;

       QCDScene *scene = cdview->scene();
       if( !scene || scene->isSaved() )
          continue;

       bool save = saveAll;
       if( !save ) {
          QMessageBox::StandardButton button =
                QMessageBox::question( this,
                                       tr( "Confirmation" ),
                                       scene->name() + tr( " is unsaved, do you want to save?" ),
                                       QMessageBox::Save | QMessageBox::SaveAll | QMessageBox::Discard | QMessageBox::Cancel );
          if( button == QMessageBox::Cancel ) {
             event->ignore();
             return;
          }
          if( button == QMessageBox::SaveAll ) {
             saveAll = true;
             save = true;
          }
          if( button == QMessageBox::Save )
             save = true;
       }
       if( save )
          if( !saveScene( scene ) ) {
             event->ignore();
             return;
          }
    }
    event->accept();
 }

void MainWindow::onMenuNewLabel( int mode )
{
   QCDView *newView = new QCDView;
   QCDScene *scene = new QCDScene( newView );
   scene->setLabelMode( LabelMode( mode ) );
   newView->setScene( scene );
   scene->setName();

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
   cdscene->setChanged();
}

void MainWindow::onMenuOpen()
{
   QString fileName = QFileDialog::getOpenFileName( this, 
                                                    tr( "Open:" ), 
                                                    QString(), 
                                                    tr("qlscribe document (*.qlx)") );
   
   if( fileName.isNull() )
      return;

   QCDView *newView = new QCDView;
   QCDScene *scene = new QCDScene( newView );
   newView->setScene( scene );

   if( !scene->load( fileName ) ) {
      delete newView;
      return;
   }

   QMdiSubWindow *subWindow = m_mdiArea->addSubWindow( newView );
   subWindow->show();
}

void MainWindow::onMenuSave()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene )
      saveScene( scene );
}

void MainWindow::onMenuSaveAs()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene )
      saveSceneAs( scene );
}

void MainWindow::onMenuProperties()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene )
      QDialogCDProperties::exec( this, scene );
}

void MainWindow::onMenuPrintPreview()
{
   QCDScene *cdscene = getScene( m_mdiArea );
   if( !cdscene )
      return;

   PrintParameters params;
   params.m_labelMode = cdscene->labelMode();
   QLightDrive *drive = QDialogPrint::exec( this, params );
   if( !drive )
      return;

   try {
      QPixmap pixmap = drive->preview( params, cdscene, QSize( 400, 400 ) );

      QLabel *label = new QLabel;
      label->setPixmap( pixmap );
      QMdiSubWindow *subWindow = m_mdiArea->addSubWindow( label );
      subWindow->setWindowTitle( "Preview: " + cdscene->name() );
      subWindow->show();
   }
   catch( const QString &err ) {
      QMessageBox::critical( this, tr( "Error" ), err );
   }
}

void MainWindow::onMenuPrint()
{
   QCDScene *cdscene = getScene( m_mdiArea );
   if( !cdscene )
      return;

   QDialogProgress::exec( this, cdscene );
}

void MainWindow::onMenuAbout()
{
   QMessageBox::about( this,
                       tr( "About" ),
                       tr( "<h3>qlscribe - Qt lisghtScribe</h3>"
                           "<p>release 0.8 $Revision$</p>"
                           "<p>visit project at home page "
                           "<a href=\"http://qlscribe.sourceforge.net/\">qlscribe.sourceforge.net</a></p>" ) );
}

void MainWindow::onMenuQtAbout()
{
   QMessageBox::aboutQt( this );
}
