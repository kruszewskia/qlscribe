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

#include "qcdview.h"
#include "qcdscene.h"
#include "mainwindow.h"

#include <QCloseEvent>
#include <QMessageBox>

QCDView::QCDView( QWidget *parent )
   : QGraphicsView( parent ),
   m_mask( new QPixmap ),
   m_labelMode( -1 )
{
   setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   centerOn( 0, 0 );
}

QCDView::~QCDView()
{
}

QSize QCDView::sizeHint () const
{
   return QSize( 600, 600 );
}

void QCDView::setScene( QCDScene *scene )
{
   QGraphicsView::setScene( scene );
}

QCDScene *QCDView::scene() const
{
   return static_cast< QCDScene * >( QGraphicsView::scene() );
}

void QCDView::closeEvent( QCloseEvent *event )
{
   QCDScene *cdscene = scene();
   if( cdscene && !cdscene->isSaved() ) {
      QMessageBox::StandardButton button =
            QMessageBox::question( this,
                                   tr( "Confitmation" ),
                                   cdscene->name() + tr( " is changed, do you want to save it?" ),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );

      if( button == QMessageBox::Cancel ) {
         event->ignore();
         return;
      }
      if( button == QMessageBox::Yes ) {
         QWidget *parent = parentWidget();
         for( ; parent->parentWidget(); parent = parent->parentWidget() );
         if( !static_cast< MainWindow * >( parent )->saveScene( cdscene ) ) {
            event->ignore();
            return;
         }
      }

   }
   event->accept();
}

inline 
void drawCircle( QPainter *painter, double radi )
{
   painter->drawEllipse( QRectF( -radi, -radi, radi * 2, radi * 2 ) );
}

void QCDView::drawCD( QPainter *painter, const QRectF & rect, bool alpha )
{
   painter->setBrush( alpha ? Qt::lightGray : Qt::gray );
   painter->drawRect( rect );

   const double fullInner = 24.7, fullOuter = 60.0;

   QCDScene *cdscene = scene();
   if( cdscene ) {
      double inner = 0, outer = 0;
      switch( cdscene->labelMode() ) {
      case modeFull :     inner = 24.7; outer = 60.0; break;
      case modeTitle :    inner = 32.0; outer = 37.4; break;
      case modeContent :  inner = 25.8; outer = 37.4; break;
      }
      if( alpha ) {
         painter->setBrush( Qt::black );
         drawCircle( painter, outer );

         painter->setBrush( Qt::lightGray );
         drawCircle( painter, inner );
      } else {
         QColor darker( cdscene->cdColor().darker( 150 ) );
         if( outer < fullOuter ) {
            painter->setBrush( darker );
            drawCircle( painter, fullOuter );
         }

         painter->setBrush( cdscene->cdColor() );
         drawCircle( painter, outer );

         if( inner > fullInner ) {
            painter->setBrush( darker );
            drawCircle( painter, inner );
         }

         painter->setBrush( Qt::darkGray );
         drawCircle( painter, fullInner );
      }
   } else {
      painter->setBrush( alpha ? Qt::black : Qt::white );
      drawCircle( painter, fullOuter );

      painter->setBrush( alpha ? Qt::lightGray : Qt::darkGray );
      drawCircle( painter, fullInner );
   }

   painter->setBrush( alpha ? Qt::white : Qt::gray );
   drawCircle( painter, 10.7 );
}

void QCDView::drawForeground ( QPainter * painter, const QRectF & rect )
{
   if( !painter ) return;
   QCDScene *cdscene = scene();
   if( m_mask->size() != frameSize() || ( cdscene && m_labelMode != cdscene->labelMode() ) ) {
      if( cdscene )
         m_labelMode = cdscene->labelMode();

      *m_mask = QPixmap( frameSize() );

      {
         QPainter paint( m_mask );
         paint.setWorldTransform( painter->transform() );
         drawCD( &paint, rect, false );
      }

      {
         QPixmap alpha( frameSize() );
         QPainter paint( &alpha );
         paint.setWorldTransform( painter->transform() );
         drawCD( &paint, rect, true );
         m_mask->setAlphaChannel( alpha );
      }

   }
   painter->resetTransform();
   painter->drawPixmap( 0, 0, *m_mask );
}

void QCDView::drawBackground ( QPainter * painter, const QRectF & rect )
{
   if( painter )
      drawCD( painter, rect, false );
}

void QCDView::resizeEvent( QResizeEvent * event )
{
   QGraphicsScene *scn = scene();
   if( scn )
      fitInView( scn->sceneRect(), Qt::KeepAspectRatio );
}
