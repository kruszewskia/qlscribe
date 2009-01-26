#include "qdialogprint.h"
#include "ui_qdialogprint.h"

#include <QMessageBox>

QDialogPrint::QDialogPrint(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::QDialogPrint)
{
    m_ui->setupUi(this);
}

QDialogPrint::~QDialogPrint()
{
    delete m_ui;
}

void QDialogPrint::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QLightDrive *QDialogPrint::exec( QWidget *parent, QLightScribe::PrintParameters &params )
{
   QList<QLightDrive *> drives = QLightScribe::instance()->getDrives();
   if( drives.isEmpty() ) {
      QMessageBox::critical( parent, tr( "Error" ), tr( "Cannot find any lightScribe drive" ) );
      return 0;
   }
   QDialogPrint dialog( parent );
   int index = 0;
   foreach( QLightDrive *drv, drives ) {
      dialog.m_ui->comboDrive->insertItem( index++, drv->displayName() );
   }
   if( dialog.QDialog::exec() == Rejected )
      return 0;

   params = QLightScribe::PrintParameters(); // reset to default

   if( dialog.m_ui->radioModeTitle->isChecked() )
      params.m_labelMode = QLightScribe::modeTitle;

   if( dialog.m_ui->radioModeContent->isChecked() )
      params.m_labelMode = QLightScribe::modeContent;

   if( dialog.m_ui->radioQualityNormal->isChecked() )
      params.m_printQuality = QLightScribe::qualityNormal;

   if( dialog.m_ui->radioQualityDraft->isChecked() )
      params.m_printQuality = QLightScribe::qualityDraft;

   if( dialog.m_ui->radioMediaGeneric->isChecked() )
      params.m_mediaOptimizationLevel = QLightScribe::mediaGeneric;

   return drives.at( dialog.m_ui->comboDrive->currentIndex() );
}
