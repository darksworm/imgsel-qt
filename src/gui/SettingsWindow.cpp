#include "SettingsWindow.h"
#include "MainWindow.h"
#include "../util/config/Config.h"
#include "../util/config/ConfigManager.h"

SettingsWindow::SettingsWindow(MainWindow *window) {
    this->window = window;

    createActions();
    createUI();
    createTrayIcon();

    connect(hotkeyChangeButton, &QAbstractButton::clicked, this, &SettingsWindow::onHotkeyChangeButton);
    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &SettingsWindow::messageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &SettingsWindow::iconActivated);

    window->setGeometry(ConfigManager::getOrLoadConfig().getScreenGeometry());

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(hotkeyLabel);
    mainLayout->addWidget(hotkeyChangeButton);
    setLayout(mainLayout);


    trayIcon->show();
}

void SettingsWindow::messageClicked() {
    QMessageBox::information(nullptr, tr("Systray"),
                             tr("Sorry, I already gave what help I could.\n"
                                "Maybe you should try asking a human?"));
}

void SettingsWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            break;
        case QSystemTrayIcon::MiddleClick:
            break;
        default:
            ;
    }
}

void SettingsWindow::setVisible(bool visible) {
    selectorAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}

void SettingsWindow::closeEvent(QCloseEvent *event) {
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    if (trayIcon->isVisible()) {
        QMessageBox::information(this, tr("Systray"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        event->ignore();
    }
}

void SettingsWindow::createTrayIcon() {
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(selectorAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void SettingsWindow::createActions() {
    selectorAction = new QAction(tr("&Open selector"), this);
    connect(selectorAction, &QAction::triggered, window, &MainWindow::display);

    restoreAction = new QAction(tr("&Settings"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void SettingsWindow::createUI() {
    hotkeyLabel = new QLabel(("Hotkey: " + settings.value("hotkey_sequence", "meta+x").toString()));
    hotkeyChangeButton = new QPushButton("Change");
}

void SettingsWindow::onHotkeyChangeButton() {
    if (!changingHotkey) {
        hotkeyLabel->setText("Press the button combination you want to bind...");
        hotkeyChangeButton->setText("Save");
    } else {
        settings.setValue("hotkey_sequence", hotkeyAccumulator);
        hotkeyLabel->setText("Hotkey: " + hotkeyAccumulator);
        hotkeyChangeButton->setText("Change");
        hotkeyAccumulator = "";

        emit hotkeyBindingChanged(hotkeyAccumulator);
    }

    changingHotkey = !changingHotkey;
}

void SettingsWindow::keyPressEvent(QKeyEvent *event) {
    QDialog::keyPressEvent(event);

    if (changingHotkey) {
        auto index = event->key();
        QString keyName;

        if (index == Qt::Key_Shift) {
            keyName = "shift";
        } else if (index == Qt::Key_Control) {
            keyName = "ctrl";
        } else if (index == Qt::Key_AltGr || index == Qt::Key_Alt) {
            keyName = "alt";
        } else if (index == Qt::Key_Meta || index == Qt::Key_Super_L || index == Qt::Key_Super_R) {
            keyName = "meta";
        } else {
            keyName = QKeySequence(index).toString();
        }

        hotkeyAccumulator += (hotkeyAccumulator.length() ? "+" : "") + keyName.toLower();
        hotkeyLabel->setText("Hotkey (recording): " + hotkeyAccumulator);
    }
}

