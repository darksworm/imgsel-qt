#include "FileDownloader.h"
#include <iostream>

FileDownloader::FileDownloader(QNetworkAccessManager* networkManager, QString downloadUrl, QString targetFilePath) {
    this->manager = networkManager;
    this->downloadUrl = downloadUrl;
    this->targetFilePath = targetFilePath;

    file = new QFile(this->targetFilePath);
}

void FileDownloader::start() {
    if (used) {
        throw -1;
    }

    if (!file->open(QIODevice::WriteOnly)) {
        emit IOError();
        return;
    }

    QNetworkRequest request(downloadUrl);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    reply = manager->get(request);

    connect(
        reply, &QNetworkReply::downloadProgress,
        this, &FileDownloader::downloadProgress
    );

    connect(
        reply, &QNetworkReply::finished,
        this, &FileDownloader::downloadFinished
    );

    connect(
        reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
        this, &FileDownloader::downloadError
    );

    used = true;
}


void FileDownloader::downloadFinished() {
    file->write(reply->readAll());
    file->close();

    emit finished();
}

void FileDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    emit progress(bytesReceived, bytesTotal);
}

void FileDownloader::downloadError(QNetworkReply::NetworkError code) {
    emit error(code);
}

FileDownloader::~FileDownloader() {
    delete file;
    delete reply;
}
