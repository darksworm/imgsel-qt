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
    
    FileDownloader *downloader = nullptr;
    Unzipper *unzipper = nullptr;

    QTemporaryDir tempDir;
    QNetworkAccessManager manager;
};
