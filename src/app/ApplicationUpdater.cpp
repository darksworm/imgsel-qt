#include "ApplicationUpdater.h"
#include "Application.h"

ApplicationUpdater::~ApplicationUpdater() {
    newVersionTempDir.remove();

    delete downloader;
    delete unzipper;
}

void ApplicationUpdater::checkForUpdates() {
    QNetworkRequest request;
    request.setUrl(QUrl("https://version.emojigun.com"));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    delete checkForUpdatesReply;
    checkForUpdatesReply = emojigunNetworkManager->get(request);

    connect(
        checkForUpdatesReply, &QNetworkReply::finished,
        this, &ApplicationUpdater::checkForUpdatesFinished
    );
}

void ApplicationUpdater::checkForUpdatesFinished() {
    QString contents = checkForUpdatesReply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(contents.toUtf8());
    QJsonObject obj = doc.object();
    
    auto jsonMap = obj.toVariantMap();

    auto version = jsonMap.take("version").toString();
    auto downloadURL = jsonMap.take("downloadURL").toString();

    auto responseVersion = QVersionNumber::fromString(version);
    auto thisAppVersion = QVersionNumber::fromString(PROJECT_VER);

    if (responseVersion > thisAppVersion) {
        ApplicationVersionDetails versionDetails;
        versionDetails.version = version;
        versionDetails.downloadUrl = downloadURL;

        emit updateAvailable(versionDetails);
    }
}

QString ApplicationUpdater::getPathToUpdater() {
    return newVersionTempDir.filePath("emojigun-updater.exe");
}

void ApplicationUpdater::updateToVersion(ApplicationVersionDetails details) {
    delete downloader;

    downloader = new FileDownloader(
        emojigunNetworkManager, details.downloadUrl, getPathToUpdater()
    );

    QObject::connect(
        downloader, &FileDownloader::finished,
        this, &ApplicationUpdater::updateDownloaded
    );

    QObject::connect(
        downloader, &FileDownloader::error,
        this, [&](QNetworkReply::NetworkError error) {
            if (error != QNetworkReply::OperationCanceledError) {
                showErrorMsg();
            }
        }
    );

    QObject::connect(
        downloader, &FileDownloader::IOError,
        this, &ApplicationUpdater::showErrorMsg
    );

    downloader->start();
}

QString ApplicationUpdater::getPathToInstalledExe() {
    QDir appDataDir(emojigunApp->installDirectory());
    return appDataDir.filePath("emojigun.exe");
}

bool ApplicationUpdater::exeIsInstalled() {
    auto exePath = getPathToInstalledExe();
    return QFile::exists(exePath);
}

void ApplicationUpdater::setPathToExecutable(QString pathToExecutable) {
    this->pathToExecutable = pathToExecutable;
}

void ApplicationUpdater::updateDownloaded() {
    emit updateReady(getPathToUpdater());
}

void ApplicationUpdater::showErrorMsg() {
    QMessageBox errorBox;

    errorBox.setText("Update download failed!");
    errorBox.setWindowTitle("Update failed!");
    errorBox.setIcon(QMessageBox::Critical);

    errorBox.open();
}
