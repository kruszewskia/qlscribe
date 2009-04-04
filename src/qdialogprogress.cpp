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

#include "qdialogprogress.h"
#include "ui_qdialogprogress.h"
#include "qlightscribe.h"
#include "qdialogprint.h"
#include "qcdscene.h"

#include <QMessageBox>
#include <QAbstractButton>
#include <QTime>
#include <QTimer>
#include <QSettings>

QDialogProgress::QDialogProgress( QWidget *parent, QLightDrive *drive )
   : QDialog(parent),
    m_ui(new Ui::QDialogProgress),
    m_drive( drive ),
    m_start( new QTime ),
    m_timer( new QTimer( this ) )
{
    m_ui->setupUi(this);
    m_start->start();
    connect( m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()) );
    m_timer->start( 500 );
}

QDialogProgress::~QDialogProgress()
{
   delete m_start;
   delete m_timer;
   delete m_ui;
}

void QDialogProgress::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool QDialogProgress::exec( QWidget *parent, QCDScene *scene )
{
   PrintParameters params;
   params.m_labelMode = scene->labelMode();
   QLightDrive *drive = QDialogPrint::exec( parent, params );
   if( !drive )
      return false;

   params.m_ejectAfterComplete = QSettings().value( cfgEjectAfterFinish, false ).toBool();

   QDialogProgress dialog( parent, drive );
   dialog.setWindowTitle( tr( "Printing: " ) + scene->name() );

   connect( dialog.m_ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &dialog, SLOT(onButtonClicked(QAbstractButton*)) );
   connect( drive, SIGNAL(prepareProgress(int,int)), &dialog, SLOT(onPrepareProgress(int,int)) );
   connect( drive, SIGNAL(labelProgress(int,int)), &dialog, SLOT(onLabelProgress(int,int)) );
   connect( drive, SIGNAL(timeEstimate(int)), &dialog, SLOT(onTimeEstimate(int)) );
   connect( drive, SIGNAL(finished(int)), &dialog, SLOT(onFinished(int)) );

   drive->print( params, scene );
   dialog.QDialog::exec();
   return true;
}

void QDialogProgress::onButtonClicked( QAbstractButton *button )
{
   if( ( QAbstractButton * ) m_ui->buttonBox->button( QDialogButtonBox::Abort ) == button ) {
      if( QMessageBox::question( this,
                                 tr( "Confirmation" ),
                                 tr( "Are you sure you want to interrupt?" ),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No ) == QMessageBox::Yes ) {
         m_drive->abort();
         button->setDisabled( true );
      }
   } else
      done( 0 );
}

void QDialogProgress::onPrepareProgress( int current, int final )
{
   m_ui->progressPreparation->setMaximum( final );
   m_ui->progressPreparation->setValue( current );
}

void QDialogProgress::onLabelProgress( int current, int final )
{
   m_ui->progressPrinting->setMaximum( final );
   m_ui->progressPrinting->setValue( current );

   if( current && final / double( current ) < 10.0 ) {
      int elapsed = m_start->elapsed();
      int estimate = int( double( elapsed ) / current * final );
      m_ui->timeEstimated->setTime( QTime().addMSecs( estimate ) );
   }
}

void QDialogProgress::onTimeEstimate( int time )
{
   m_ui->timeEstimated->setTime( QTime().addSecs( time ) );
}

void QDialogProgress::onFinished(int status )
{
   m_timer->stop();
   m_ui->progressPrinting->setValue( m_ui->progressPrinting->maximum() );
   m_ui->lineResult->setText( status ? tr( "Failed - error code: 0x" ) + QString::number( status, 16 ) : tr( "Success" ) );
   m_ui->buttonBox->setStandardButtons( QDialogButtonBox::Close );
}

void QDialogProgress::onTimeout()
{
   m_ui->timeElapsed->setTime( QTime().addMSecs( m_start->elapsed() ) );
}
