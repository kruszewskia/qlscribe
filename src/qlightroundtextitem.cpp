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

#include "qlightroundtextitem.h"
#include "qdialogroundtext.h"
#include "qcdscene.h"

#include <QPainter>
#include <QFontMetrics>
#include <QStyleOptionGraphicsItem>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QTextLayout>

#include <math.h>

QLightRoundTextItem::QLightRoundTextItem( QGraphicsItem *parent )
   : QAbstractGraphicsShapeItem( parent ),
   m_radius( 40.0 ),
   m_angle( 0.0 ),
   m_alignment( Qt::AlignLeft ),
   m_outside( true )
{
   setBrush( Qt::black );
}

const QFont & QLightRoundTextItem::font() const
{
   return m_font;
}

const QString & QLightRoundTextItem::text() const
{
   return m_text;
}

double QLightRoundTextItem::radius() const
{
   return m_radius;
}

double QLightRoundTextItem::angle() const
{
   return m_angle;
}

Qt::Alignment QLightRoundTextItem::alignment() const
{
   return m_alignment;
}

bool QLightRoundTextItem::outside() const
{
   return m_outside;
}

void QLightRoundTextItem::setFont( const QFont &font )
{
   prepareGeometryChange();
   m_font = font;
}

void QLightRoundTextItem::setText( const QString &text )
{
   prepareGeometryChange();
   m_text = text;
}

void QLightRoundTextItem::setRadius( double radius )
{
   prepareGeometryChange();
   m_radius = radius;
}

void QLightRoundTextItem::setAngle( double angle )
{
   prepareGeometryChange();
   m_angle = angle;
}

void QLightRoundTextItem::setAlignment( Qt::Alignment alignment )
{
   prepareGeometryChange();
   m_alignment = alignment;
}

void QLightRoundTextItem::setOutside( bool outside )
{
   prepareGeometryChange();
   m_outside = outside;
}

QRectF QLightRoundTextItem::boundingRect() const
{
   double height = m_radius;
   if( m_outside ) {
      QFontMetrics mcs( m_font );
      height += ( mcs.height() * 2 ) / 3;
   }
   height += 1; // pad
   return QRectF( -height, -height, 2 * height, 2 * height );
}

QPainterPath QLightRoundTextItem::shape() const
{
   QFont font( m_font );
   font.setPointSizeF( m_font.pointSizeF() / 2 );
   QFontMetrics mcs( font );
   double arcAngle = 0.0;
   const double pad = 1;

   for( int i = 0; i < m_text.size(); ++i )
      arcAngle += mcs.width( m_text[ i ] );

   double height = ( mcs.height() * 2 ) / 3;
   double in, out;
   if( m_outside ) {
      in = m_radius - pad;
      out = m_radius + height + pad;
   } else {
      in = m_radius - height - pad;
      out = m_radius + pad;
   }
   arcAngle /= in * 3.14159 / 180.0;
   double padAngle = pad * 180.0 / ( in * 3.14159 );

   int sign = m_outside ? 1 : -1;

   double angle = m_angle;
   switch( m_alignment ) {
      case Qt::AlignCenter :
      case Qt::AlignHCenter :
         angle -= sign * arcAngle / 2;
         break;
      case Qt::AlignRight :
         angle -= sign * arcAngle;
         break;
   }

   QPainterPath path;
   double radAngle = ( angle - sign * padAngle ) * 3.14159 / 180.0;
   path.moveTo( sign * out * sin( radAngle ), -sign * out * cos( radAngle ) );
   path.arcTo( -out, -out, out * 2, out * 2,
               sign * ( 90 + padAngle ) - angle,
               -sign * ( arcAngle + 2 * padAngle ) );
   path.arcTo( -in, -in, in * 2, in * 2,
               sign * ( 90 - padAngle - arcAngle ) - angle,
               sign * ( arcAngle + 2 * padAngle ) );
   path.closeSubpath();

   return path;
}


void QLightRoundTextItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
   Q_UNUSED( option );
   Q_UNUSED( widget );
   if( !painter || !m_radius ) return;
   painter->save();

   QFont font( m_font );
   font.setPointSizeF( m_font.pointSizeF() / 2 );
   painter->setFont( font );

   //painter->drawEllipse( - m_radius, - m_radius, m_radius * 2, m_radius * 2 );

   QFontMetrics mcs( font );

   const double radi = m_radius - ( m_outside ? 0 : mcs.xHeight() );
   const double diam = 2 * radi;
   double angle = m_angle * 3.14159 / 180.0;
   switch( m_alignment ) {
      case Qt::AlignCenter :
      case Qt::AlignHCenter :
         angle += mcs.width( m_text ) / diam * ( m_outside ? -1 : 1 );
         break;
      case Qt::AlignRight :
         angle += mcs.width( m_text ) / radi * ( m_outside ? -1 : 1 );
         break;
   }

   QTransform trans;
   trans.rotateRadians( angle );
   painter->setRenderHint( QPainter::TextAntialiasing, true );
   painter->setWorldTransform( trans, true );

   QPen p;
   p.setBrush( brush() );
   painter->setPen( p );
   painter->setBrush( Qt::NoBrush );

   int prevWidth = 0;
   for( int i = 0; i < m_text.size(); ++i ) {
      QChar ch = m_text[ i ];

      double d = ( prevWidth + mcs.width( ch ) ) / diam;
      trans.reset();
      trans.rotateRadians( m_outside ? d : -d );
      painter->setWorldTransform( trans, true );

      prevWidth = mcs.width( ch );

      painter->drawText( int( -prevWidth / 2.0 ),
                         int( m_outside ? -m_radius : m_radius ),
                         ch );
   }
   painter->restore();
   if (option->state & QStyle::State_Selected) {
      painter->setPen( QPen( option->palette.windowText(), 0, Qt::DashLine ) );
      painter->setBrush(Qt::NoBrush);
      painter->drawPath( shape() );
   }
}

QVariant QLightRoundTextItem::itemChange( GraphicsItemChange change, const QVariant & value )
{
   if( scene() && change == ItemPositionHasChanged )
      static_cast<QCDScene *>( scene() )->setChanged();

   return value;
}

RegisterController< QShapeControllerRoundText > regControllerRoundText;

QString QShapeControllerRoundText::name() const
{
   return "roundText";
}

QString QShapeControllerRoundText::menuName() const
{
   return QObject::tr( "Round Text", "QShapeControllerRoundText name" );
}

QGraphicsItem *QShapeControllerRoundText::create() const
{
   QLightRoundTextItem *item = new QLightRoundTextItem;
   return item;
}

QItemDialog *QShapeControllerRoundText::createDialog( QWidget *parent ) const
{
   return new QDialogRoundText( parent );
}

void QShapeControllerRoundText::replace( QGraphicsItem *item, const QString &from, const QString &to ) const
{
   QLightRoundTextItem *textItem = static_cast< QLightRoundTextItem * >( item );
   bool placeHolder = textItem->text().left( 1 ) == "?" && textItem->text().right( 1 ) == "?";

   bool replace = false;
   if( from.isEmpty() )
      replace = placeHolder;
   else
      replace = textItem->text() == ( placeHolder ? QString( "?%1?" ).arg( from ) : from );

   if( replace )
      textItem->setText( to );
}

void QShapeControllerRoundText::writeData( QXmlStreamWriter &writer, const QGraphicsItem *item ) const
{
   const QLightRoundTextItem *textItem = static_cast< const QLightRoundTextItem * >( item );
   writer.writeEmptyElement( "pos" );
   writer.writeAttribute( QXmlStreamAttribute( "x", QString::number( textItem->pos().x() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "y", QString::number( textItem->pos().y() ) ) );
   writer.writeAttribute( QXmlStreamAttribute( "z", QString::number( textItem->zValue() ) ) );

   writer.writeTextElement( "radius", QString::number( textItem->radius() ) );
   writer.writeTextElement( "angle", QString::number( textItem->angle() ) );
   writer.writeTextElement( "alignment", QString::number( textItem->alignment() ) );
   writer.writeTextElement( "location", QString::number( textItem->outside() ) );
   writer.writeTextElement( "font", textItem->font().toString() );
   writer.writeTextElement( "color", textItem->brush().color().name() );
   writer.writeTextElement( "text", textItem->text() );
}

void QShapeControllerRoundText::readData( const QString &element,
                                          const QXmlStreamAttributes &attrs,
                                          const QString &data,
                                          QGraphicsItem *item ) const
{
   QLightRoundTextItem *textItem = static_cast< QLightRoundTextItem * >( item );

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

   if( element == "location" ) {
      textItem->setOutside( data.toInt() );
      return;
   }

   if( element == "radius" ) {
      textItem->setRadius( data.toDouble() );
      return;
   }

   if( element == "angle" ) {
      textItem->setAngle( data.toDouble() );
      return;
   }

   if( element == "alignment" ) {
      textItem->setAlignment( Qt::Alignment( data.toInt() ) );
      return;
   }
}

