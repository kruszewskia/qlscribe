#include "qdialogprogress.h"
#include "ui_qdialogprogress.h"
#include "qlightscribe.h"
#include "qdialogprint.h"

#include <QMessageBox>
#include <QAbstractButton>

QDialogProgress::QDialogProgress(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::QDialogProgress)
{
    m_ui->setupUi(this);
}

QDialogProgress::~QDialogProgress()
{
    delete m_ui;
}

void QDialogProgress::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool QDialogProgress::exec( QWidget *parent, QCDScene *scene )
{
   QLightScribe::PrintParameters params;
   QLightDrive *drive = QDialogPrint::exec( parent, params );
   if( !drive )
      return false;

   QLightScribe *scribe = QLightScribe::instance();
   QDialogProgress dialog( parent );

   connect( dialog.m_ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), &dialog, SLOT(onButtonClicked(QAbstractButton*)) );
   connect( scribe, SIGNAL(prepareProgress(long,long)), &dialog, SLOT(onPrepareProgress(long,long)) );
   connect( scribe, SIGNAL(labelProgress(long,long)), &dialog, SLOT(onLabelProgress(long,long)) );
   connect( scribe, SIGNAL(timeEstimate(long)), &dialog, SLOT(onTimeEstimate(long)) );
   connect( scribe, SIGNAL(finished(int)), &dialog, SLOT(onFinished(int)) );

   scribe->print( drive, params, scene );
   dialog.QDialog::exec();
   return true;
}

void QDialogProgress::onButtonClicked( QAbstractButton *button )
{
   if( ( QAbstractButton * ) m_ui->buttonBox->button( QDialogButtonBox::Abort ) == button ) {
      if( QMessageBox::question( this,
                                 tr( "Confirmation" ),
                                 tr( "Are you sure you want to interrupt?" ),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No ) == QMessageBox::Yes ) {
         QLightScribe::instance()->abort();
         button->setDisabled( true );
      }
   } else
      done( 0 );
}

void QDialogProgress::onPrepareProgress( long current, long final )
{
   m_ui->progressPreparation->setMaximum( final );
   m_ui->progressPreparation->setValue( current );
}

void QDialogProgress::onLabelProgress( long current, long final )
{
   m_ui->progressPrinting->setMaximum( final );
   m_ui->progressPrinting->setValue( current );
}

void QDialogProgress::onTimeEstimate( long time )
{
}

void QDialogProgress::onFinished(int status )
{
   m_ui->lineResult->setText( status ? tr( "Failed - error code: 0x" ) + QString::number( status, 16 ) : tr( "Success" ) );
   m_ui->buttonBox->setStandardButtons( QDialogButtonBox::Close );
}
