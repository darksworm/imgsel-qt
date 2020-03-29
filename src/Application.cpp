#include "Application.h"
#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif
#include <QStandardPaths>


Application::Application(int &argc, char *argv[], bool oneShotMode) : QApplication(argc, argv, true) {
    _singular = new QSharedMemory("EMOJIGUN", this);

    this->oneShotMode = oneShotMode;
    setQuitOnLastWindowClosed(oneShotMode);
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
