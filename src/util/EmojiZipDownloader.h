#pragma once

#include <QObject>
#include "FileDownloader.h"
#include "Unzipper.h"
#include <QTemporaryFile>

class EmojiZipDownloader : public QObject {
Q_OBJECT;
public:
    EmojiZipDownloader(QNetworkAccessManager* manager, QString zipUrl, QString outputPath);
    ~EmojiZipDownloader() override;
    void downloadAndExtract();
public slots:
    void downloaded();
    void downloadError(QNetworkReply::NetworkError error);
signals:
    void done();
private:
    QTemporaryFile zipFile;
    FileDownloader *downloader = nullptr;
    Unzipper *unzipper = nullptr;

    QString outputPath;

    void showErrorMessage();
};
