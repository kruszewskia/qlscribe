#ifndef QCONSOLEPRINTPROGRESS_H
#define QCONSOLEPRINTPROGRESS_H

#include <QObject>

class QConsolePrintProgress : public QObject {
   Q_OBJECT
public:
   QConsolePrintProgress();
   ~QConsolePrintProgress();

private slots:
   void onPrepareProgress( long current, long final );
   void onLabelProgress( long current, long final );
   void onTimeEstimate( long time );
   void onFinished(int status );
};

#endif // QCONSOLEPRINTPROGRESS_H
