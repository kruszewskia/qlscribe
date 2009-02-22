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

#include "qdialogprint.h"
#include "ui_qdialogprint.h"

#include <QMessageBox>

QDialogPrint::QDialogPrint(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::QDialogPrint)
{
    m_ui->setupUi(this);
}

QDialogPrint::~QDialogPrint()
{
    delete m_ui;
}

void QDialogPrint::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QLightDrive *QDialogPrint::exec( QWidget *parent, PrintParameters &params )
{
   QList<QLightDrive *> drives = QLightScribe::instance()->getDrives();
   if( drives.isEmpty() ) {
      QMessageBox::critical( parent, tr( "Error" ), tr( "Cannot find any lightScribe drive" ) );
      return 0;
   }
   QDialogPrint dialog( parent );
   switch( params.m_labelMode ) {
   case modeFull    : dialog.m_ui->radioModeFull->setChecked( true ); break;
   case modeContent : dialog.m_ui->radioModeContent->setChecked( true ); break;
   case modeTitle   : dialog.m_ui->radioModeTitle->setChecked( true ); break;
   }

   int index = 0;
   foreach( QLightDrive *drv, drives ) {
      dialog.m_ui->comboDrive->insertItem( index++, drv->displayName() );
   }
   if( dialog.QDialog::exec() == Rejected )
      return 0;

   params = PrintParameters(); // reset to default

   if( dialog.m_ui->radioModeFull->isChecked() )
      params.m_labelMode = modeFull;

   if( dialog.m_ui->radioModeTitle->isChecked() )
      params.m_labelMode = modeTitle;

   if( dialog.m_ui->radioModeContent->isChecked() )
      params.m_labelMode = modeContent;

   if( dialog.m_ui->radioQualityNormal->isChecked() )
      params.m_printQuality = qualityNormal;

   if( dialog.m_ui->radioQualityDraft->isChecked() )
      params.m_printQuality = qualityDraft;

   if( dialog.m_ui->radioMediaGeneric->isChecked() )
      params.m_mediaOptimizationLevel = mediaGeneric;

   return drives.at( dialog.m_ui->comboDrive->currentIndex() );
}
