#pragma once

#include <QObject>
#include <QProgressDialog>

class FileDownloaderProgressWindow : public QObject {
Q_OBJECT;
public:
    FileDownloaderProgressWindow();
    void show();
    void cancelDownload();
public slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFail();
    void onDownloadFinish();
signals:
    void cancelRequested();
private:
    bool rangeSet = false;
    bool toasted = false;
    QProgressDialog progressDialog;
};
