#include "Application.h"
#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif
#include <QStandardPaths>
#include <project_config.h>

Application::Application(int &argc, char *argv[], bool oneShotMode) : QApplication(argc, argv, true) {
    _singular = new QSharedMemory("EMOJIGUN", this);
    settings = new QSettings("EMOJIGUN", "EMOJIGUN");
    updater = new ApplicationUpdater();
    networkManager = new QNetworkAccessManager(this);

    this->oneShotMode = oneShotMode;
    setQuitOnLastWindowClosed(oneShotMode);

    const QIcon eyesIcon(":/assets/eyes-32x25.png");
    setWindowIcon(eyesIcon);

    connect(
        updater, &ApplicationUpdater::updateAvailable,
        this, &Application::updateAvailable
    );

    connect(
        updater, &ApplicationUpdater::updateReady,
        this, &Application::updateReady
    );
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

    connect(
        mainWindow, &MainWindow::imageCopied,
        this, [&]() {
            if (oneShotMode) {
                return;
            }         

            bool hasCopiedOnce = getSettings().value("has_copied_once", false).toBool();

            if (!hasCopiedOnce) {
                QMessageBox::information(mainWindow, tr("Image copied to clipboard"),
                                         tr("The selected image has been copied to the clipboard, "
                                            "you can now paste it with CTRL + V."));

                getSettings().setValue("has_copied_once", true);
            }
        }
    );

    connect(
        mainWindow, &MainWindow::noImagesToDisplay,
        this, &Application::noImagesToDisplay
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

QString Application::installDirectory() {
    return QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).at(0);
}

bool Application::isOneShotMode() {
    return oneShotMode;
}

void Application::launchOnStartupChanged(int state) {
    auto appDataLocation = installDirectory();
    auto emojigunExeInstallPath = updater->getPathToInstalledExe();

    QSettings bootUpSettings(
        "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
        QSettings::NativeFormat
    );

    if (state) {
        bootUpSettings.setValue("emojigun", emojigunExeInstallPath.replace('/', '\\'));
    } else {
        bootUpSettings.remove("emojigun");
    }
}

void Application::updateAvailable(ApplicationVersionDetails details) {
    QMessageBox msgBox;

#if WIN32
    std::unique_ptr<QPushButton>downloadNewVersionBtn(msgBox.addButton("Download", QMessageBox::ActionRole));
#endif
    msgBox.addButton("Ok", QMessageBox::ActionRole);

    msgBox.setParent(nullptr);
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.setWindowTitle("New EMOJIGUN version available!");
    msgBox.setText(
        tr("A new version of EMOJIGUN is available!<br>"
            "Version %1 has been released!").arg(details.version)
    );

    msgBox.exec();

#if WIN32
    if (msgBox.clickedButton() == downloadNewVersionBtn.get()) {
        if (!updater->exeIsInstalled()) {
            QDesktopServices::openUrl(details.downloadUrl);
        } else {
            updater->updateToVersion(details);
        }
    }
#endif
}

void Application::updateReady(QString updaterPath) {
    bool success = QProcess::startDetached(updaterPath, QStringList("/S"));

    if (success) {
        Application::quit();
        return;
    }

    QMessageBox::critical(mainWindow, "Update failed!", "Couldn't update. Please try to update manually.");
}

void Application::noImagesToDisplay() {
    auto defaultDir = Application::defaultLibraryDirectory();
    QString imageDirFromSettings = emojigunSettings.value("library_path", defaultDir).toString();

    QMessageBox noImagesMsgBox(
        QMessageBox::Icon::Critical,
        "No images found!",
        "To use EMOJIGUN, you must first add emojis to your library. Please copy your emojis to your library folder at " + imageDirFromSettings + ".",
        QMessageBox::Close
    );

    std::unique_ptr<QPushButton> getEmojisBtn(noImagesMsgBox.addButton("Download top emojis pack", QMessageBox::ActionRole));
    std::unique_ptr<QPushButton> selectEmojisBtn(noImagesMsgBox.addButton("Create custom emoji pack", QMessageBox::ActionRole));

    noImagesMsgBox.setParent(nullptr);

    noImagesMsgBox.exec();
    noImagesMsgBox.raise();

    noImagesMsgBox.setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint | noImagesMsgBox.windowFlags());

    if (noImagesMsgBox.clickedButton() == getEmojisBtn.get()) {
        QString url = "https://s3-eu-west-1.amazonaws.com/emojigun.com/top-emojis.zip";
        zipDownloader = new EmojiZipDownloader(emojigunNetworkManager, url, imageDirFromSettings);

        connect(
            zipDownloader, &EmojiZipDownloader::done,
            this, [&]() { mainWindow->display(true); delete zipDownloader; }
        );

        zipDownloader->downloadAndExtract();
    }

    if (noImagesMsgBox.clickedButton() == selectEmojisBtn.get()) {
        QString link = "https://emojigun.com/#/custom";
        QDesktopServices::openUrl(link);
    }
}
