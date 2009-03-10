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

#ifndef QDIALOGPIXMAP_H
#define QDIALOGPIXMAP_H

#include "qshapefactory.h"

namespace Ui {
    class QDialogPixmap;
}

class QLightPixmapItem;

class QDialogPixmap : public QItemDialog {
    Q_OBJECT
    Q_DISABLE_COPY(QDialogPixmap)
public:
    explicit QDialogPixmap( QWidget *parent );
    virtual ~QDialogPixmap();

    virtual bool exec( QGraphicsItem *item );

protected:
    virtual void changeEvent(QEvent *e);

private slots:
   void onLoadImage();
   void posChanged();
   void onResetSize();
   void sizeChanged();

private:
    Ui::QDialogPixmap *m_ui;
    QLightPixmapItem  *m_item;
};


#endif // QDIALOGPIXMAP_H
