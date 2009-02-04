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

#include "qdialogcdproperties.h"
#include "ui_qdialogcdproperties.h"

#include "qcdscene.h"

#include <QColorDialog>

QDialogCDProperties::QDialogCDProperties(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::QDialogCDProperties)
{
    m_ui->setupUi(this);
    connect( m_ui->btnColor, SIGNAL(clicked()), this, SLOT(onColor()) );
    connect( m_ui->radioModeFull, SIGNAL(toggled(bool)), this, SLOT(onChangeMode()) );
    connect( m_ui->radioModeContent, SIGNAL(toggled(bool)), this, SLOT(onChangeMode()) );
    connect( m_ui->radioModeTitle, SIGNAL(toggled(bool)), this, SLOT(onChangeMode()) );
}

bool QDialogCDProperties::exec( QWidget *parent, QCDScene *scene )
{
   QCDScene lscene;
   lscene.setLabelMode( scene->labelMode() );
   lscene.setCDColor( scene->cdColor() );

   QDialogCDProperties dialog( parent );
   dialog.setWindowTitle( tr( "Label properties: ", "Dialog title \"Label properties: \"" )
                          + scene->name() );
   dialog.m_ui->cdView->setScene( &lscene );

   if( dialog.QDialog::exec() == Rejected ) return false;

   scene->setLabelMode( lscene.labelMode() );
   scene->setCDColor( lscene.cdColor() );

   return true;
}


QDialogCDProperties::~QDialogCDProperties()
{
    delete m_ui;
}

void QDialogCDProperties::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void QDialogCDProperties::onChangeMode()
{
   QCDScene *scene = m_ui->cdView->scene();
   if( m_ui->radioModeFull->isChecked() )
      scene->setLabelMode( modeFull );

   if( m_ui->radioModeTitle->isChecked() )
      scene->setLabelMode( modeTitle );

   if( m_ui->radioModeContent->isChecked() )
      scene->setLabelMode( modeContent );
}

void QDialogCDProperties::onColor()
{
   QCDScene *scene = m_ui->cdView->scene();
   QColor color = QColorDialog::getColor( scene->cdColor(), this );
   if( color.isValid() ){
      scene->setCDColor( color );
   }
}
