#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QtWidgets>
#include "MainWindow.h"

class SettingsWindow : public QDialog {
public:
    explicit SettingsWindow(MainWindow* window);
    void setVisible(bool visible) override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void setIcon(int index);
    void messageClicked();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void createTrayIcon();
    void createActions();

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QAction *minimizeAction;
    QAction *selectorAction;
    QAction *restoreAction;
    QAction *quitAction;

    MainWindow* window;
};



