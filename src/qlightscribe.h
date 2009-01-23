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

#ifndef QLIGHTSCRIBE_H
#define QLIGHTSCRIBE_H

#include <QThread>
#include <QPixmap>

class QCDScene;
class QMutex;
class QWaitCondition;

class QLightDrive {
public:
   const QString &productName() const { return m_productName; }
   const QString &vendorName() const { return m_vendorName; }
   const QString &displayName() const { return m_displayName; }
   const QString &path() const { return m_path; }

   double innerRadius() const { return m_innerRadius; }
   double outerRadius() const { return m_outerRadius; }

   friend class QLightScribe;
private:
   QString m_productName;
   QString m_vendorName;
   QString m_displayName;
   QString m_path;

   double m_innerRadius;
   double m_outerRadius;
};

class QLightScribe : public QThread {
   Q_OBJECT
public:
   QLightScribe( QObject *parent = 0 );
   virtual ~QLightScribe();

   QList< QLightDrive * > getDrives( bool refresh = false );
   QPixmap preview( QLightDrive *drive, QCDScene *scene, const QSize &size );
   void print( QLightDrive *drive, QCDScene *scene );

public slots:
   void abort();
   void stopThread();

signals:
   void prepareProgress( long current, long final );
   void labelProgress( long current, long final );
   void timeEstimate( long time );
   void finished( int status );

protected:
   virtual void run ();

private:
   struct Task;

   QList< QLightDrive * > m_drives;
   Task                  *m_task;
   bool                   m_aborted;
   QMutex                *m_mutex;
   QWaitCondition        *m_waitQueue;
   QWaitCondition        *m_waitDone;
};

#endif // QLIGHTSCRIBE_H
