#ifndef QDIALOGPRINT_H
#define QDIALOGPRINT_H

#include <QtGui/QDialog>

#include "qlightscribe.h"

namespace Ui {
    class QDialogPrint;
}

class QDialogPrint : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(QDialogPrint)
public:
   static QLightDrive *exec( QWidget *parent, QLightScribe::PrintParameters &params );

protected:
    explicit QDialogPrint(QWidget *parent = 0);
    virtual ~QDialogPrint();

    virtual void changeEvent(QEvent *e);

private:
    Ui::QDialogPrint *m_ui;
};

#endif // QDIALOGPRINT_H
