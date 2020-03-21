#include "Application.h"
#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif

#include <iostream>

Application::Application(int &argc, char *argv[]) : QApplication(argc, argv, true) {
    _singular = new QSharedMemory("IMGSEL", this);
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

void Application::hotkeyBindingChanged(QString newBinding) {
    if (hotkeyConnected) {
        disconnect(hotkeyConnection);
    }

    auto hotkey = new QHotkey(QKeySequence(newBinding), true, this);
    hotkeyConnection = connect(
        hotkey, &QHotkey::activated,
        this, [&]() {
            mainWindow->display();
        }
    );
}

void Application::setMainWindow(MainWindow *window) {
    this->mainWindow = window;

#ifdef WITH_X11
    connect(
        mainWindow, &MainWindow::displayed,
        this, [&](WId id) {
            XMoveWindow(QX11Info::display(), mainWindow->winId(), 0, 0);
        }
    );
#endif
}

void Application::setSettingsWindow(SettingsWindow *window) {
    this->settingsWindow = window;

    connect(
        settingsWindow, &SettingsWindow::hotkeyBindingChanged, 
        this, &Application::hotkeyBindingChanged
    );
}
