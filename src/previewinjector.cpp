#include "previewinjector.h"

#include <QFileDialog>
#include <QLabel>
#include <QSplitter>
#include <QImageReader>

PreviewInjector::PreviewInjector( QFileDialog * dl ) :
    m_label( 0 ),
    m_size( 160, 120 )
{
    if( !dl )
        return;

    QSplitter *sp = qFindChild<QSplitter *>( dl, "splitter" );
    if( !sp )
        return;

    m_label = new QLabel;
    m_label->resize( m_size );
    sp->addWidget( m_label );
    connect( dl, SIGNAL(currentChanged(QString)),SLOT(onSelectionChanged(QString)));
}

PreviewInjector::~PreviewInjector()
{
    delete m_label;
}

void PreviewInjector::onSelectionChanged(QString file)
{
    QImageReader reader( file );

    if( !reader.canRead() ) {
        m_label->setPixmap( QPixmap() );
        return;
    }

    m_label->resize( m_size );
    QSize toSize = m_size;
    QSize sz = reader.size();
    if( !sz.isNull() ) {
        double r1 = m_label->size().width() / double( sz.width() );
        double r2 = m_label->size().height() / double( sz.height() );

        double ratio = std::max( r1, r2 );
        if( ratio < 1.0 ) {
            toSize.setWidth( sz.width() * ratio );
            toSize.setHeight( sz.height() * ratio );
        }
    }

    reader.setScaledSize( toSize );
    QImage img = reader.read();
    if( img.isNull() ) {
        m_label->setPixmap( QPixmap() );
        return;
    }
    m_label->setPixmap( QPixmap::fromImage( img ) );
}

