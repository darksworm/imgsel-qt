#pragma once

#include <QtWidgets/QApplication>
#include <QtCore/QSharedMemory>
#include <optional>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include "../util/FileDownloader.h"
#include "../gui/SettingsWindow.h"
#include "../gui/MainWindow.h"
#include "ApplicationUpdater.h"

#define emojigunApp ((Application*) qApp)
#define emojigunSettings (emojigunApp->getSettings())
#define emojigunNetworkManager (emojigunApp->getNetworkManager())
#define emojigunUpdater (emojigunApp->getUpdater())

class Application : public QApplication {
Q_OBJECT
public:
    Application(int &argc, char **argv, bool oneShotMode);

    ~Application() override;

    bool lock();

    void setMainWindow(MainWindow *window);
    void setSettingsWindow(SettingsWindow *window);

    static QString defaultLibraryDirectory();
    bool isOneShotMode();

    QString installDirectory();
    QSettings& getSettings() { return *settings; }
    QNetworkAccessManager* getNetworkManager() { return networkManager; }
    ApplicationUpdater& getUpdater() { return *updater; };

signals:
    void failedToRegisterHotkey(QString hotkey);
    void successfullyRegisteredHotkey(QString hotkey);

public slots:
    void hotkeyBindingChange(QString newBinding);
    void launchOnStartupChanged(int state);
    void updateAvailable(ApplicationVersionDetails details);

private:
    QSettings* settings;
    ApplicationUpdater* updater;

    QSharedMemory *_singular;

    MainWindow *mainWindow;
    SettingsWindow *settingsWindow;

    bool oneShotMode = false;
    QString pathToExecutable = "";

    std::optional<QMetaObject::Connection> hotkeyConnection = std::nullopt;
    QNetworkAccessManager *networkManager;

    FileDownloader *downloader;
};
