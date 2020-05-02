#include "EmojiImporter.h"

EmojiImporter::EmojiImporter(QString outputDirectoryPath, QList<QUrl> files) {
    this->outputDirectoryPath = outputDirectoryPath;
    auto imageExtensions = Config::getImageExtensions();

    foreach (auto fileUrl, files) {
        auto fileName = fileUrl.fileName().toLower();

        if (!fileUrl.isLocalFile()) {
            filesToDownload.append(fileUrl.toString());
            continue;
        }

        if (fileName.endsWith(".zip")) {
            filesToExtract.append(fileUrl.toLocalFile());
            continue;
        }

        for (const auto &ext : imageExtensions) {
            if (fileName.endsWith(QString(".") + ext.c_str())) {
                filesToCopy.append(fileUrl.toLocalFile());
                continue;
            }
        }
    }
}

EmojiImporter::~EmojiImporter() {
    tempDir.remove();
}

void EmojiImporter::start() {
    emit started();
    startNextDownload();
}

void EmojiImporter::startNextDownload() {
    if (filesToDownload.isEmpty()) {
        startNextExtraction();
        return;
    }
    
    downloadingFile = filesToDownload.takeFirst();
    downloader = std::make_unique<FileDownloader>(&manager, downloadingFile, getLocalFilePath(downloadingFile));

    connect(&*downloader, &FileDownloader::finished, this, &EmojiImporter::downloadFinished);
    connect(&*downloader, &FileDownloader::error, this, &EmojiImporter::downloadFailed);
    connect(&*downloader, &FileDownloader::IOError, this, &EmojiImporter::downloadFailed);

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
    if (filesToExtract.isEmpty()) {
        startNextCopy();
        return;
    }

    extractingFile = filesToExtract.takeFirst();
    unzipper = std::make_unique<Unzipper>(extractingFile);

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

    qDebug() << "new" << newFilePath << "old" << pathToCopy;

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
