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

#include "qdialogsettings.h"
#include "ui_qdialogsettings.h"
#include "qlscribe.h"

#include <QSettings>
#include <QFontDialog>

QFont *QDialogSettings::m_lastUsedFont = 0;

QDialogSettings::QDialogSettings(QWidget *parent) :
    QDialog( parent ),
    m_ui( new Ui::QDialogSettings )
{
   m_ui->setupUi( this );
   connect( m_ui->btnFont,          SIGNAL(clicked()), this, SLOT(onFont()) );
   connect( m_ui->btnLastUsedFont,  SIGNAL(clicked()), this, SLOT(onLastUsedFont()) );
   m_ui->btnLastUsedFont->setEnabled( m_lastUsedFont );
}

QDialogSettings::~QDialogSettings()
{
    delete m_ui;
}

void QDialogSettings::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool QDialogSettings::exec( QWidget *parent )
{
   QDialogSettings dialog( parent );
   QSettings sts;
   dialog.m_font = sts.value( cfgDefaultFont ).value<QFont>();
   dialog.m_ui->fontName->setCurrentFont( dialog.m_font );
   dialog.m_ui->chkEject->setChecked( sts.value( cfgEjectAfterFinish, false ).toBool() );

   if( dialog.QDialog::exec() == Rejected ) return false;

   sts.setValue( cfgDefaultFont, dialog.m_font );
   sts.setValue( cfgEjectAfterFinish, dialog.m_ui->chkEject->isChecked() );

   return true;
}

void QDialogSettings::onFont()
{
   bool ok = true;
   QFont font = QFontDialog::getFont( &ok, m_font, this );
   if( ok ) {
      m_font = font;
      m_ui->fontName->setCurrentFont( font );
   }
}

void QDialogSettings::onLastUsedFont()
{
   if( m_lastUsedFont ) {
      m_font = *m_lastUsedFont;
      m_ui->fontName->setCurrentFont( *m_lastUsedFont );
   }
}

void QDialogSettings::setLastUsedFont( const QFont &font )
{
   if( !m_lastUsedFont ) m_lastUsedFont = new QFont( font );
   else *m_lastUsedFont = font;
}

QFont QDialogSettings::defaultFont()
{
   return QSettings().value( cfgDefaultFont ).value<QFont>();
}
