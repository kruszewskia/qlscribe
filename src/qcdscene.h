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

#ifndef QCDSCENE_H
#define QCDSCENE_H

#include "qlscribe.h"

#include <QGraphicsScene>

class QXmlStreamWriter;
class QXmlStreamReader;

class QCDScene : public QGraphicsScene {
   Q_OBJECT
public:
   typedef QMap<QString,QString> QString2String;

   QCDScene( QObject * parent = 0 );
   virtual ~QCDScene();

   LabelMode labelMode() const { return m_labelMode; }
   void setLabelMode( LabelMode mode );

   QColor cdColor() const { return m_cdColor; }
   void setCDColor( const QColor &color );

   bool isSaved() const { return m_saved; }
   bool isUnnamed() const { return m_fileName.isEmpty(); }
   bool canUndo() const;
   bool canRedo() const;

   void itemMoved() { m_itemMoved = true; }
   void setChanged( bool undo = true );
   void setName();
   void start( LabelMode mode );
   bool load( const QString &fileName, QString *errMessage = 0 );
   void replace( const QString2String &strings );
   bool save();
   bool saveAs( const QString &fileName );
   void undo() { unredo( true ); }
   void redo() { unredo( false ); }

   void putItemToClipboard( bool move );
   void getItemFromClipboard();
   void addItem( QGraphicsItem *item, bool top );

   void updateTitles() const;
   void redrawViews() const;
   QString name() const { return m_name; }

signals:
   void changed();

protected:
   virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent *contextMenuEvent );
   virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent *mouseEvent );

private slots:
   void onMenuEdit();
   void onMenuCopy();
   void onMenuCut();
   void onMenuToFront();
   void onMenuToBack();
   void onMenuDelete();

private:
   void write( QXmlStreamWriter &writer );
   bool readItem( QXmlStreamReader &reader, bool top );
   void read( QXmlStreamReader &reader );
   void sendItemTo( bool front );
   void pushUndo();
   void unredo( bool undo );

private:
   typedef QList<QString> UndoBuffer;

   QString                    m_name;
   QString                    m_fileName;
   int                        m_index;
   bool                       m_saved;
   bool                       m_itemMoved;
   LabelMode                  m_labelMode;
   QColor                     m_cdColor;
   UndoBuffer                 m_undoBuffer;
   UndoBuffer::const_iterator m_undoPosition;
};

#endif //QCDSCENE_H
