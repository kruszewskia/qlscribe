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

#include "qdialogroundtext.h"
#include "ui_qdialogroundtext.h"
#include "qgraphicsroundtextitem.h"
#include "qcdscene.h"

#include <QFontDialog>
#include <QColorDialog>
#include <QPalette>

QDialogRoundText::QDialogRoundText(QWidget *parent) :
    QItemDialog( parent ),
    m_ui( new Ui::QDialogRoundText ),
    m_item( 0 )
{
   m_ui->setupUi(this);
   connect( m_ui->btnFont,  SIGNAL(clicked()), this, SLOT(onFont()) );
   connect( m_ui->btnColor, SIGNAL(clicked()), this, SLOT(onColor()) );
}

QDialogRoundText::~QDialogRoundText()
{
    delete m_ui;
}

void QDialogRoundText::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool QDialogRoundText::exec( QGraphicsItem *graphicsItem )
{
   QGraphicsRoundTextItem *item = dynamic_cast< QGraphicsRoundTextItem * >( graphicsItem );
   if( !item )
      return false;

   QCDScene scene;
   m_ui->cdView->setScene( &scene );
   m_item = new QGraphicsRoundTextItem;

   m_item->setPos( item->pos() );
   m_item->setText( item->text() );
   m_item->setFont( item->font() );
   m_item->setBrush( item->brush() );
   m_item->setRadius( item->radius() );
   m_item->setAngle( item->angle() );
   m_item->setAlignment( item->alignment() );
   m_item->setOutside( item->outside() );

   scene.addItem( m_item );

   m_ui->spinX->setValue( m_item->pos().x() );
   m_ui->spinY->setValue( m_item->pos().y() );
   m_ui->spinRadi->setValue( m_item->radius() );
   m_ui->spinAngle->setValue( m_item->angle() );

   int index = 0;
   if( m_item->alignment() == Qt::AlignRight ) index = 2;
   else if( m_item->alignment() == Qt::AlignCenter ) index = 1;

   m_ui->comboAlignment->setCurrentIndex( index );
   m_ui->comboLocation->setCurrentIndex( m_item->outside() );
   m_ui->textEdit->setText( m_item->text() );

   fontChanged();
   colorChanged();

   connect( m_ui->textEdit,       SIGNAL(textEdited(const QString&)),
            this, SLOT(textChanged()) );

   connect( m_ui->spinX,          SIGNAL(valueChanged(double)),
            this, SLOT(posChanged()) );

   connect( m_ui->spinY,          SIGNAL(valueChanged(double)),
            this, SLOT(posChanged()) );

   connect( m_ui->spinRadi,       SIGNAL(valueChanged(double)),
            this, SLOT(radiChanged()) );

   connect( m_ui->spinAngle,      SIGNAL(valueChanged(double)),
            this, SLOT(angleChanged()) );

   connect( m_ui->comboAlignment, SIGNAL(currentIndexChanged(int)),
            this, SLOT(alignChanged()) );

   connect( m_ui->comboLocation,  SIGNAL(currentIndexChanged(int)),
            this, SLOT(locChanged()) );

   if( QDialog::exec() == Rejected ) return false;

   item->setPos( m_item->pos() );
   item->setText( m_item->text() );
   item->setFont( m_item->font() );
   item->setBrush( m_item->brush() );
   item->setRadius( m_item->radius() );
   item->setAngle( m_item->angle() );
   item->setAlignment( m_item->alignment() );
   item->setOutside( m_item->outside() );

   return true;
}

void QDialogRoundText::onFont()
{
   bool ok = true;
   QFont font = QFontDialog::getFont( &ok, m_item->font(), this );
   if( ok ) {
      m_item->setFont( font );
      fontChanged();
   }
}

void QDialogRoundText::onColor()
{
   QColor color = QColorDialog::getColor( m_item->brush().color(), this );
   if( color.isValid() ){
      m_item->setBrush( color );
      colorChanged();
   }
}

void QDialogRoundText::fontChanged()
{
   m_ui->fontName->setCurrentFont( m_item->font() );
}

void QDialogRoundText::colorChanged()
{
   QPixmap pixmap( m_ui->colorWidget->size() );
   pixmap.fill( m_item->brush().color() );
   m_ui->colorWidget->setPixmap( pixmap );
}


void QDialogRoundText::textChanged()
{
   m_item->setText( m_ui->textEdit->text() );
}

void QDialogRoundText::posChanged()
{
   m_item->setPos( m_ui->spinX->value(), m_ui->spinY->value() );
}

void QDialogRoundText::radiChanged()
{
   m_item->setRadius( m_ui->spinRadi->value() );
}

void QDialogRoundText::angleChanged()
{
   m_item->setAngle( m_ui->spinAngle->value() );
}

void QDialogRoundText::alignChanged()
{
   Qt::Alignment align[] = { Qt::AlignLeft, Qt::AlignCenter, Qt::AlignRight };
   m_item->setAlignment( align[ m_ui->comboAlignment->currentIndex() ] );
}

void QDialogRoundText::locChanged()
{
   m_item->setOutside( m_ui->comboLocation->currentIndex() );
}
