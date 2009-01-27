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

#ifndef QDIALOGPROGRESS_H
#define QDIALOGPROGRESS_H

#include <QtGui/QDialog>

namespace Ui {
    class QDialogProgress;
}

class QCDScene;
class QAbstractButton;
class QTime;
class QTimer;

class QDialogProgress : public QDialog {
   Q_OBJECT
   Q_DISABLE_COPY(QDialogProgress)
public:
   static bool exec( QWidget *parent, QCDScene *scene );

protected slots:
   void onButtonClicked( QAbstractButton* button );
   void onPrepareProgress( long current, long final );
   void onLabelProgress( long current, long final );
   void onTimeEstimate( long time );
   void onFinished(int status );
   void onTimeout();

protected:
    virtual void changeEvent(QEvent *e);

private:
    explicit QDialogProgress(QWidget *parent = 0);
    virtual ~QDialogProgress();

    Ui::QDialogProgress *m_ui;
    QTime               *m_start;
    QTimer              *m_timer;
};

#endif // QDIALOGPROGRESS_H
