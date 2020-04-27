#pragma once

#include <QObject>
#include "FileDownloader.h"
#include "Unzipper.h"
#include <QTemporaryFile>

enum class EmojiZipError {
    IOError,
    NetworkError,
    UnarchivingError
};

class EmojiZipDownloader : public QObject {
Q_OBJECT;
public:
    EmojiZipDownloader(QNetworkAccessManager* manager, QString zipUrl, QString outputPath);
    ~EmojiZipDownloader() override;
    void downloadAndExtract();
public slots:
    void downloaded();
signals:
    void done();
    void failed();
private:
    QTemporaryFile zipFile;
    FileDownloader *downloader = nullptr;
    Unzipper *unzipper = nullptr;

    QString outputPath;

    void showErrorMessage(EmojiZipError cause);
};
