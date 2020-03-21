#pragma once

#include "gui/SettingsWindow.h"
#include "gui/MainWindow.h"
#include <QtWidgets/QApplication>
#include <QtCore/QSharedMemory>

class Application : public QApplication {
Q_OBJECT
public:
    Application(int &argc, char **argv);

    ~Application() override;

    bool lock();

    void setMainWindow(MainWindow *window);
    void setSettingsWindow(SettingsWindow *window);

public slots:
    void hotkeyBindingChanged(QString newBinding);

private:
    QSharedMemory *_singular;

    MainWindow *mainWindow;
    SettingsWindow *settingsWindow;

    QMetaObject::Connection hotkeyConnection;
    bool hotkeyConnected = false;
};
