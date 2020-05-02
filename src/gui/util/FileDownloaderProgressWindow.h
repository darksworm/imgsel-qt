#pragma once

#include <QObject>
#include <QProgressDialog>

class FileDownloaderProgressWindow : public QObject {
Q_OBJECT;
public:
    FileDownloaderProgressWindow();
    ~FileDownloaderProgressWindow();
    void show();
    void cancelDownload();
    void dontDisplayErrorMessages();
public slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFail();
    void onDownloadFinish();
signals:
    void cancelRequested();
private:
    bool rangeSet = false;
    bool toasted = false;
    bool noErrorMessages = false;
    QProgressDialog progressDialog;
};
