#include "Application.h"
#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif
#include <QStandardPaths>
#include <project_config.h>

#include <iostream>


Application::Application(int &argc, char *argv[], bool oneShotMode) : QApplication(argc, argv, true) {
    _singular = new QSharedMemory("EMOJIGUN", this);

    this->oneShotMode = oneShotMode;
    setQuitOnLastWindowClosed(oneShotMode);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
        this, &Application::versionRequestFinished);
}

Application::~Application() {
    if (_singular->isAttached()) {
        _singular->detach();
    }
}

bool Application::lock() {
    if (_singular->attach(QSharedMemory::ReadOnly)) {
        _singular->detach();
        return false;
    }

    return _singular->create(1);
}

void Application::hotkeyBindingChange(QString newBinding) {
    auto hotkey = new QHotkey(QKeySequence(newBinding), true, this);

    if (!hotkey->isRegistered()) {
        emit failedToRegisterHotkey(newBinding);        

        return;
    }

    if (hotkeyConnection.has_value()) {
        disconnect(hotkeyConnection.value());
        hotkeyConnection.reset();
    }

    hotkeyConnection = connect(
        hotkey, &QHotkey::activated,
        this, [&]() {
            if(!settingsWindow->isChangingHotkey()) {
                mainWindow->display(true);
            }
        }
    );

    emit successfullyRegisteredHotkey(newBinding);
}

void Application::setMainWindow(MainWindow *window) {
    this->mainWindow = window;

#ifdef WITH_X11
    // fix for app not being put in true 0,0 for X11 systems with window managers that
    // have reserved space for statusbars etc. https://github.com/darksworm/imgsel-x11/issues/23
    connect(
        mainWindow, &MainWindow::displayed,
        this, [&](WId id) {
            XMoveWindow(QX11Info::display(), mainWindow->winId(), 0, 0);
        }
    );
#endif

    connect(
        mainWindow, &MainWindow::exitInstructionReceived,
        this, [&]() {
            if (oneShotMode) {
                exit(0);
            } else {
                mainWindow->hide();
            }
        }
    );
}

void Application::setSettingsWindow(SettingsWindow *window) {
    this->settingsWindow = window;

    connect(
        settingsWindow, &SettingsWindow::hotkeyBindingChange, 
        this, &Application::hotkeyBindingChange
    );
}

QString Application::defaultLibraryDirectory() {
    auto homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    return homeDir + "/" + ".emojigun";
}

bool Application::isOneShotMode() {
    return oneShotMode;
}

void Application::setPathToExecutable(QString pathToExecutable) {
    this->pathToExecutable = pathToExecutable;
}

void Application::launchOnStartupChanged(int state) {
    auto appDataLocation = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first();
    auto emojigunExeInstallPath = appDataLocation + QDir::separator() + "emojigun.exe";

    QSettings bootUpSettings(
        "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
        QSettings::NativeFormat
    );

    if (state) {
        QDir emojigunDir(appDataLocation);
        if (!emojigunDir.exists()) {
            emojigunDir.mkpath(".");
        }

        // trust me windows defender, i'm totally not a virus
        QFile::remove(emojigunExeInstallPath);
        QFile::copy(pathToExecutable, emojigunExeInstallPath);
        bootUpSettings.setValue("emojigun", emojigunExeInstallPath.replace('/', '\\'));
    } else {
        // I'd love to delete the dumped exe, but windows won't allow it if it is
        // running, so i guess we're just leaving it there to rot on the users system
        // QFile::remove(emojigunExeInstallPath);
        bootUpSettings.remove("emojigun");
    }
}

void Application::checkForUpdates() {
    QNetworkRequest request;
    request.setUrl(QUrl("https://version.emojigun.com"));

    networkManager->get(QNetworkRequest(request));
}

void Application::versionRequestFinished(QNetworkReply *reply) {
    QString contents = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(contents.toUtf8());
    QJsonObject obj = doc.object();
    
    auto jsonMap = obj.toVariantMap();

    auto version = jsonMap.take("version").toString();
    auto downloadURL = jsonMap.take("downloadURL").toString();

    auto responseVersion = QVersionNumber::fromString(version);
    auto thisAppVersion = QVersionNumber::fromString(PROJECT_VER);

    if (responseVersion > thisAppVersion) {
        QMessageBox msgBox;

        QPushButton *downloadNewVersionBtn = msgBox.addButton("Download", QMessageBox::ActionRole);
        msgBox.addButton("Ok", QMessageBox::ActionRole);

        msgBox.setParent(nullptr);
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.setWindowTitle("New EMOJIGUN version available!");
        msgBox.setText(
            tr("A new version of EMOJIGUN is available!<br>"
                "Version %1 has been released!").arg(version)
        );

        msgBox.exec();

        if (msgBox.clickedButton() == downloadNewVersionBtn) {
            QDesktopServices::openUrl(downloadURL);
        }
    }
}
