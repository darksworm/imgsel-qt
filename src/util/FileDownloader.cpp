#include "FileDownloader.h"
#include <memory>
#include <utility>

FileDownloader::FileDownloader(QNetworkAccessManager* networkManager, QString downloadUrl, QString targetFilePath) {
    this->manager = networkManager;
    this->downloadUrl = std::move(downloadUrl);
    this->targetFilePath = std::move(targetFilePath);

    file = std::make_unique<QFile>(this->targetFilePath);
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

    reply.reset(manager->get(request));

    connect(
        &*reply, &QNetworkReply::downloadProgress,
        this, &FileDownloader::downloadProgress
    );

    connect(
        &*reply, &QNetworkReply::finished,
        this, &FileDownloader::downloadFinished
    );

    connect(
        &*reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
        this, &FileDownloader::downloadError
    );

    used = true;
}

void FileDownloader::setupGUI() {
    progressWindow = std::make_unique<FileDownloaderProgressWindow>();

    if (noErrorMessages) {
        progressWindow->dontDisplayErrorMessages();
    }

    connect(
        this, &FileDownloader::IOError,
        &*progressWindow, &FileDownloaderProgressWindow::onDownloadFail
    );

    connect(
        this, &FileDownloader::error,
        &*progressWindow, &FileDownloaderProgressWindow::onDownloadFail
    );

    connect(
        this, &FileDownloader::progress,
        &*progressWindow, &FileDownloaderProgressWindow::onDownloadProgress
    );

    connect(
        this, &FileDownloader::finished,
        &*progressWindow, &FileDownloaderProgressWindow::onDownloadFinish
    );

    connect(
        &*progressWindow, &FileDownloaderProgressWindow::cancelRequested,
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

void FileDownloader::cancel() {
    reply->abort();
}

void FileDownloader::dontDisplayErrorMessages() {
    noErrorMessages = true;
}
