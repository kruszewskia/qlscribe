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

#ifndef QLIGHTPIXMAPITEM_H
#define QLIGHTPIXMAPITEM_H

#include <QGraphicsPixmapItem>

#include "qshapefactory.h"

class QLightPixmapItem : public QGraphicsPixmapItem
{
public:
    QLightPixmapItem();

    void imageName( const QString &name ) { m_imageName = name; }
    const QString &imageName() const { return m_imageName; }

protected:
   virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value );

private:
   QString m_imageName;
};

class QShapeControllerPixmap : public QShapeController {
public:
   enum { Type = QLightPixmapItem::Type };

   virtual QString name() const;
   virtual QString menuName() const;

   virtual QGraphicsItem *create() const;

protected:
   virtual QItemDialog *createDialog( QWidget *parent ) const;
   virtual void writeData( QXmlStreamWriter &writer, const QGraphicsItem *item ) const;
   virtual void readData( const QString &element,
                          const QXmlStreamAttributes &attrs,
                          const QString &data,
                          QGraphicsItem *item ) const;
};


#endif // QLIGHTPIXMAPITEM_H
