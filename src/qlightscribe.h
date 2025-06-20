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

#include "qlscribe.h"

#include <QPixmap>
#include <QMap>
#include <QMetaType>

class QCDScene;
class QLightDrive;

struct PrintParameters {
   LabelMode              m_labelMode;
   DrawOptions            m_drawOptions;
   PrintQuality           m_printQuality;
   MediaOptimizationLevel m_mediaOptimizationLevel;
   bool                   m_ejectAfterComplete;

   PrintParameters()
            : m_labelMode( modeFull ), m_drawOptions( drawDefault ),
            m_printQuality( qualityBest ), m_mediaOptimizationLevel( mediaRecognized ), m_ejectAfterComplete( false )  {}
   PrintParameters( LabelMode labelMode,
                    DrawOptions drawOptions,
                    PrintQuality printQuality,
                    MediaOptimizationLevel mediaOptimizationLevel,
                    bool ejectAfterComplete )
            : m_labelMode( labelMode ), m_drawOptions( drawOptions ),
            m_printQuality( printQuality ), m_mediaOptimizationLevel( mediaOptimizationLevel ),
            m_ejectAfterComplete( ejectAfterComplete )
   {}
};

Q_DECLARE_METATYPE( PrintParameters );

typedef QMap<QString,QString> QObject2StringMap;
Q_DECLARE_METATYPE( QObject2StringMap );

class OrgLightscribePrintManagerInterface;

class QLightScribe : public QObject {
   Q_OBJECT
public:
   static QLightScribe *instance();

   QList<QLightDrive *> getDrives();

private:
   QLightScribe();
   virtual ~QLightScribe();

   OrgLightscribePrintManagerInterface *m_managerPrx;
   QList< QLightDrive * >               m_drives;
};

class OrgLightscribeDriveInterface;

class QLightDrive : public QObject {
   Q_OBJECT
public:
   const QString &productName() const { return m_productName; }
   const QString &vendorName() const { return m_vendorName; }
   const QString &displayName() const { return m_displayName; }
   const QString &path() const { return m_path; }

   double innerRadius() const { return m_innerRadius; }
   double outerRadius() const { return m_outerRadius; }

   QPixmap preview( const PrintParameters &params, QCDScene *scene, const QSize &size ) noexcept;
   void print( const PrintParameters &params, QCDScene *scene ) noexcept;

   friend class QLightScribe;

public slots:
   void abort();

signals:
   void prepareProgress( int current, int final );
   void labelProgress( int current, int final );
   void timeEstimate( int time );
   void finished( int status );

private:
   QLightDrive( QObject *parent, const QString &path, const QString &name );

   OrgLightscribeDriveInterface *m_drivePrx;

   QString                       m_productName;
   QString                       m_vendorName;
   QString                       m_displayName;
   QString                       m_path;

   double                        m_innerRadius;
   double                        m_outerRadius;
};

#endif // QLIGHTSCRIBE_H
