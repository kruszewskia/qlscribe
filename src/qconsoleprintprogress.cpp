#include "qconsoleprintprogress.h"
#include "qlightscribe.h"

#include <QApplication>

#include <iostream>

std::ostream &operator<<( std::ostream &os, const QString &str );

QConsolePrintProgress::QConsolePrintProgress( QLightDrive *drive )
      : m_drive( drive )
{
   connect( drive, SIGNAL(prepareProgress(long,long)), this, SLOT(onPrepareProgress(long,long)) );
   connect( drive, SIGNAL(labelProgress(long,long)), this, SLOT(onLabelProgress(long,long)) );
   connect( drive, SIGNAL(timeEstimate(long)), this, SLOT(onTimeEstimate(long)) );
   connect( drive, SIGNAL(finished(int)), this, SLOT(onFinished(int)) );

}

QConsolePrintProgress::~QConsolePrintProgress()
{
}

void QConsolePrintProgress::onPrepareProgress( long current, long final )
{
   std::cout << "Preparing label: " << current << '/' << final << "         \r";
}

void QConsolePrintProgress::onLabelProgress( long current, long final )
{
   std::cout << "Printing label: " << current << '/' << final << "         \r";
}

void QConsolePrintProgress::onTimeEstimate( long time )
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
