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

QCDScene::QCDScene( QObject * parent )
   : QGraphicsScene( parent ),
   m_index( 0 ),
   m_saved( false )
{
   setSceneRect( -60.0, -60.0, 60.0 * 2, 60.0 * 2 );
}

//#include <iostream>

QCDScene::~QCDScene()
{
}

bool QCDScene::load( const QString &fileName )
{
   QFile file( fileName );
   if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
      QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot open file for reading\n" ) + fileName );
      return false;
   }
   QXmlStreamReader reader( &file );
   try {
      read( reader );
   }
   catch( const QString &err ) {
      QMessageBox::critical( 0, tr( "Error" ), tr( "Cannot read file " ) + fileName + "\n" + err );
      return false;
   }
   m_fileName = fileName;
   m_saved = true;
   setName();
   return true;
}

void QCDScene::save()
{
   QFile file( m_fileName );
   if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) ) {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open file for writing: " ) + m_fileName );
      return;
   }
   QXmlStreamWriter writer( &file );
   writer.setAutoFormatting( true );
   write( writer );
   m_saved = true;
   setName();
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

void QCDScene::updateTitles() const
{
   QString name = m_name;
   if( !m_saved )
      name += " *";

   QList<QGraphicsView *> allViews = views();
   foreach( QGraphicsView *view, allViews ) {
      view->setWindowTitle( name );
   }
}

void QCDScene::saveAs( const QString &fileName )
{
   QFile file( fileName );
   if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) ) {
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open file for writing: " ) + fileName );
      return;
   }
   QXmlStreamWriter writer( &file );
   writer.setAutoFormatting( true );
   write( writer );
   m_saved = true;
   m_fileName = fileName;
   setName();
}

void QCDScene::write( QXmlStreamWriter &writer )
{
   writer.writeStartDocument();
   writer.writeStartElement( "scene" );

   QShapeFactory &sfactory = QShapeFactory::instance();
   QList<QGraphicsItem *> list = items();
   for( QList<QGraphicsItem *>::const_iterator it = list.begin(); it != list.end(); ++it ) {
      QShapeFactory::iterator f = sfactory.find( (*it)->type() );
      if( f == sfactory.end() ) {
         QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot find controller for type %n", 0, (*it)->type() ) );
         continue;
      }
      f->second->write( writer, *it );
   }

   writer.writeEndElement();
   writer.writeEndDocument();
}

void QCDScene::read( QXmlStreamReader &reader )
{
   bool gotScene = false;

   QShapeFactory &sfactory = QShapeFactory::instance();

   while( !reader.atEnd() ) {
      QXmlStreamReader::TokenType ttype = reader.readNext();
      if( ttype != QXmlStreamReader::StartElement )
         continue;

      QString elementName = reader.name().toString();
      if( !gotScene ) {
         if( elementName != "scene" )
            throw QString( "QCDScene: missing expected root element \"scene\", got \"" )
                  + elementName + "\" intead";

         gotScene = true;
         continue;
      }

      if( elementName == "item" ) {
         QString type = reader.attributes().value( "type" ).toString();
         QShapeFactory::iterator f = sfactory.find( type );
         if( f == sfactory.end() ) {
            QMessageBox::warning( 0,
                                  "Warning",
                                  QString( "QCDScene: unknown item type \"" )
                                  + type + "\"" );
            continue;
         }
         QGraphicsItem *item = f->second->read( reader );
         if( item ) {
            item->setFlag( QGraphicsItem::ItemIsMovable, true );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
            addItem( item );
         }
      } else
         throw QString( "QCDScene: unknown element \"" ) + elementName + "\"";
   }

}

void QCDScene::contextMenuEvent( QGraphicsSceneContextMenuEvent *mouseEvent )
{
   QGraphicsItem *item = itemAt( mouseEvent->scenePos() );
   if( !item )
      return;
   item->setSelected( true );
   QMenu menu;
   menu.addAction( tr( "edit..." ), this, SLOT( onMenuEdit() ) );
   menu.exec( mouseEvent->screenPos() );
}

void QCDScene::onMenuEdit()
{
   QList<QGraphicsItem *> list = selectedItems();
   if( list.empty() )
      return;
   QGraphicsItem *item = list.front();
   QShapeFactory::instance().edit( item, 0 );
}
