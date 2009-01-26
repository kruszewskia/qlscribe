#ifndef QDIALOGPROGRESS_H
#define QDIALOGPROGRESS_H

#include <QtGui/QDialog>

namespace Ui {
    class QDialogProgress;
}

class QCDScene;
class QAbstractButton;

class QDialogProgress : public QDialog {
   Q_OBJECT
   Q_DISABLE_COPY(QDialogProgress)
public:
   static bool exec( QWidget *parent, QCDScene *scene );

protected slots:
   void onButtonClicked( QAbstractButton* button );
   void onPrepareProgress( long current, long final );
   void onLabelProgress( long current, long final );
   void onTimeEstimate( long time );
   void onFinished(int status );

protected:
    virtual void changeEvent(QEvent *e);

private:
    explicit QDialogProgress(QWidget *parent = 0);
    virtual ~QDialogProgress();

    Ui::QDialogProgress *m_ui;
};

#endif // QDIALOGPROGRESS_H
