#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QtWidgets>
#include <QHotkey>
#include "MainWindow.h"

class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit SettingsWindow(MainWindow* window);
    void setVisible(bool visible) override;

signals:
    void hotkeyBindingChanged(QString newBinding);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void messageClicked();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onHotkeyChangeButton();

private:
    void createTrayIcon();
    void createActions();
    void createUI();

    QHotkey *mainWindowHotkey;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QAction *selectorAction;
    QAction *restoreAction;
    QAction *quitAction;

    MainWindow* window;

    QSettings settings = QSettings("EMOJIGUN", "EMOJIGUN");

    bool changingHotkey = false;
    QString hotkeyAccumulator = "";
    QLabel* hotkeyLabel;
    QPushButton* hotkeyChangeButton;
};



