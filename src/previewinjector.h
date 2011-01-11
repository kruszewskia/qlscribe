#ifndef PREVIEWINJECTOR_HPP
#define PREVIEWINJECTOR_HPP

#include <QObject>
#include <QSize>

class QFileDialog;
class QLabel;

class PreviewInjector : public QObject {
    Q_OBJECT
public:

    PreviewInjector( QFileDialog * dl );
    ~PreviewInjector();

    bool injected() const { return m_label; }

private slots:
    void onSelectionChanged(QString );

private:
    QLabel *m_label;
    QSize   m_size;
};

#endif // PREVIEWINJECTOR_HPP
