#include "EmojiZipDownloader.h"

EmojiZipDownloader::EmojiZipDownloader(QNetworkAccessManager* manager, QString downloadUrl, QString outputPath) {
    this->outputPath = outputPath;

    // temporary files only get a filename once they have
    // been opened once
    zipFile.open();
    zipFile.close();

    downloader = new FileDownloader(manager, downloadUrl, zipFile.fileName());

    connect(
        downloader, &FileDownloader::finished,
        this, &EmojiZipDownloader::downloaded
    );

    connect(
        downloader, &FileDownloader::IOError,
        this, &EmojiZipDownloader::showErrorMessage
    );

    connect(
        downloader, &FileDownloader::error,
        this, &EmojiZipDownloader::downloadError
    );
}

EmojiZipDownloader::~EmojiZipDownloader() {
    delete downloader;
    delete unzipper;
}

void EmojiZipDownloader::downloadAndExtract() {
    downloader->start();
}

void EmojiZipDownloader::downloaded() {
    delete unzipper;

    unzipper = new Unzipper(zipFile.fileName());
    bool result = unzipper->unzipAllFilesToPath(outputPath);

    if (!result) {
        showErrorMessage();
    } else {
        emit done();
    }
}

void EmojiZipDownloader::showErrorMessage() {

}

void EmojiZipDownloader::downloadError(QNetworkReply::NetworkError error) {

}
