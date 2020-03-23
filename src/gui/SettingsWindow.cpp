#include "SettingsWindow.h"
#include "MainWindow.h"
#include "../util/config/Config.h"
#include "../util/config/ConfigManager.h"
#include "../Application.h"
#include <QHotkey>
#include <QStandardPaths>

#include <iostream>

SettingsWindow::SettingsWindow(MainWindow *window) {
    this->window = window;

    createActions();
    createUI();
    createTrayIcon();
    connectUI();

    trayIcon->show();
}

void SettingsWindow::connectUI() {
    connect(
        hotkeyChangeButton, &QAbstractButton::clicked, 
        this, &SettingsWindow::onHotkeyChangeButton
    );

    connect(
        hotkeyChangeCancelButton, &QAbstractButton::clicked, 
        this, &SettingsWindow::onHotkeyChangeCancelButton
    );

    connect(
        trayIcon, &QSystemTrayIcon::messageClicked, 
        this, &SettingsWindow::messageClicked
    );
    connect(
        trayIcon, &QSystemTrayIcon::activated, 
        this, &SettingsWindow::iconActivated
    );

    connect(
        startMinimizedCheckbox, &QCheckBox::stateChanged,
        this, &SettingsWindow::startMinimizedChanged
    );
    // connect(
    //     resizeForWhatsappCheckbox, &QCheckBox::stateChanged,
    //     this, &SettingsWindow::resizeForWhatsappChanged
    // );
    connect(
        changeDirectoryButton, &QAbstractButton::clicked,
        this, &SettingsWindow::onChangeDirectoryButton
    );
#ifdef WIN32
    connect(
        launchOnStartupCheckbox, &QCheckBox::stateChanged,
        this, &SettingsWindow::launchOnStartupChanged
    );
#endif

    connect(
        (Application *)Application::instance(), &Application::failedToRegisterHotkey,
        this, &SettingsWindow::failedToRegisterHotkey
    );

    connect(
        (Application *)Application::instance(), &Application::successfullyRegisteredHotkey,
        this, &SettingsWindow::successfullyRegisteredHotkey
    );
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
    auto hotkey = settings.value("hotkey_sequence", "meta+x").toString();
    auto libraryPath = settings.value("library_path", Application::defaultLibraryDirectory()).toString();

    hotkeyLabel = new QLabel("Hotkey: " + hotkey);
    hotkeyChangeButton = new QPushButton("Change hotkey");
    hotkeyChangeButton->setAutoDefault(false);

    hotkeyChangeCancelButton = new QPushButton("Cancel");
    hotkeyChangeCancelButton->setAutoDefault(false);
    hotkeyChangeCancelButton->hide();

    startMinimizedCheckbox = new QCheckBox("Launch EMOJIGUN minimized");

   // resizeForWhatsappCheckbox = new QCheckBox("Resize images to fit better in whatsapp");

    startMinimizedCheckbox->setChecked(settings.value("start_minimized").toInt());
    // resizeForWhatsappCheckbox->setChecked(settings.value("resize_for_whatsapp").toInt());
    // resizeOutputGroup = new QGroupBox("Resize output images");
    
    libraryDirectoryLabel = new QLabel("Library path: " + libraryPath);
    changeDirectoryButton = new QPushButton("Change image library directory");
    
    auto dragImagesLabel = new QLabel("- Drag images/archives on this window to add to library -");

    auto hotkeyLayout = new QVBoxLayout();
    hotkeyLayout->addWidget(hotkeyLabel);

    auto hotkeyBtnLayout = new QHBoxLayout();
    hotkeyBtnLayout->addWidget(hotkeyChangeButton);
    hotkeyBtnLayout->addWidget(hotkeyChangeCancelButton);

    hotkeyLayout->addLayout(hotkeyBtnLayout);

    auto mainLayout = new QVBoxLayout();

    mainLayout->addLayout(hotkeyLayout);
#ifdef WIN32
    launchOnStartupCheckbox = new QCheckBox("Launch EMOJIGUN on system startup");
    launchOnStartupCheckbox->setChecked(settings.value("launch_on_startup").toInt());
    mainLayout->addWidget(launchOnStartupCheckbox);
#endif
    // mainLayout->addWidget(resizeForWhatsappCheckbox);
    mainLayout->addWidget(libraryDirectoryLabel);
    mainLayout->addWidget(changeDirectoryButton);
    mainLayout->addWidget(startMinimizedCheckbox);
    mainLayout->addWidget(dragImagesLabel);
    // mainLayout->addWidget(resizeOutputGroup);

    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);
}

void SettingsWindow::onHotkeyChangeButton() {
    if (!changingHotkey) {
        hotkeyLabel->setText("Press the button combination you want to bind...");
        hotkeyChangeButton->setText("Save");
        hotkeyChangeCancelButton->show();
        hotkeyChangeButton->setEnabled(false);

        changingHotkey = true;
    } else {
        emit hotkeyBindingChange(hotkeyAccumulator);
    }
}

void SettingsWindow::onHotkeyChangeCancelButton() {
    auto hotkey = settings.value("hotkey_sequence", "meta+x").toString();

    changingHotkey = false;

    hotkeyLabel->setText("Hotkey: " + hotkey);
    hotkeyChangeButton->setText("Change hotkey");

    hotkeyAccumulator = "";
    hotkeyChangeCancelButton->hide();
    hotkeyChangeButton->setEnabled(true);
}

QString SettingsWindow::translateKey(int key) {
    if (key == Qt::Key_Shift) {
        return "shift";
    } else if (key == Qt::Key_Control) {
        return "ctrl";
    } else if (key == Qt::Key_AltGr || key == Qt::Key_Alt) {
        return "alt";
    } else if (key == Qt::Key_Meta || key == Qt::Key_Super_L || key == Qt::Key_Super_R) {
        return "meta";
    } else {
        return QKeySequence(key).toString().toLower();
    }
}

void SettingsWindow::keyReleaseEvent(QKeyEvent *event) {
    QDialog::keyReleaseEvent(event);

    if (!changingHotkey) {
        return;
    }

    auto keys = hotkeyAccumulator.split("+");

    bool onlyKeyIsFKey = keys.length() == 1 && event->key() >= Qt::Key_F1 && event->key() <= Qt::Key_F12;

    if (onlyKeyIsFKey) {
        return;
    }

    if (keys.length() > 1) {
        QStringList modifiers = {"shift", "ctrl", "alt", "meta"};

        bool hasAtleastOneModifier = false;
        bool allModifiers = true;

        for (int i = 0; i < keys.size(); ++i) {
            if (modifiers.contains(keys.at(i))) {
                hasAtleastOneModifier = true;
            } else {
                allModifiers = false;
            }
        }

        // valid hotkey reached, don't reset the keys.
        if (hasAtleastOneModifier && !allModifiers) {
            return;
        }
    }

    auto keyName = translateKey(event->key());

    keys.removeAll(keyName);
    hotkeyAccumulator = keys.join("+");

    hotkeyLabel->setText("Hotkey (recording): " + hotkeyAccumulator);
}

void SettingsWindow::keyPressEvent(QKeyEvent *event) {
    QDialog::keyPressEvent(event);

    if (!changingHotkey) {
        return;
    }

    auto keyName = translateKey(event->key());
    auto keys = hotkeyAccumulator.split("+");

    // no duplicates
    if (keys.contains(keyName)) {
        return;
    }

    hotkeyAccumulator += (hotkeyAccumulator.length() ? "+" : "") + keyName;
    hotkeyLabel->setText("Hotkey (recording): " + hotkeyAccumulator);
    
    keys = hotkeyAccumulator.split("+");

    bool onlyKeyIsFKey = keys.length() == 1 && event->key() >= Qt::Key_F1 && event->key() <= Qt::Key_F12;

    if (keys.length() >= 2|| onlyKeyIsFKey) {
        auto oldHotkey = settings.value("hotkey_sequence", "meta+x").toString();

        if (oldHotkey == hotkeyAccumulator) {
            hotkeyChangeButton->setEnabled(false);
            return;
        }

        auto testHotkey = new QHotkey(QKeySequence(hotkeyAccumulator), true);
        hotkeyChangeButton->setEnabled(testHotkey->isRegistered());
        delete testHotkey;
    }
}

void SettingsWindow::resizeForWhatsappChanged(int state) {
    settings.setValue("resize_for_whatsapp", state);
}

void SettingsWindow::startMinimizedChanged(int state) {
    settings.setValue("start_minimized", state);
}

void SettingsWindow::launchOnStartupChanged(int state) {
    settings.setValue("launch_on_startup", state);
}

void SettingsWindow::failedToRegisterHotkey(QString hotkey) {
    QString title =  "Failed to bind hotkey!";
    QString error = "The key combination " + hotkey + " could not be bound. Please try a different combination.";

    QMessageBox::critical(this, title, error);

    onHotkeyChangeCancelButton();
}

void SettingsWindow::successfullyRegisteredHotkey(QString hotkey) {
    if (!changingHotkey) {
        return;
    }

    settings.setValue("hotkey_sequence", hotkey);
    onHotkeyChangeCancelButton();
}

void SettingsWindow::onChangeDirectoryButton() {
    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    auto loc = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    dialog.setDirectory(loc);

    if (dialog.exec()) {
        auto newDir = dialog.selectedFiles().first();
        settings.setValue("library_path", newDir);
    }
}
