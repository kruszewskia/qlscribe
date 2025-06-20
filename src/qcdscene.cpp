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

#include "qcdscene.h"
#include "qshapefactory.h"
#include "qlightpixmapitem.h"

#include <QMenu>
#include <QMessageBox>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QGraphicsView>
#include <QRegExp>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QRegExp>

QCDScene::QCDScene( QObject * parent )
   : QGraphicsScene( parent ),
   m_index( 0 ),
   m_saved( true ),
   m_itemMoved( false ),
   m_labelMode( modeFull ),
   m_cdColor( Qt::white ),
   m_undoPosition( m_undoBuffer.end() )
{
   setSceneRect( -60.0, -60.0, 60.0 * 2, 60.0 * 2 );

}

//#include <iostream>

QCDScene::~QCDScene()
{
}

void QCDScene::replace( const QString2String &strings )
{
   QShapeFactory &sfactory = QShapeFactory::instance();
   QList<QGraphicsItem *> list = items();

   for( QString2String::const_iterator st = strings.begin(); st != strings.end(); ++st ) {
      foreach(QGraphicsItem *item, list ) {
         QShapeFactory::iterator f = sfactory.find( item->type() );
         if( f != sfactory.end() )
            f->second->replace( item, st.key(), st.value() );
      }
   }
   foreach(QGraphicsItem *item, list ) {
      QShapeFactory::iterator f = sfactory.find( item->type() );
      if( f != sfactory.end() )
         f->second->replace( item, "", "" ); // to clean placeholders
   }
}

void QCDScene::start( LabelMode mode )
{
   setLabelMode( mode );
   pushUndo();
}

bool QCDScene::load( const QString &fileName, QString *errMessage )
{
   if( QRegExp( ".*.\\.qlx$", Qt::CaseInsensitive ).exactMatch( fileName ) ) {
      QFile file( fileName );
      if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
         if( errMessage )
            *errMessage = "cannot open file for reading";
         else
            QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot open file for reading\n" ) + fileName );
         return false;
      }
      QXmlStreamReader reader( &file );
      try {
         read( reader );
      }
      catch( const QString &err ) {
         if( errMessage )
            *errMessage = err;
         else
            QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot read file " ) + fileName + "\n" + err );
         return false;
      }
      m_fileName = fileName;
      m_saved = true;
      setName();

      pushUndo();

      QList<QGraphicsItem *> list = items();
      bool oldFormat = true;
      foreach(QGraphicsItem *item, list ) {
         if( item->zValue() != 0.0 ) {
            oldFormat = false;
            break;
         }
      }
      if( oldFormat ) {
         double z = 1.0;
         foreach(QGraphicsItem *item, list ) {
            item->setZValue( z );
            z += 0.1;
         }
      }

      return true;
   }
   QShapeFactory &sfactory = QShapeFactory::instance();
   QLightPixmapItem *item =
         static_cast<QLightPixmapItem *>(sfactory.create( QShapeControllerPixmap::Type ) );
   addItem( item, true );
   if( item->loadImage( fileName ) ) {
      QSize size = item->pixmap().size();
      item->setOffset( -QPointF( size.height(), size.width()  ) / 2.0 );
      item->setTransform( QTransform().scale( size.width() ? 120.0 / size.width() : 1.0,
                                             size.height() ? 120.0 / size.height() : 1.0 ) );
      setName();
      pushUndo();
      return true;
   } else {
      if( errMessage )
         *errMessage = "cannot load image";
      else
         QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot load image " ) +
                                fileName + "\n" );
   }
   return false;
}


bool QCDScene::save()
{
   QFile file( m_fileName );
   if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) ) {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open file for writing: " ) + m_fileName );
      return false;
   }
   QXmlStreamWriter writer( &file );
   writer.setAutoFormatting( true );
   write( writer );
   m_saved = true;
   setName();
   return true;
}

void QCDScene::setLabelMode( LabelMode mode )
{
   m_labelMode = mode;
   update();
   setChanged();
}

void QCDScene::setCDColor( const QColor &color )
{
   m_cdColor = color;
   update();
   setChanged();
}

void QCDScene::setName()
{
   if( m_fileName.isEmpty() ) {
      if( !m_index ) {
         static int index = 1;
         m_index = index++;
      }
      m_name = "Unnamed_" + QString::number( m_index );
   } else {
      QRegExp rx( "(.*/)?(.*)\\.qlx" );
      rx.indexIn( m_fileName );
      m_name = rx.cap( 2 );
   }

   updateTitles();
}

void QCDScene::setChanged( bool undo )
{
   m_saved = false;
   updateTitles();

   if( undo && m_undoPosition != m_undoBuffer.end() )
      pushUndo();

   emit changed();
}

void QCDScene::pushUndo()
{
   if( canRedo() ) {
      UndoBuffer::iterator pos = m_undoBuffer.begin() + 1 + ( m_undoPosition - m_undoBuffer.begin() );
      m_undoBuffer.erase( pos, m_undoBuffer.end() );
   }

   QString data;
   QXmlStreamWriter writer( &data );
   write( writer );
   m_undoBuffer.push_back( data );
   if( m_undoBuffer.size() > 20 )
      m_undoBuffer.pop_front();

   m_undoPosition = m_undoBuffer.end() - 1;
}

void QCDScene::updateTitles() const
{
   QString name = tr( "Label: ", "Window title \"Label\"" ) + m_name;
   if( !m_saved )
      name += " *";

   QList<QGraphicsView *> allViews = views();
   foreach( QGraphicsView *view, allViews ) {
      view->setWindowTitle( name );
   }
}

void QCDScene::redrawViews() const
{
}

bool QCDScene::saveAs( const QString &fileName )
{
   QFile file( fileName );
   if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) ) {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open file for writing: " ) + fileName );
      return false;
   }
   QXmlStreamWriter writer( &file );
   writer.setAutoFormatting( true );
   write( writer );
   m_saved = true;
   m_fileName = fileName;
   setName();
   return true;
}

void QCDScene::write( QXmlStreamWriter &writer )
{
   writer.writeStartDocument();
   writer.writeStartElement( "scene" );
   writer.writeAttribute( QXmlStreamAttribute( "mode", QString::number( m_labelMode ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "color", m_cdColor.name() ) );

   QShapeFactory &sfactory = QShapeFactory::instance();
   QList<QGraphicsItem *> list = items();
   foreach(QGraphicsItem *item, list ) {
      QShapeFactory::iterator f = sfactory.find( item->type() );
      if( f == sfactory.end() ) {
         QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find controller for type %n", 0, item->type() ) );
         continue;
      }
      f->second->write( writer, item );
   }

   writer.writeEndElement();
   writer.writeEndDocument();
}

void QCDScene::addItem( QGraphicsItem *item, bool top )
{
   if( top ) {
      double topZ = 1.0;
      QList<QGraphicsItem *> list = items();
      foreach(QGraphicsItem *item, list ) {
         if( item->zValue() >= topZ )
            topZ = item->zValue() + 0.1;
      }
      item->setZValue( topZ );
   }
   QGraphicsScene::addItem( item );
}


bool QCDScene::readItem( QXmlStreamReader &reader, bool top )
{
   QShapeFactory &sfactory = QShapeFactory::instance();

   QString type = reader.attributes().value( "type" ).toString();
   QShapeFactory::iterator f = sfactory.find( type );
   if( f == sfactory.end() ) {
      QMessageBox::warning( 0,
                            "Warning",
                            QString( "QCDScene: unknown item type \"" )
                            + type + "\"" );
      return false;
   }
   QGraphicsItem *item = f->second->read( reader );
   if( item ) {
      item->setFlag( QGraphicsItem::ItemIsMovable, true );
      item->setFlag( QGraphicsItem::ItemIsSelectable, true );
      addItem( item, top );
      return true;
   }
   return false;
}

void QCDScene::read( QXmlStreamReader &reader )
{
   bool gotScene = false;

   while( !reader.atEnd() ) {
      QXmlStreamReader::TokenType ttype = reader.readNext();
      if( ttype != QXmlStreamReader::StartElement )
         continue;

      QString elementName = reader.name().toString();
      if( !gotScene ) {
         if( elementName != "scene" )
            throw QString( "QCDScene: missing expected root element \"scene\", got \"" )
                  + elementName + "\" intead";

         m_labelMode = LabelMode( reader.attributes().value( "mode" ).toString().toInt() );
         QString colorStr = reader.attributes().value( "color" ).toString();
         m_cdColor = colorStr.isEmpty() ? Qt::white : QColor( colorStr );
         gotScene = true;
         continue;
      }

      if( elementName == "item" ) {
         readItem( reader, false );
      } else
         throw QString( "QCDScene: unknown element \"" ) + elementName + "\"";
   }

}

void QCDScene::putItemToClipboard( bool move )
{
   QList<QGraphicsItem *> list = selectedItems();
   if( list.empty() )
      return;
   QGraphicsItem *item = list.front();

   QShapeFactory &sfactory = QShapeFactory::instance();
   QShapeFactory::iterator f = sfactory.find( item->type() );
   if( f == sfactory.end() ) {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find controller for type %n", 0, item->type() ) );
      return;
   }

   QByteArray array;
   QXmlStreamWriter writer( &array );

   f->second->write( writer, item );
   QMimeData *mdata = new QMimeData;
   mdata->setData( itemMimeType, array );
   mdata->setText( array.data() );

   QApplication::clipboard()->setMimeData( mdata );

   if( move ) {
      removeItem( item );
      delete item;
      setChanged();
   }
}

void QCDScene::getItemFromClipboard()
{
   const QMimeData *mdata = QApplication::clipboard()->mimeData();
   if( !mdata )
      return;

   if( !mdata->hasFormat( itemMimeType ) )
      return;

   QByteArray array = mdata->data( itemMimeType );
   QXmlStreamReader reader( array );

   while( !reader.atEnd() ) {
      QXmlStreamReader::TokenType ttype = reader.readNext();
      if( ttype != QXmlStreamReader::StartElement )
         continue;

      QString elementName = reader.name().toString();

      if( elementName == "item" ) {
         if( readItem( reader, true ) )
            setChanged();
      }
   }
}

void QCDScene::contextMenuEvent( QGraphicsSceneContextMenuEvent *mouseEvent )
{
   QGraphicsItem *item = itemAt( mouseEvent->scenePos() ,QTransform());
   if( !item )
      return;
   item->setSelected( true );
   QMenu menu;
   menu.addAction( tr( "edit..." ), this, SLOT( onMenuEdit() ) );
   menu.addAction( tr( "copy" ), this, SLOT( onMenuCopy() ) );
   menu.addAction( tr( "cut" ), this, SLOT( onMenuCut() ) );
   menu.addAction( tr( "bring to Front" ), this, SLOT( onMenuToFront() ) );
   menu.addAction( tr( "send to Back" ), this, SLOT( onMenuToBack() ) );
   menu.addAction( tr( "delete" ), this, SLOT( onMenuDelete() ) );
   menu.exec( mouseEvent->screenPos() );
}

void QCDScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
   if( m_itemMoved ) {
      setChanged();
      m_itemMoved = false;
   }

   QGraphicsScene::mouseReleaseEvent( mouseEvent );
}

void QCDScene::onMenuEdit()
{
   QList<QGraphicsItem *> list = selectedItems();
   if( list.empty() )
      return;
   QGraphicsItem *item = list.front();
   QShapeFactory::instance().edit( item, 0 );
}

void QCDScene::onMenuCopy()
{
   putItemToClipboard( false );
}

void QCDScene::onMenuCut()
{
   putItemToClipboard( true );
}

void QCDScene::onMenuToFront()
{
   sendItemTo( true );
}

void QCDScene::onMenuToBack()
{
   sendItemTo( false );
}

void QCDScene::onMenuDelete()
{
   QList<QGraphicsItem *> list = selectedItems();
   if( list.empty() )
      return;
   QGraphicsItem *item = list.front();
   removeItem( item );
   delete item;
   setChanged();
}

void QCDScene::sendItemTo( bool front )
{
   if ( selectedItems().isEmpty())
      return;

   QGraphicsItem *selectedItem = selectedItems().first();
   QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

   qreal zValue = 0;
   foreach( QGraphicsItem *item, overlapItems ) {
      if( front && item->zValue() >= zValue )
         zValue = item->zValue() + 0.1;

      if( !front && item->zValue() <= zValue )
         zValue = item->zValue() - 0.1;
   }
   selectedItem->setZValue( zValue );
   setChanged();
}

bool QCDScene::canUndo() const
{
   return m_undoPosition - m_undoBuffer.begin() > 0;
}

bool QCDScene::canRedo() const
{
   return m_undoBuffer.end() - m_undoPosition > 1;
}

void QCDScene::unredo( bool undo )
{
   if( undo ) {
      if( !canUndo() )
      return;
      --m_undoPosition;
   } else {
      if( !canRedo() )
         return;
      ++m_undoPosition;
   }

   try {
      clear();
      QXmlStreamReader reader( *m_undoPosition );
      read( reader );
   }
   catch(...) {
   }
   setChanged( false );
}

