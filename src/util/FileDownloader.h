#pragma once
#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include "../gui/util/FileDownloaderProgressWindow.h"
#include <memory>

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
    void dontDisplayErrorMessages();
    void start();
    void cancel();
private:
    void setupGUI();

    bool used = false;
    bool noErrorMessages = false;

    QString downloadUrl;
    QString targetFilePath;

    QNetworkAccessManager* manager = nullptr;
    std::unique_ptr<QFile> file = nullptr;
    std::unique_ptr<QNetworkReply> reply = nullptr;
    std::unique_ptr<FileDownloaderProgressWindow> progressWindow = nullptr;
};
