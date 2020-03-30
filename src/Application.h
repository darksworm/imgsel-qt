#pragma once

#include "gui/SettingsWindow.h"
#include "gui/MainWindow.h"
#include <QtWidgets/QApplication>
#include <QtCore/QSharedMemory>
#include <optional>

class Application : public QApplication {
Q_OBJECT
public:
    Application(int &argc, char **argv, bool oneShotMode);

    ~Application() override;

    bool lock();

    void setMainWindow(MainWindow *window);
    void setSettingsWindow(SettingsWindow *window);
    void setPathToExecutable(QString pathToExecutable);

    static QString defaultLibraryDirectory();
    bool isOneShotMode();

signals:
    void failedToRegisterHotkey(QString hotkey);
    void successfullyRegisteredHotkey(QString hotkey);

public slots:
    void hotkeyBindingChange(QString newBinding);
    void launchOnStartupChanged(int state);

private:
    QSharedMemory *_singular;

    MainWindow *mainWindow;
    SettingsWindow *settingsWindow;

    bool oneShotMode = false;
    QString pathToExecutable = "";

    std::optional<QMetaObject::Connection> hotkeyConnection = std::nullopt;
};
