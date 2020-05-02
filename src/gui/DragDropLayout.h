#pragma once
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <memory>

class DragDropLayout : public QVBoxLayout {
Q_OBJECT;
public:
    DragDropLayout(QWidget *widget);
    void noSuitableFilesDropped();
    void reset();
public slots:
    void importStarted();
    void importFinished();
    void importFailed(QString errorMessage);
signals:
    void expired();
private:
    std::unique_ptr<QLabel> textLabel;
    QTimer expireTimer;
};
