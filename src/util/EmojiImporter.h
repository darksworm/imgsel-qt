#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QNetworkAccessManager>
#include "Unzipper.h"
#include "FileDownloader.h"
#include "config/Config.h"

class EmojiImporter : public QObject {
Q_OBJECT;
public:
    EmojiImporter(QString outputDirectoryPath, QList<QUrl> files);
    ~EmojiImporter();
    void start();
public slots:
    void downloadFinished();
    void downloadFailed();
signals:
    void started();
    void imported();
    void failed(QString errorMessage);
private:
    void startNextDownload();
    void startNextExtraction();
    void startNextCopy();
    void errorOut(QString actionName, QString filePath);

    QString getLocalFilePath(QString url);

    QString outputDirectoryPath;

    QList<QString> filesToDownload;
    QList<QString> filesToCopy;
    QList<QString> filesToExtract;

    QString downloadingFile;
    QString extractingFile;
    
    std::unique_ptr<FileDownloader> downloader;
    std::unique_ptr<Unzipper> unzipper;

    QTemporaryDir tempDir;
    QNetworkAccessManager manager;
};
