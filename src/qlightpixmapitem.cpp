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

#include "qlightpixmapitem.h"
#include "qdialogpixmap.h"
#include "qcdscene.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

QLightPixmapItem::QLightPixmapItem()
{
}

QVariant QLightPixmapItem::itemChange( GraphicsItemChange change, const QVariant & value )
{
   if( scene() && change == ItemPositionHasChanged )
      static_cast<QCDScene *>( scene() )->setChanged();

   return value;
}


RegisterController< QShapeControllerPixmap > regControllerPixmap;

QString QShapeControllerPixmap::name() const
{
   return "image";
}

QString QShapeControllerPixmap::menuName() const
{
   return QObject::tr( "Image", "QShapeControllerPixmap name" );
}

QGraphicsItem *QShapeControllerPixmap::create() const
{
   QGraphicsPixmapItem *item = new QLightPixmapItem;
   item->setShapeMode( QGraphicsPixmapItem::BoundingRectShape );
   item->setTransformationMode( Qt::SmoothTransformation );

   return item;
}

QItemDialog *QShapeControllerPixmap::createDialog( QWidget *parent ) const
{
   return new QDialogPixmap( parent );
}

void QShapeControllerPixmap::writeData( QXmlStreamWriter &writer, const QGraphicsItem *item ) const
{
   const QGraphicsPixmapItem *pixmapItem
         = static_cast< const QGraphicsPixmapItem * >( item );

   writer.writeEmptyElement( "pos" );
   writer.writeAttribute( QXmlStreamAttribute( "x", QString::number( pixmapItem->pos().x() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "y", QString::number( pixmapItem->pos().y() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "z", QString::number( pixmapItem->zValue() ) ) );

   QPointF scale = pixmapItem->transform().map( QPointF( 1.0, 1.0 ) );
   writer.writeEmptyElement( "scale" );
   writer.writeAttribute(
         QXmlStreamAttribute( "sx", QString::number( scale.x() ) ) );
   writer.writeAttribute(
         QXmlStreamAttribute( "sy", QString::number( scale.y() ) ) );

   writer.writeTextElement( "image", pixmapItem->data( 0 ).toString() );
}


void QShapeControllerPixmap::readData( const QString &element,
                                     const QXmlStreamAttributes &attrs,
                                     const QString &data,
                                     QGraphicsItem *item ) const
{
   QGraphicsPixmapItem *pixmapItem = static_cast< QGraphicsPixmapItem * >( item );

   if( element == "pos" ) {
      pixmapItem->setPos( attrs.value( "x" ).toString().toDouble(),
                          attrs.value( "y" ).toString().toDouble() );
      pixmapItem->setZValue( attrs.value( "z" ).toString().toDouble() );
      return;
   }

   if( element == "scale" ) {
      pixmapItem->setTransform( QTransform().scale( attrs.value( "sx" ).toString().toDouble(),
                                                    attrs.value( "sy" ).toString().toDouble() ) );
   }

   if( element == "image" ) {
      pixmapItem->setData( 0, data );
      QPixmap pixmap( data );
      if( pixmap.isNull() ) {
         throw QString( "QShapeControllerPixmap: image loading from \"" ) +
               data + "\" failed";
      }
      pixmapItem->setPixmap( pixmap );
      pixmapItem->setOffset( -QPointF( pixmap.size().height(), pixmap.size().width()  ) / 2.0 );
      return;
   }
}
