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
#include "qlightpixmapitem.h"
#include "qcdscene.h"
#include "previewinjector.h"

#include <QImageReader>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

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
   QLightPixmapItem *item = dynamic_cast< QLightPixmapItem * >( graphicsItem );
   if( !item )
      return false;

   QCDScene scene;
   m_ui->cdView->setScene( &scene );
   m_item = new QLightPixmapItem;

   m_item->setPos( item->pos() );
   m_item->setOffset( item->offset() );
   m_item->setPixmap( item->pixmap() );
   m_item->imageName( item->imageName() );
   m_item->setTransform( item->transform() );

   scene.addItem( m_item, true );

   m_ui->spinX->setValue( m_item->pos().x() );
   m_ui->spinY->setValue( m_item->pos().y() );
   m_ui->lineEdit->setText( m_item->imageName() );
   if( !m_item->pixmap().isNull() ) {
      QSize size = m_item->pixmap().size();
      QPointF scale = m_item->transform().map( QPointF( size.width(), size.height() ) );
      m_ui->spinWidth->setValue( scale.x() );
      m_ui->spinHeight->setValue( scale.y() );
      m_ui->lockedRatio->setChecked( scale.x() == scale.y() ? Qt::Checked : Qt::Unchecked );
      m_ui->buttonBox->setStandardButtons( m_ui->buttonBox->standardButtons() | QDialogButtonBox::Ok );
   }

   connect( m_ui->spinX,          SIGNAL(valueChanged(double)),
            this, SLOT(posChanged()) );

   connect( m_ui->spinY,          SIGNAL(valueChanged(double)),
            this, SLOT(posChanged()) );

   connect( m_ui->spinWidth,      SIGNAL(valueChanged(double)),
            this, SLOT(sizeChanged()) );

   connect( m_ui->spinHeight,     SIGNAL(valueChanged(double)),
            this, SLOT(sizeChanged()) );

   connect( m_ui->btnSizeReset,   SIGNAL(clicked()),
            this, SLOT(onResetSize()) );

   connect( m_ui->lockedRatio,    SIGNAL(stateChanged(int)),
            this, SLOT(sizeChanged()) );

   if( QDialog::exec() == Rejected ) return false;

   item->setPos( m_item->pos() );
   item->setOffset( m_item->offset() );
   item->setPixmap( m_item->pixmap() );
   item->imageName( m_item->imageName() );
   item->setTransform( m_item->transform() );

   return true;
}

void QDialogPixmap::onLoadImage()
{
   QString filter( tr("Images ( ") );
   QList<QByteArray> list = QImageReader::supportedImageFormats();
   foreach( QByteArray arr, list ) {
       filter += QString( "*." ) + arr.data() + " ";
   }
   filter += tr( ");;All Files (*.*)" );

   QFileDialog fd( this, "Load image:", QString(), filter );
   //fd.setOption( QFileDialog::DontUseNativeDialog ); // for testing

   bool injectPreview = QSettings().value( cfgInjectPreview, true ).toBool();
   PreviewInjector pj( injectPreview ? &fd : 0 );
   fd.setFileMode( QFileDialog::ExistingFile );

   if( fd.exec() != QDialog::Accepted )
       return;

   QString fileName = fd.selectedFiles().value(0);
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
   m_item->imageName( fileName );
   m_item->setOffset( -QPointF( pixmap.size().width(), pixmap.size().height()  ) / 2.0 );

   m_ui->lineEdit->setText( fileName );
   m_ui->buttonBox->setStandardButtons( m_ui->buttonBox->standardButtons() | QDialogButtonBox::Ok );
   onResetSize();
}

void QDialogPixmap::posChanged()
{
   m_item->setPos( m_ui->spinX->value(), m_ui->spinY->value() );
}

void QDialogPixmap::onResetSize()
{
   m_ui->spinWidth->setValue( m_item->pixmap().size().width() / 10.0 );
   m_ui->spinHeight->setValue( m_item->pixmap().size().height() / 10.0 );
}

void QDialogPixmap::sizeChanged()
{
   if( m_ui->lockedRatio->checkState() == Qt::Checked ) {
      QObject *senderObj = sender();
      QSize size = m_item->pixmap().size();
      if( senderObj == m_ui->spinWidth || senderObj == m_ui->lockedRatio ) {
         m_ui->spinHeight->setValue( m_ui->spinWidth->value() / size.width() * size.height() );
      } else {
         m_ui->spinWidth->setValue( m_ui->spinHeight->value() / size.height() * size.width() );
      }

   }
   m_item->setTransform( QTransform().scale( m_ui->spinWidth->value() / m_item->pixmap().size().width(),
                                             m_ui->spinHeight->value() / m_item->pixmap().size().height() ) );
}
