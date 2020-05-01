#pragma once
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

class DragDropLayout : public QVBoxLayout {
Q_OBJECT;
public:
    DragDropLayout(QWidget *widget);
    void noSuitableFilesDropped();
    void importStarted();
    void reset();
public slots:
    void importFinished();
    void importFailed(QString errorMessage);
signals:
    void expired();
private:
    QLabel *textLabel;
    QTimer expireTimer;
};
