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

    $Id:$ */

#include "mainwindow.h"
#include "qgraphicsroundtextitem.h"
#include "qcdview.h"
#include "qcdscene.h"
#include "qshapefactory.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QSignalMapper>
#include <QFileDialog>
#include <QXmlStreamWriter>

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

   m_menuFile->addAction( tr( "Save as...", "Menu item \"Save as\"" ),
                          this,
                          SLOT(onMenuSaveAs()) );

   m_menuInsert = menuBar()->addMenu( tr( "Insert", "Menu item \"Insert\"" ) );


    //m_ui->graphicsView->setScene( m_scene );
    //m_ui->graphicsView->fitInView( m_scene->sceneRect(), Qt::KeepAspectRatioByExpanding );
    //m_ui->graphicsView->centerOn( 0, 0 );
    //m_ui->graphicsView->scale( 10, 10 );
    //m_ui->graphicsView->setInteractive( true );
    //QFont font = QFontDialog::getFont( 0 );
    //font.setPointSizeF( font.pointSizeF() / 2.0 ) ;
    //m_scene->addSimpleText( "Test and test and test again", font );

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

/*void MainWindow::onClicked2()
{
   QGraphicsRoundTextItem *titem = new QGraphicsRoundTextItem;
   titem->setText( "Hello world!" );
   titem->setRadius( 50 );
   titem->setFont( QFont("Helvetica", 8 ) );
   static int a = 0;
   titem->setAlignment( Qt::AlignCenter );
   titem->setOutside( false );
   titem->setAngle( a );
   a += 90;

   //m_scene->addItem( titem );
}*/

/*void MainWindow::onClicked3()
{
   QGraphicsRoundTextItem *titem = new QGraphicsRoundTextItem;
   titem->setText( "Hello world!" );
   titem->setRadius( 50 );
   titem->setFont( QFont("Helvetica", 2 ) );
   static int a = 0;
   titem->setAlignment( Qt::AlignCenter );
   titem->setOutside( true );
   titem->setAngle( a );
   a += 90;

   //m_scene->addItem( titem );
}*/

/*void checkLS( LSError err, const QString &name )
{
   if( err == LS_OK )
      return;

   QMessageBox::warning( 0, "Failed", name + QString( " failed: 0x" ) + QString::number( err,16 ) );
   throw 1;
}

QString color( const LS_ColorRGB &col )
{
   return QString::number( col.red, 16 ) + QString::number( col.green, 16 ) + QString::number( col.blue, 16 );
   }*/

/*
const size_t bitmapHeaderSize = 54;

void MainWindow::onClicked()
{
   QImage image( 2772, 2772, QImage::Format_RGB888 );
   //QImage image( 800, 800, QImage::Format_RGB888 );
   image.fill( 0xFFFFFFFF );

   QPainter painter( &image );
   m_scene->render( &painter, image.rect() );

   image.setDotsPerMeterX( 23622 );
   image.setDotsPerMeterY( 23622 );

   QByteArray ba;
   QBuffer buffer( &ba );
   buffer.open( QIODevice::WriteOnly );
   image.save( &buffer, "bmp", 100 );

   {
     std::ofstream of( "test1.bmp" );
     of.write( ba.constData(), ba.size() );
   }
   //return;

   //QMessageBox::information( this, "Buf size", QString::number( ba.size() ) );


   try {
      LS_DiscPrintMgrHandle mgrHandle;
      checkLS( LS_DiscPrintMgr_Create( &mgrHandle ), "LS_DiscPrintMgr_Create" );

      LS_EnumDiscPrintersHandle enumHandle;
      checkLS( LS_DiscPrintMgr_EnumDiscPrinters( mgrHandle, &enumHandle ), "LS_DiscPrintMgr_EnumDiscPrinters" );

      //unsigned long count = 0;
      //checkLS( LS_EnumDiscPrinters_Count( enumHandle, &count ), "LS_EnumDiscPrinters_Count" );
      //QMessageBox::information( this, "Count", QString::number( count ) );
      LS_DiscPrinterHandle printerHandle;
      checkLS( LS_EnumDiscPrinters_Item( enumHandle, 0, &printerHandle ), "LS_EnumDiscPrinters_Item" );

      LS_DiscPrintSessionHandle sessionHandle;
      checkLS( LS_DiscPrinter_OpenPrintSession( printerHandle, &sessionHandle ), "LS_DiscPrinter_OpenPrintSession" );

      LS_Size size;
      size.x = 1000;
      size.y = 1000;
      checkLS( LS_DiscPrintSession_PrintPreview( sessionHandle,
                                                 LS_windows_bitmap,
                                                 LS_label_mode_full,
                                                 LS_draw_default,
                                                 LS_quality_normal,
                                                 LS_media_recognized,
                                                 ba.data() + 14, bitmapHeaderSize - 14,
                                                 ba.data() + bitmapHeaderSize, ba.size() - bitmapHeaderSize,
                                                 "test.bmp", LS_windows_bitmap, size, false ),
               "LS_DiscPrintSession_PrintPreview" );

      checkLS( LS_DiscPrintSession_Destroy( &sessionHandle ), "LS_DiscPrintSession_Destroy" );

      checkLS( LS_DiscPrinter_Destroy( &printerHandle ), "LS_DiscPrinter_Destroy" );
      checkLS( LS_EnumDiscPrinters_Destroy( &enumHandle ), "LS_EnumDiscPrinters_Destroy" );
      checkLS( LS_DiscPrintMgr_Destroy( &mgrHandle ), "LS_DiscPrintMgr_Destroy" );
   }
   catch( int ) {
   }

}
*/
