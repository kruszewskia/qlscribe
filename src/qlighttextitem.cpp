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
#include "qdialogsettings.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextDocument>


QLightTextItem::QLightTextItem()
{
}

QVariant QLightTextItem::itemChange( GraphicsItemChange change, const QVariant & value )
{
   if( scene() && change == ItemPositionHasChanged )
      static_cast<QCDScene *>( scene() )->itemMoved();

   return value;
}

QString QLightTextItem::text() const
{
    return QGraphicsSimpleTextItem::text();
}

QColor QLightTextItem::color() const
{
    return this->brush().color();
}

Qt::Alignment QLightTextItem::alignment() const
{
    return m_alignment;
}

void QLightTextItem::setText( const QString &txt )
{
    QGraphicsSimpleTextItem::setText( txt );
    updateTrans();
}

void QLightTextItem::setFont( const QFont &font )
{
    QGraphicsSimpleTextItem::setFont( font );
    updateTrans();
}


void QLightTextItem::setColor( const QColor &color )
{
    this->setBrush( color );
}


void QLightTextItem::setAlignment( Qt::Alignment align )
{
    m_alignment = align;
    updateTrans();
}

void QLightTextItem::updateTrans()
{
    double size = this->boundingRect().width();

    QTransform trans;
    trans.scale(  0.5, 0.5 );
    switch( m_alignment ) {
    case Qt::AlignRight :   trans.translate( -size, 0 ); break;
    case Qt::AlignCenter :  trans.translate( -size / 2.0, 0 ); break;
    }
    this->setTransform( trans );
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
   QLightTextItem *item = new QLightTextItem;
   item->setFont( QDialogSettings::defaultFont() );
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
   const QLightTextItem *textItem = static_cast< const QLightTextItem * >( item );
   writer.writeEmptyElement( "pos" );
   writer.writeAttribute( QXmlStreamAttribute( "x", QString::number( textItem->pos().x() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "y", QString::number( textItem->pos().y() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "z", QString::number( textItem->zValue() ) ) );

   writer.writeTextElement( "font", textItem->font().toString() );
   writer.writeTextElement( "color", textItem->color().name() );
   writer.writeTextElement( "text", textItem->text() );
   writer.writeTextElement( "alignment", QString::number( textItem->alignment() ) );
}

void QShapeControllerText::replace( QGraphicsItem *item, const QString &from, const QString &to ) const
{
   QLightTextItem *textItem = static_cast< QLightTextItem * >( item );

   bool placeHolder = textItem->text().left( 1 ) == "?" && textItem->text().right( 1 ) == "?";

   bool replace = false;
   if( from.isEmpty() )
      replace = placeHolder;
   else
      replace = textItem->text() == ( placeHolder ? QString( "?%1?" ).arg( from ) : from );

   if( replace )
      textItem->setText( to );
}


void QShapeControllerText::readData( const QString &element,
                                     const QXmlStreamAttributes &attrs,
                                     const QString &data,
                                     QGraphicsItem *item ) const
{
   QLightTextItem *textItem = static_cast< QLightTextItem * >( item );

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
      textItem->setColor( QColor( data ) );
      return;
   }

   if( element == "text" ) {
      textItem->setText( data );
      return;
   }

   if( element == "alignment" ) {
      textItem->setAlignment( Qt::Alignment( data.toInt() ) );
      return;
   }
}
