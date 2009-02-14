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

#include "qlighttextitem.h"
#include "qdialogtext.h"
#include "qcdscene.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>


QLightTextItem::QLightTextItem()
{
}

QVariant QLightTextItem::itemChange( GraphicsItemChange change, const QVariant & value )
{
   if( scene() && change == ItemPositionHasChanged )
      static_cast<QCDScene *>( scene() )->setChanged();

   return value;
}


RegisterController< QShapeControllerText > regControllerText;

QString QShapeControllerText::name() const
{
   return "text";
}

QString QShapeControllerText::menuName() const
{
   return QObject::tr( "Text", "QShapeControllerText name" );
}

QGraphicsItem *QShapeControllerText::create() const
{
   QGraphicsItem *item = new QLightTextItem;
   item->setTransform( QTransform().scale( 0.5, 0.5 ), true );
   item->setPos( 0, -40.0 );

   return item;
}

QItemDialog *QShapeControllerText::createDialog( QWidget *parent ) const
{
   return new QDialogText( parent );
}

void QShapeControllerText::writeData( QXmlStreamWriter &writer, const QGraphicsItem *item ) const
{
   const QGraphicsSimpleTextItem *textItem = static_cast< const QGraphicsSimpleTextItem * >( item );
   writer.writeEmptyElement( "pos" );
   writer.writeAttribute( QXmlStreamAttribute( "x", QString::number( textItem->pos().x() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "y", QString::number( textItem->pos().y() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "z", QString::number( textItem->zValue() ) ) );

   writer.writeTextElement( "font", textItem->font().toString() );
   writer.writeTextElement( "color", textItem->brush().color().name() );
   writer.writeTextElement( "text", textItem->text() );
}


void QShapeControllerText::readData( const QString &element,
                                     const QXmlStreamAttributes &attrs,
                                     const QString &data,
                                     QGraphicsItem *item ) const
{
   QGraphicsSimpleTextItem *textItem = static_cast< QGraphicsSimpleTextItem * >( item );

   if( element == "pos" ) {
      textItem->setPos( attrs.value( "x" ).toString().toDouble(),
                        attrs.value( "y" ).toString().toDouble() );
      textItem->setZValue( attrs.value( "z" ).toString().toDouble() );
      return;
   }

   if( element == "font" ) {
      QFont font;
      font.fromString( data );
      textItem->setFont( font );
      return;
   }

   if( element == "color" ) {
      textItem->setBrush( QColor( data ) );
      return;
   }

   if( element == "text" ) {
      textItem->setText( data );
      return;
   }
}
