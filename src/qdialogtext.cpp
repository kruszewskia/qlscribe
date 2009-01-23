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

#include "qdialogtext.h"
#include "ui_qdialogtext.h"
#include "qcdscene.h"

#include <QGraphicsSimpleTextItem>
#include <QFontDialog>
#include <QColorDialog>

QDialogText::QDialogText(QWidget *parent) :
    QItemDialog(parent),
    m_ui(new Ui::QDialogText),
    m_item( 0 )
{
   m_ui->setupUi(this);
   connect( m_ui->btnFont,  SIGNAL(clicked()),            this, SLOT(onFont()) );
   connect( m_ui->btnColor, SIGNAL(clicked()),            this, SLOT(onColor()) );
}

QDialogText::~QDialogText()
{
   delete m_ui;
}

void QDialogText::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool QDialogText::exec( QGraphicsItem *graphicsItem )
{
   QGraphicsSimpleTextItem *item = dynamic_cast< QGraphicsSimpleTextItem * >( graphicsItem );
   if( !item )
      return false;

   QCDScene scene;
   m_ui->cdView->setScene( &scene );
   m_item = new QGraphicsSimpleTextItem;

   m_item->setPos( item->pos() );
   m_item->setText( item->text() );
   m_item->setFont( item->font() );
   m_item->setBrush( item->brush() );

   scene.addItem( m_item );

   m_ui->spinX->setValue( m_item->pos().x() );
   m_ui->spinY->setValue( m_item->pos().y() );
   m_ui->textEdit->setPlainText( m_item->text() );

   colorChanged();
   fontChanged();

   connect( m_ui->textEdit, SIGNAL(textChanged()),        this, SLOT(textChanged()) );
   connect( m_ui->spinX,    SIGNAL(valueChanged(double)), this, SLOT(posChanged()) );
   connect( m_ui->spinY,    SIGNAL(valueChanged(double)), this, SLOT(posChanged()) );

   if( QDialog::exec() == Rejected ) return false;

   item->setPos( m_item->pos() );
   item->setText( m_item->text() );
   item->setFont( m_item->font() );
   item->setBrush( m_item->brush() );

   return true;
}

void QDialogText::onFont()
{
   bool ok = true;
   QFont font = QFontDialog::getFont( &ok, m_item->font(), this );
   if( ok ) {
      m_item->setFont( font );
      fontChanged();
   }
}

void QDialogText::onColor()
{
   QColor color = QColorDialog::getColor( m_item->brush().color(), this );
   if( color.isValid() ){
      m_item->setBrush( color );
      colorChanged();
   }
}

void QDialogText::fontChanged()
{
   m_ui->fontName->setCurrentFont( m_item->font() );
}

void QDialogText::colorChanged()
{
   QPixmap pixmap( m_ui->colorWidget->size() );
   pixmap.fill( m_item->brush().color() );
   m_ui->colorWidget->setPixmap( pixmap );
}

void QDialogText::textChanged()
{
   m_item->setText( m_ui->textEdit->toPlainText() );
}

void QDialogText::posChanged()
{
   m_item->setPos( m_ui->spinX->value(), m_ui->spinY->value() );
}

