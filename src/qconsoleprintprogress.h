#ifndef QCONSOLEPRINTPROGRESS_H
#define QCONSOLEPRINTPROGRESS_H

#include <QObject>

class QLightDrive;

class QConsolePrintProgress : public QObject {
   Q_OBJECT
public:
   QConsolePrintProgress( QLightDrive *drive );
   ~QConsolePrintProgress();

private slots:
   void onPrepareProgress( int current, int final );
   void onLabelProgress( int current, int final );
   void onTimeEstimate( int time );
   void onFinished(int status );

private:
   QLightDrive *m_drive;
};

#endif // QCONSOLEPRINTPROGRESS_H
