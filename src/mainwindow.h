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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

class QLightScribe;
class QMdiArea;
class QSignalMapper;

class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

private slots:
   void onMenuNew();
   void onMenuOpen();
   void onMenuSaveAs();
   void onMenuInsert( int id );
   void onMenuAbout();
   void onMenuQtAbout();

private:
   QLightScribe   *m_lscribe;
   QMdiArea       *m_mdiArea;

   QMenu          *m_menuFile;
   QMenu          *m_menuInsert;
   QMenu          *m_menuHelp;

   QSignalMapper  *m_insertMapper;
};

#endif // MAINWINDOW_H
