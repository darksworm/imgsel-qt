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

    setupGUI();

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

void FileDownloader::setupGUI() {
    progressWindow = new FileDownloaderProgressWindow();

    connect(
        this, &FileDownloader::IOError,
        progressWindow, &FileDownloaderProgressWindow::onDownloadFail
    );

    connect(
        this, &FileDownloader::error,
        progressWindow, &FileDownloaderProgressWindow::onDownloadFail
    );

    connect(
        this, &FileDownloader::progress,
        progressWindow, &FileDownloaderProgressWindow::onDownloadProgress
    );

    connect(
        this, &FileDownloader::finished,
        progressWindow, &FileDownloaderProgressWindow::onDownloadFinish
    );

    connect(
        progressWindow, &FileDownloaderProgressWindow::cancelRequested,
        this, [&]() { cancel(); }
    );

    progressWindow->show();
}

void FileDownloader::downloadFinished() {
    if (reply->error()) {
        downloadError(reply->error());
        return;
    }

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

void FileDownloader::cancel() {
    reply->abort();
}
