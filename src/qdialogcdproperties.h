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


#ifndef QDIALOGCDPROPERTIES_H
#define QDIALOGCDPROPERTIES_H

#include <QDialog>

namespace Ui {
    class QDialogCDProperties;
}

class QCDScene;

class QDialogCDProperties : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(QDialogCDProperties)
public:
   static bool exec( QWidget *parent, QCDScene *scene );

protected:
   virtual void changeEvent(QEvent *e);

private slots:
   void onChangeMode();
   void onColor();

private:
   explicit QDialogCDProperties(QWidget *parent = 0);
   virtual ~QDialogCDProperties();

   Ui::QDialogCDProperties *m_ui;
};

#endif // QDIALOGCDPROPERTIES_H
