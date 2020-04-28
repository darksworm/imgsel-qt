#include "EmojiImporter.h"

EmojiImporter::EmojiImporter(QString outputDirectoryPath, QList<QUrl> files) {
    this->outputDirectoryPath = outputDirectoryPath;

    foreach (auto fileUrl, files) {
        auto fileName = fileUrl.fileName().toLower();

        if (!fileUrl.isLocalFile()) {
            filesToDownload.append(fileUrl.toString());
            continue;
        }

        if (fileName.endsWith(".zip")) {
            filesToExtract.append(fileUrl.path());
            continue;
        }

        for (const auto &ext : Config::getImageExtensions()) {
            if (fileName.endsWith(QString(".") + ext.c_str())) {
                filesToCopy.append(fileUrl.path());
                continue;
            }
        }
    }
}

EmojiImporter::~EmojiImporter() {
    delete downloader;
    delete unzipper;

    tempDir.remove();
}

void EmojiImporter::start() {
    startNextDownload();   
}

void EmojiImporter::startNextDownload() {
    delete downloader;

    if (filesToDownload.isEmpty()) {
        startNextExtraction();
        return;
    }
    
    downloadingFile = filesToDownload.takeFirst();
    downloader = new FileDownloader(&manager, downloadingFile, getLocalFilePath(downloadingFile));

    connect(
        downloader, &FileDownloader::finished,
        this, &EmojiImporter::downloadFinished
    );

    // TODO handle error here?

    downloader->start();
}

void EmojiImporter::downloadFailed() {}

void EmojiImporter::downloadFinished() {
    auto downloadedFilePath = getLocalFilePath(downloadingFile);

    if (downloadingFile.toLower().endsWith(".zip")) {
        filesToExtract.append(downloadedFilePath);
    } else {
        filesToCopy.append(downloadedFilePath);
    }
    
    startNextDownload();
}

void EmojiImporter::startNextExtraction() {
    delete unzipper;

    if (filesToExtract.isEmpty()) {
        startNextCopy();
        return;
    }

    extractingFile = filesToExtract.takeFirst();
    unzipper = new Unzipper(extractingFile);

    if (unzipper->unzipAllFilesToPath(outputDirectoryPath)) {
        startNextExtraction();
        return;
    }

    // TODO handle error here
}

void EmojiImporter::startNextCopy() {
    if (filesToCopy.isEmpty()) {
        emit imported();
        return;
    }

    QDir targetDirectory(outputDirectoryPath);

    auto pathToCopy = filesToCopy.takeFirst();
    QFile fileToCopy(pathToCopy);
    QFileInfo fileToCopyInfo(fileToCopy.fileName());

    auto newFilePath = targetDirectory.filePath(fileToCopyInfo.fileName());

    // TODO file already exists?
    
    if (fileToCopy.rename(newFilePath)) {
        startNextCopy();
        return;
    }

    // TODO handle error here
}

QString EmojiImporter::getLocalFilePath(QString url) {
    auto downloadingFileName = QUrl(downloadingFile).fileName();

    return tempDir.filePath(downloadingFileName);
}
