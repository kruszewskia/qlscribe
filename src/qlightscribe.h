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
   enum LabelMode {
      /** label the entire disc */
      modeFull=0,
      /** label within the title mode constraints */
      modeTitle=1,
      /** label within the content mode constraints */
      modeContent=2
   };

   enum DrawOptions {
      /** disable scaling of bitmaps; they will be cropped if needed */
      drawDefault=0,
      /** Fit the height to the label size */
      drawFitHeightToLabel=1,
      /** Fit the width to the label size */
      drawFitWidthToLabel=2,
      /** Fit the smallest dimension to the label size */
      drawFitSmallestToLabel=4
   };

   enum PrintQuality {
      /** Best and slowest. */
      qualityBest=0,
      /** OK for everyday use. */
      qualityNormal=1,
      /** Fast but lower contrast. */
      qualityDraft=2
   };

   enum MediaOptimizationLevel {
      /** Require that media is present and optimized labeling
         * parameters are available */
      mediaRecognized,
      /** Require that media is present but optimized labeling
         * parameters are not available */
      mediaGeneric
   };

   struct PrintParameters {
      LabelMode    m_labelMode;
      DrawOptions  m_drawOptions;
      PrintQuality m_printQuality;
      MediaOptimizationLevel m_mediaOptimizationLevel;

      PrintParameters()
            : m_labelMode( modeFull ), m_drawOptions( drawDefault ),
            m_printQuality( qualityBest ), m_mediaOptimizationLevel( mediaRecognized )  {}
      PrintParameters( LabelMode labelMode, DrawOptions drawOptions, PrintQuality printQuality, MediaOptimizationLevel mediaOptimizationLevel )
            : m_labelMode( labelMode ), m_drawOptions( drawOptions ),
            m_printQuality( printQuality ), m_mediaOptimizationLevel( mediaOptimizationLevel )  {}
   };

   static QLightScribe *instance();

   QList< QLightDrive * > getDrives( bool refresh = false );
   QPixmap preview( QLightDrive *drive, const PrintParameters &params, QCDScene *scene, const QSize &size );
   void print( QLightDrive *drive, const PrintParameters &params, QCDScene *scene );

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
   QLightScribe();
   virtual ~QLightScribe();

   struct Task;

   QList< QLightDrive * > m_drives;
   Task                  *m_task;
   bool                   m_aborted;
   QMutex                *m_mutex;
   QWaitCondition        *m_waitQueue;
   QWaitCondition        *m_waitDone;
};

#endif // QLIGHTSCRIBE_H
