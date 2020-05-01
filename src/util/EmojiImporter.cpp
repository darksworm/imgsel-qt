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

    connect(downloader, &FileDownloader::finished, this, &EmojiImporter::downloadFinished);
    connect(downloader, &FileDownloader::error, this, &EmojiImporter::downloadFailed);
    connect(downloader, &FileDownloader::IOError, this, &EmojiImporter::downloadFailed);

    downloader->start();
}

void EmojiImporter::downloadFailed() {
    errorOut("download", downloadingFile);
}

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

    errorOut("extract", extractingFile);
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

    QFile targetFile(newFilePath);
    if (fileToCopy.copy(newFilePath) || targetFile.exists()) {
        startNextCopy();
        return;
    }

    errorOut("copy", pathToCopy);
}

QString EmojiImporter::getLocalFilePath(QString url) {
    auto downloadingFileName = QUrl(downloadingFile).fileName();

    return tempDir.filePath(downloadingFileName);
}

void EmojiImporter::errorOut(QString actionName, QString filePath) {
    QFileInfo fileInfo(filePath);

    QString errorMessage = "Failed to " + actionName + " file " +  fileInfo.fileName();
    emit failed(errorMessage);
}
