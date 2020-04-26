#include "FileDownloaderProgressWindow.h"
#include <QIcon>

FileDownloaderProgressWindow::FileDownloaderProgressWindow() {
    QIcon icon(":/assets/eyes-32x25.png");

    progressDialog.setWindowTitle("Downloading file...");
    progressDialog.setMinimumWidth(300);
    progressDialog.setWindowIcon(icon);

    connect(
        &progressDialog, &QProgressDialog::canceled,
        this, &FileDownloaderProgressWindow::cancelDownload
    );
}

void FileDownloaderProgressWindow::show() {
    progressDialog.open();
}

void FileDownloaderProgressWindow::onDownloadFail() {
    if (toasted) {
        return;
    }

    toasted = true;

    progressDialog.setLabelText("Download failed!");
    progressDialog.setCancelButtonText("Close");
}

void FileDownloaderProgressWindow::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (toasted) {
        return;
    }

    if (!rangeSet) {
        progressDialog.setRange(0, bytesTotal);
        rangeSet = true;
    }
    
    progressDialog.setValue(bytesReceived);
}

void FileDownloaderProgressWindow::onDownloadFinish() {
    if (toasted) {
        return;
    }

    progressDialog.setValue(progressDialog.maximum());
    progressDialog.setLabelText("Download finished!");
}

void FileDownloaderProgressWindow::cancelDownload() {
    if (toasted) {
        return;
    }

    toasted = true;
    emit cancelRequested();
}
