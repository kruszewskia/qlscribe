#include "qconsoleprintprogress.h"
#include "qlightscribe.h"

#include <QApplication>

#include <iostream>

QConsolePrintProgress::QConsolePrintProgress()
{
   QLightScribe *scribe = QLightScribe::instance();

   connect( scribe, SIGNAL(prepareProgress(long,long)), this, SLOT(onPrepareProgress(long,long)) );
   connect( scribe, SIGNAL(labelProgress(long,long)), this, SLOT(onLabelProgress(long,long)) );
   connect( scribe, SIGNAL(timeEstimate(long)), this, SLOT(onTimeEstimate(long)) );
   connect( scribe, SIGNAL(finished(int)), this, SLOT(onFinished(int)) );

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
      std::cout << "Prin failed: 0x" << QString::number( status, 16 ) << std::endl;

   QLightScribe *scribe = QLightScribe::instance();
   scribe->stopThread();
   scribe->wait( 1000 );
   qApp->exit( status );
}
