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

#include "qshapefactory.h"

#include <QGraphicsItem>
#include <QMessageBox>
#include <QXmlStreamWriter>

QShapeController::~QShapeController()
{
}

bool QShapeController::edit( QGraphicsItem *item, QWidget *parent ) const
{
   QItemDialog *dialog = createDialog( parent );
   if( !dialog )
      return false;

   bool rez = false;
   try {
      rez = dialog->exec( item ) == QDialog::Accepted;
   }
   catch(...) {
      delete dialog;
      throw;
   }
   delete dialog;
   return rez;
}

void QShapeController::write( QXmlStreamWriter &writer, const QGraphicsItem *item ) const
{
   writer.writeStartElement( "item" );
   writer.writeAttribute( QXmlStreamAttribute( "type", name() ) );
   writeData( writer, item );
   writer.writeEndElement();
}

QGraphicsItem *QShapeController::read( QXmlStreamReader &reader ) const
{
   QGraphicsItem *item = create();
   try {
      while( true ) {
         QXmlStreamReader::TokenType ttype = reader.readNext();

         if( reader.atEnd() )
            throw QString( "QShapeController: unexpected end of document" );

         if( ttype == QXmlStreamReader::EndElement )
            break;;

         if( ttype == QXmlStreamReader::StartElement ) {
            QString elementName = reader.name().toString();
            QXmlStreamAttributes attributes = reader.attributes();
            QString data = reader.readElementText();

            readData( elementName, attributes, data, item );
         }
      }
   }
   catch(...) {
      delete item;
      throw;
   }
   return item;
}

QShapeFactory &QShapeFactory::QShapeFactory::instance()
{
   static QShapeFactory inst;
   return inst;
}

QShapeFactory::QShapeFactory()
{
}

QShapeFactory::~QShapeFactory()
{
   for( iterator i = begin(); i != end(); ++i )
      delete i->second;
}


QString QShapeFactory::name( int type ) const
{
   QShapeController *ctrl = getController( type );
   return ctrl ? ctrl->name() : QString();
}

QGraphicsItem *QShapeFactory::create( int type ) const
{
   QShapeController *ctrl = getController( type );
   return ctrl ? ctrl->create() : 0;
}

bool QShapeFactory::edit( QGraphicsItem *item, QWidget *parent ) const
{
   QShapeController *ctrl = getController( item->type() );
   if( ctrl )
      return ctrl->edit( item, parent );

   return false;
}

QShapeController *QShapeFactory::getController( int type ) const
{
   Controllers::const_iterator f = m_controllers.find( type );
   if( f == m_controllers.end() ) {
      QMessageBox::warning( 0, "Warning", QObject::tr( "Cannot find controller for type %n", 0, type ) );
      return 0;
   }
   return f->second;
}

QShapeFactory::iterator QShapeFactory::find( const QString &name ) const
{
   Name2Type::const_iterator f = m_name2type.find( name );
   return f == m_name2type.end() ? end() : find( f->second );
}

void QShapeFactory::registerController( int type, QShapeController *ctrl )
{
   m_controllers.insert( std::make_pair( type, ctrl ) );
   m_name2type.insert( std::make_pair( ctrl->name(), type ) );
}
