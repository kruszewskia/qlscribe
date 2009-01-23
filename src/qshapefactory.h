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

#ifndef QSHAPEFACTORY_H
#define QSHAPEFACTORY_H

#include <map>

#include <QString>
#include <QDialog>

class QGraphicsItem;
class QXmlStreamAttributes;

class QItemDialog : public QDialog {
public:
   QItemDialog( QWidget *parent ) : QDialog( parent ) {}

   virtual bool exec( QGraphicsItem *item ) = 0;
};

class QXmlStreamWriter;
class QXmlStreamReader;

class QShapeController {
public:
   virtual ~QShapeController();

   virtual QString name() const = 0;
   virtual QString menuName() const = 0;
   virtual QGraphicsItem *create() const = 0;
   bool edit( QGraphicsItem *item, QWidget *parent ) const;
   void write( QXmlStreamWriter &writer, const QGraphicsItem *item ) const;
   QGraphicsItem *read( QXmlStreamReader &reader ) const;

protected:
   virtual QItemDialog *createDialog( QWidget *parent ) const = 0;
   virtual void writeData( QXmlStreamWriter &writer, const QGraphicsItem *item ) const = 0;
   virtual void readData( const QString &element,
                          const QXmlStreamAttributes &attrs,
                          const QString &data,
                          QGraphicsItem *item ) const = 0;
};

class QShapeFactory {
public:
   typedef std::map< int, QShapeController * > Controllers;
   typedef Controllers::const_iterator iterator;
   static QShapeFactory &instance();

   void registerController( int type, QShapeController *ctrl );

   QString name( int type ) const;
   QGraphicsItem *create( int type ) const;
   bool edit( QGraphicsItem *item, QWidget *parent ) const;

   iterator find( const QString &name ) const;
   iterator find( int type ) const { return m_controllers.find( type ); }
   iterator begin() const { return m_controllers.begin(); }
   iterator end() const { return m_controllers.end(); }

private:
    QShapeFactory();
    ~QShapeFactory();

   QShapeController *getController( int type ) const;

    Controllers m_controllers;

    typedef std::map< QString, int >  Name2Type;
    Name2Type   m_name2type;
};

template <class T>
struct RegisterController {
   RegisterController() { QShapeFactory::instance().registerController( T::Type, new T ); }
};

#endif // QSHAPEFACTORY_H
