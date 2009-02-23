#include "qconsoleprintprogress.h"
#include "qlightscribe.h"

#include <QApplication>

#include <iostream>

std::ostream &operator<<( std::ostream &os, const QString &str );

QConsolePrintProgress::QConsolePrintProgress( QLightDrive *drive )
      : m_drive( drive )
{
   connect( drive, SIGNAL(prepareProgress(int,int)), this, SLOT(onPrepareProgress(int,int)) );
   connect( drive, SIGNAL(labelProgress(int,int)), this, SLOT(onLabelProgress(int,int)) );
   connect( drive, SIGNAL(timeEstimate(int)), this, SLOT(onTimeEstimate(int)) );
   connect( drive, SIGNAL(finished(int)), this, SLOT(onFinished(int)) );

}

QConsolePrintProgress::~QConsolePrintProgress()
{
}

void QConsolePrintProgress::onPrepareProgress( int current, int final )
{
   std::cout << "Preparing label: " << current << '/' << final << "         \r";
}

void QConsolePrintProgress::onLabelProgress( int current, int final )
{
   std::cout << "Printing label: " << current << '/' << final << "         \r";
}

void QConsolePrintProgress::onTimeEstimate( int time )
{
   std::cout << std::endl;
}

void QConsolePrintProgress::onFinished( int status )
{
   std::cout << "\n";
   if( !status )
      std::cout << "Print successfull" << std::endl;
   else
      std::cout << "Print failed: 0x" << QString::number( status, 16 ) << std::endl;

   qApp->exit( status );
}
