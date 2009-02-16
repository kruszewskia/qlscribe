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

#ifndef QLIGHTROUNDTEXTITEM_H
#define QLIGHTROUNDTEXTITEM_H

#include "qshapefactory.h"

#include <QAbstractGraphicsShapeItem>
#include <QFont>

class QLightRoundTextItem : public QAbstractGraphicsShapeItem {
public:
   enum { Type = UserType + 1 };

    QLightRoundTextItem( QGraphicsItem * parent = 0 );

    const QFont &font() const;
    const QString &text() const;
    double radius() const;
    double angle() const;
    Qt::Alignment alignment() const;
    bool outside() const;

    void setFont( const QFont &font );
    void setText( const QString &text );
    void setRadius( double radius );
    void setAngle( double angle );
    void setAlignment( Qt::Alignment alignment );
    void setOutside( bool outside );

    virtual QRectF boundingRect() const;
    virtual QPainterPath shape () const;
    virtual void paint( QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget );

    virtual int type() const { return Type; }

protected:
   virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value );

private:
    QFont         m_font;
    QString       m_text;
    double        m_radius;
    double        m_angle;
    Qt::Alignment m_alignment;
    bool          m_outside;
};

class QShapeControllerRoundText : public QShapeController {
public:
   enum { Type = QLightRoundTextItem::Type };

   virtual QString name() const;
   virtual QString menuName() const;

   virtual QGraphicsItem *create() const;
   virtual void replace( QGraphicsItem *item, const QString &from, const QString &to ) const;

protected:
   virtual QItemDialog *createDialog( QWidget *parent ) const;
   virtual void writeData( QXmlStreamWriter &writer, const QGraphicsItem *item ) const;
   virtual void readData( const QString &element,
                          const QXmlStreamAttributes &attrs,
                          const QString &data,
                          QGraphicsItem *item ) const;
};


#endif // QLIGHTROUNDTEXTITEM_H
