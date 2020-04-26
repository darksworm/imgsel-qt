#pragma once

#include <QObject>
#include "../util/FileDownloader.h"
#include "../util/Unzipper.h"
#include <project_config.h>
#include <QTemporaryFile>
#include <QTemporaryDir>

struct ApplicationVersionDetails {
    QString version;
    QString downloadUrl;
};

class ApplicationUpdater : public QObject {
Q_OBJECT;
public:
    void checkForUpdates();
    void updateToVersion(ApplicationVersionDetails details);

    void checkSavedExeVersion();

    bool exeIsInstalled();
    QString getPathToInstalledExe();

    void setPathToExecutable(QString pathToExecutable);
    QString getPathToExecutable() { return pathToExecutable; };

    ~ApplicationUpdater();
signals:
    void updateAvailable(ApplicationVersionDetails details);
    void updateReady(QString updaterPath);
    void updateFailed();
private slots:
    void checkForUpdatesFinished();
    void updateDownloaded();
    void updateDownloadFailed();
private:
    bool installedExeOlderThanLaunchedExe();
    QString getPathToUpdater();

    QString pathToExecutable;

    QTemporaryDir newVersionTempDir;
    FileDownloader *downloader = nullptr;
    Unzipper *unzipper = nullptr;

    QNetworkReply* checkForUpdatesReply;
};
