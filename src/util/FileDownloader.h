#pragma once
#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include "../gui/util/FileDownloaderProgressWindow.h"

class FileDownloader : public QObject {
Q_OBJECT;
signals:
    void finished();
    void progress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void IOError();
public slots:
    void downloadFinished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadError(QNetworkReply::NetworkError code);
public:
    FileDownloader(QNetworkAccessManager* networkManager, QString downloadUrl, QString targetFilePath);
    ~FileDownloader() override;
    void dontDisplayErrorMessages();
    void start();
    void cancel();
private:
    void setupGUI();

    bool used = false;
    bool noErrorMessages = false;

    QString downloadUrl;
    QString targetFilePath;

    QFile* file = nullptr;
    QNetworkReply* reply = nullptr;
    QNetworkAccessManager* manager = nullptr;
    FileDownloaderProgressWindow* progressWindow = nullptr;
};
