#pragma once
#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QFile>

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
    FileDownloader(QString downloadUrl, QString targetFilePath);
    ~FileDownloader();
    void start();
    void pause();
    void cancel();
private:
    bool used = false;
    QString downloadUrl;

    QFile* file = nullptr;
    QNetworkReply * reply = nullptr;

    QNetworkAccessManager manager;
};
