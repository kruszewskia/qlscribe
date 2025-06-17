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

#ifndef QDIALOGSETTINGS_H
#define QDIALOGSETTINGS_H

#include <QDialog>

namespace Ui {
    class QDialogSettings;
}

class QDialogSettings : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(QDialogSettings)
public:

   static bool exec( QWidget *parent = 0 );
   static void setLastUsedFont( const QFont &font );
   static QFont defaultFont();

private:
    explicit QDialogSettings( QWidget *parent = 0 );
    virtual ~QDialogSettings();

protected:
    virtual void changeEvent(QEvent *e);

private slots:
   void onFont();
   void onLastUsedFont();

private:
    Ui::QDialogSettings *m_ui;
    QFont                m_font;
    static QFont        *m_lastUsedFont;
};

#endif // QDIALOGSETTINGS_H
