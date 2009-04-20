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
#include "qdialogsettings.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSignalMapper>
#include <QFileDialog>
#include <QLabel>
#include <QCloseEvent>
#include <QApplication>
#include <QClipboard>
#include <QSettings>


MainWindow::MainWindow( bool enablePrint )
    : QMainWindow( 0 ),
    m_mdiArea( new QMdiArea( this ) ),
    m_menuFile( 0 ), m_menuInsert( 0 ),
    m_insertMapper( new QSignalMapper( this ) ),
    m_newLabelMapper( new QSignalMapper( this ) )
{
   setWindowTitle( tr( "Qt lightScribe", "Main window title \"Qt lightScribe\"" ) );
   setCentralWidget( m_mdiArea );

   m_menuFile = menuBar()->addMenu( tr( "File",   "Menu item \"File\"" ) );

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
                          SLOT(onMenuOpen()),
                          QKeySequence( Qt::CTRL + Qt::Key_O ) );

   m_actionSave = m_menuFile->addAction( tr( "Save", "Menu item \"Save\"" ),
                                         this,
                                         SLOT(onMenuSave()),
                                         QKeySequence( Qt::CTRL + Qt::Key_S ) );

   m_actionSaveAs = m_menuFile->addAction( tr( "Save as...", "Menu item \"Save as\"" ),
                                           this,
                                           SLOT(onMenuSaveAs()) );

   m_menuFile->addSeparator();

   m_actionLabelProperties =
         m_menuFile->addAction( tr( "Label properties...", "Menu item \"Label propeties...\"" ),
                                this,
                                SLOT(onMenuProperties()) );

   m_menuFile->addSeparator();

   m_actionPrintPreview =
         m_menuFile->addAction( tr( "Print preview...", "Menu item \"Print preview\"" ),
                                this,
                                SLOT(onMenuPrintPreview()) );

   m_actionPrint =
         m_menuFile->addAction( tr( "Print...", "Menu item \"Print\"" ),
                                this,
                                SLOT(onMenuPrint()) );

   m_menuFile->addAction( tr( "Exit", "Menu item \"Exit\"" ),
                          this,
                          SLOT(close()),
                          QKeySequence( Qt::CTRL + Qt::Key_Q ) );


   m_menuEdit = menuBar()->addMenu( tr( "Edit",   "Menu item \"Edit\"" ) );

   m_actionUndo = m_menuEdit->addAction( tr( "Undo", "Menu item \"Undo\"" ),
                                         this,
                                         SLOT(onMenuUndo()),
                                         QKeySequence( Qt::CTRL + Qt::Key_Z ) );

   m_actionRedo = m_menuEdit->addAction( tr( "Redo", "Menu item \"Redo\"" ),
                                         this,
                                         SLOT(onMenuRedo()),
                                         QKeySequence( Qt::CTRL + Qt::Key_R ) );

   m_actionCopy = m_menuEdit->addAction( tr( "Copy", "Menu item \"Copy\"" ),
                                         this,
                                         SLOT(onMenuCopy()),
                                         QKeySequence( Qt::CTRL + Qt::Key_C ) );

   m_actionCut = m_menuEdit->addAction( tr( "Cut", "Menu item \"Cut\"" ),
                                        this,
                                        SLOT(onMenuCut()),
                                        QKeySequence( Qt::CTRL + Qt::Key_X ) );

   m_actionPaste = m_menuEdit->addAction( tr( "Paste", "Menu item \"Paste\"" ),
                                          this,
                                          SLOT(onMenuPaste()),
                                          QKeySequence( Qt::CTRL + Qt::Key_V ) );

   m_menuInsert = m_menuEdit->addMenu( tr( "Insert", "Menu item \"Insert\"" ) );

   m_menuEdit->addSeparator();

   m_actionSettings = m_menuEdit->addAction( tr( "Settings...", "Menu item \"Settings\"" ),
                                             this,
                                             SLOT(onMenuSettings()) );

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

    connect( QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)),
             this, SLOT(updateMenu()) );
    connect( m_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
             this, SLOT(updateMenu()) );

    updateMenu();

    QSettings settings;

    settings.beginGroup( "MainWindow");
    resize( settings.value( "size", QSize(800, 400) ).toSize() );
    move( settings.value( "pos", QPoint(200, 200) ).toPoint() );
    settings.endGroup();
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
    QSettings settings;

    settings.beginGroup( "MainWindow" );
    settings.setValue( "size", size());
    settings.setValue( "pos", pos());
    settings.endGroup();

    event->accept();
 }

void MainWindow::onMenuNewLabel( int mode )
{
   QCDView *newView = new QCDView;
   QCDScene *scene = new QCDScene( newView );
   scene->start( LabelMode( mode ) );
   newView->setScene( scene );
   scene->setName();
   connect( scene, SIGNAL(selectionChanged()), this, SLOT(updateMenu()) );
   connect( scene, SIGNAL(changed()), this, SLOT(updateMenu()) );

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
   connect( scene, SIGNAL(selectionChanged()), this, SLOT(updateMenu()) );
   connect( scene, SIGNAL(changed()), this, SLOT(updateMenu()) );

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
      QMessageBox::critical( this, tr( "Error on print preview" ), err );
   }
}

void MainWindow::onMenuPrint()
{
   QCDScene *cdscene = getScene( m_mdiArea );
   if( !cdscene )
      return;

   try {
      QDialogProgress::exec( this, cdscene );
   }
   catch( const QString &err ) {
      QMessageBox::critical( this, tr( "Error on print" ), err );
   }
}

void MainWindow::onMenuUndo()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene ) 
      scene->undo();
}

void MainWindow::onMenuRedo()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene )
      scene->redo();
}

void MainWindow::onMenuCopy()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene )
      scene->putItemToClipboard( false );
}

void MainWindow::onMenuCut()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene )
      scene->putItemToClipboard( true );
}

void MainWindow::onMenuPaste()
{
   QCDScene *scene = getScene( m_mdiArea );
   if( scene )
      scene->getItemFromClipboard();
}

void MainWindow::onMenuAbout()
{
   QMessageBox::about( this,
                       tr( "About" ),
                       tr( "<h3>qlscribe - Qt lisghtScribe</h3>"
                           "<p>prerelease 0.12 $Revision$</p>"
                           "<p>visit project at home page "
                           "<a href=\"http://qlscribe.sourceforge.net/\">qlscribe.sourceforge.net</a></p>" ) );
}

void MainWindow::onMenuQtAbout()
{
   QMessageBox::aboutQt( this );
}

void MainWindow::onMenuSettings()
{
   QDialogSettings::exec( this );
}

void MainWindow::updateMenu()
{
   QCDScene *scene = getScene( m_mdiArea );

   m_actionSave->setEnabled( scene );
   m_actionSaveAs->setEnabled( scene );
   m_actionLabelProperties->setEnabled( scene );
   m_actionPrint->setEnabled( scene );
   m_actionPrintPreview->setEnabled( scene );
   m_menuInsert->setEnabled( scene );

   if( !scene ) {
      m_actionUndo->setEnabled( false );
      m_actionRedo->setEnabled( false );
      m_actionCopy->setEnabled( false );
      m_actionCut->setEnabled( false );
      m_actionPaste->setEnabled( false );
   } else {
      m_actionUndo->setEnabled( scene->canUndo() );
      m_actionRedo->setEnabled( scene->canRedo() );

      const QMimeData *mdata = QApplication::clipboard()->mimeData();
      m_actionPaste->setEnabled( mdata && mdata->hasFormat( itemMimeType ) );

      bool selected = scene->selectedItems().count();
      m_actionCopy->setEnabled( selected );
      m_actionCut->setEnabled( selected );
   }
}
