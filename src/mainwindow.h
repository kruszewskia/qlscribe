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

class QMdiArea;
class QSignalMapper;
class QCDScene;

class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
public:
    explicit MainWindow( bool enablePrint );
    virtual ~MainWindow();

   bool saveScene( QCDScene *scene );
   void open( const QStringList &files );

protected:
    virtual void closeEvent( QCloseEvent *event );

private slots:
   void onMenuNewLabel( int mode );
   void onMenuOpen();
   void onMenuSave();
   void onMenuSaveAs();
   void onMenuProperties();
   void onMenuPrintPreview();
   void onMenuPrint();
   void onMenuUndo();
   void onMenuRedo();
   void onMenuCopy();
   void onMenuCut();
   void onMenuPaste();
   void onMenuSettings();
   void onMenuInsert( int id );
   void onMenuAbout();
   void onMenuQtAbout();

   void updateMenu();

private:
   bool saveSceneAs( QCDScene *scene );

   QMdiArea       *m_mdiArea;

   QMenu          *m_menuFile;
   QMenu          *m_menuEdit;
   QMenu          *m_menuInsert;
   QMenu          *m_menuHelp;

   QAction        *m_actionSave;
   QAction        *m_actionSaveAs;
   QAction        *m_actionLabelProperties;
   QAction        *m_actionPrint;
   QAction        *m_actionPrintPreview;
   QAction        *m_actionUndo;
   QAction        *m_actionRedo;
   QAction        *m_actionCopy;
   QAction        *m_actionCut;
   QAction        *m_actionPaste;
   QAction        *m_actionSettings;

   QSignalMapper  *m_insertMapper;
   QSignalMapper  *m_newLabelMapper;
};

#endif // MAINWINDOW_H
