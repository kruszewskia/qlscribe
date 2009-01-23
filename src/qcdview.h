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

#ifndef QCDVIEW_H
#define QCDVIEW_H

#include <QGraphicsView>

class QPixmap;
class QCDScene;

class QCDView : public QGraphicsView {
   //Q_OBJECT
public:
   QCDView( QWidget *parent = 0 );
   virtual ~QCDView();
   //QCDView( QGraphicsScene * scene, QWidget * parent = 0 );

   void setScene( QCDScene *scene );
   QCDScene *scene() const;

   virtual QSize sizeHint () const;

protected:

   void drawForeground ( QPainter * painter, const QRectF & rect );
   void drawBackground ( QPainter * painter, const QRectF & rect );
   void resizeEvent ( QResizeEvent * event );

private:
   void drawCD( QPainter * painter, const QRectF & rect, bool alpha );

   QPixmap *m_mask;
};

#endif // QCDVIEW_H
