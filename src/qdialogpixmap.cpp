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

#include "qdialogpixmap.h"
#include "ui_qdialogpixmap.h"
#include "qcdscene.h"

#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QFileDialog>
#include <QMessageBox>

QDialogPixmap::QDialogPixmap(QWidget *parent) :
    QItemDialog( parent ),
    m_ui( new Ui::QDialogPixmap ),
    m_item( 0 )
{
   m_ui->setupUi(this);
   connect( m_ui->btnImage,  SIGNAL(clicked()), this, SLOT(onLoadImage()) );
}

QDialogPixmap::~QDialogPixmap()
{
    delete m_ui;
}

void QDialogPixmap::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool QDialogPixmap::exec( QGraphicsItem *graphicsItem )
{
   QGraphicsPixmapItem *item = dynamic_cast< QGraphicsPixmapItem * >( graphicsItem );
   if( !item )
      return false;

   QCDScene scene;
   m_ui->cdView->setScene( &scene );
   m_item = new QGraphicsPixmapItem;

   m_item->setPos( item->pos() );
   m_item->setOffset( item->offset() );
   m_item->setPixmap( item->pixmap() );
   m_item->setData( 0, item->data( 0 ) );

   scene.addItem( m_item );

   m_ui->spinX->setValue( m_item->pos().x() );
   m_ui->spinY->setValue( m_item->pos().y() );

   connect( m_ui->spinX,          SIGNAL(valueChanged(double)),
            this, SLOT(posChanged()) );

   connect( m_ui->spinY,          SIGNAL(valueChanged(double)),
            this, SLOT(posChanged()) );

   if( QDialog::exec() == Rejected ) return false;

   item->setPos( m_item->pos() );
   item->setOffset( m_item->offset() );
   item->setPixmap( m_item->pixmap() );
   item->setData( 0, m_item->data( 0 ) );

   return true;
}

void QDialogPixmap::onLoadImage()
{
   QString filter( tr("Images ( ") );
   QList<QByteArray> list = QImageReader::supportedImageFormats();
   foreach( QByteArray arr, list ) {
       filter += QString( "*." ) + arr.data() + " ";
   }
   filter += tr( ")\nAll Files (*)" );
   QString fileName = QFileDialog::getOpenFileName( this, "Load image:", QString(), filter );
   if( fileName.isNull() )
      return;

   QPixmap pixmap( fileName );
   if( pixmap.isNull() ) {
      QMessageBox::warning( this,
                            "Warning",
                            tr( "Cannot create image from file: " ) + fileName );
      return;
   }
   m_item->setPixmap( pixmap );
   m_item->setData( 0, fileName );
   m_item->setOffset( -QPointF( pixmap.size().height(), pixmap.size().width()  ) / 2.0 );

   m_ui->lineEdit->setText( fileName );
   m_ui->buttonBox->setStandardButtons( m_ui->buttonBox->standardButtons() | QDialogButtonBox::Ok );
}

void QDialogPixmap::posChanged()
{
   m_item->setPos( m_ui->spinX->value(), m_ui->spinY->value() );
}
