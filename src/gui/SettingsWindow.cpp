#include <QHotkey>
#include <QStandardPaths>
#include "SettingsWindow.h"
#include "../util/config/Config.h"
#include "../util/config/ConfigManager.h"
#include "../app/Application.h"
#include <iostream>

SettingsWindow::SettingsWindow(MainWindow *window) {
    this->window = window;

    createActions();
    createUI();
    createTrayIcon();
    connectUI();

    QIcon icon(":/assets/eyes-32x25.png");

    setWindowTitle("EMOJIGUN");
    setWindowIcon(icon);
    window->setWindowIcon(icon);
    trayIcon->setIcon(icon);

    setAcceptDrops(true);
    
    setWindowFlags(
        windowFlags() 
        // remove the ? icon
        & ~Qt::WindowContextHelpButtonHint 
        // disable resizing
        | Qt::MSWindowsFixedSizeDialogHint
    );

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

    connect(
        launchOnStartupCheckbox, &QCheckBox::stateChanged,
        (Application *)Application::instance(), &Application::launchOnStartupChanged
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

    connect(
        resizeOutputImageCheckbox, &QCheckBox::stateChanged,
        this, &SettingsWindow::resizeOutputChanged
    );

    connect(
        outputImageWidthLineEdit, &QLineEdit::textChanged,
        this, &SettingsWindow::outputSizeChanged
    );
    connect(
        outputImageHeightLineEdit, &QLineEdit::textChanged,
        this, &SettingsWindow::outputSizeChanged
    );

    connect(
        applyOutputImageResizeSettingsButton, &QAbstractButton::clicked,
        this, &SettingsWindow::onApplyOutputImageResizeSettingsButton
    );

    connect (
        checkForUpdatesOnStartupCheckbox, &QCheckBox::stateChanged,
        this, &SettingsWindow::checkForUpdatesOnStartupChanged
    );

    connect(dragDropLayout, &DragDropLayout::expired, this, [&]() { hideDragDropLayout(); });
}

void SettingsWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            show();
            break;
        case QSystemTrayIcon::MiddleClick:
            window->display();
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
    auto hasClosedOnce = settings.value("closed_settings_once", false).toBool();

    if (trayIcon->isVisible() && !hasClosedOnce) {
        QMessageBox::information(this, tr("Systray"),
                                 tr("EMOJIGUN will keep running in the "
                                    "system tray. To terminate EMOJIGUN, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
    }

    hide();
    settings.setValue("closed_settings_once", true);
    event->ignore();
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
    auto hotkey = settings.value("hotkey_sequence", "ctrl+shift+x").toString();
    auto libraryPath = settings.value("library_path", Application::defaultLibraryDirectory()).toString();

    hotkeyLabel = new QLabel("Hotkey: " + hotkey);
    hotkeyChangeButton = new QPushButton("Change hotkey");
    hotkeyChangeButton->setAutoDefault(false);

    hotkeyChangeCancelButton = new QPushButton("Cancel");
    hotkeyChangeCancelButton->setAutoDefault(false);
    hotkeyChangeCancelButton->hide();

    startMinimizedCheckbox = new QCheckBox("Launch minimized");

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

    auto resizeOutputGroup = new QGridLayout();

    resizeOutputImageCheckbox = new QCheckBox("Limit output image size");

    auto resizeOutputEnabled = settings.value("resize_output_image", true).toBool();
    auto defaultResize = resizeOutputEnabled ? "32" : "";
    auto savedWidth = settings.value("resize_output_image_width", defaultResize).toString();
    auto savedHeight = settings.value("resize_output_image_height", defaultResize).toString();

    resizeOutputImageCheckbox->setChecked(resizeOutputEnabled);

    QRegExpValidator* rxv = new QRegExpValidator(QRegExp("\\d*"), this); 

    outputWidthLabel = new QLabel("Max width:");
    outputHeightLabel = new QLabel("Max height:");

    outputImageWidthLineEdit = new QLineEdit();
    outputImageHeightLineEdit = new QLineEdit();

    outputImageWidthLineEdit->setText(savedWidth);
    outputImageHeightLineEdit->setText(savedHeight);
    outputImageWidthLineEdit->setMaximumWidth(45);
    outputImageHeightLineEdit->setMaximumWidth(45);

    outputImageWidthLineEdit->setValidator(rxv);
    outputImageHeightLineEdit->setValidator(rxv);

    applyOutputImageResizeSettingsButton = new QPushButton("Apply");
    applyOutputImageResizeSettingsButton->setEnabled(false);

    if (!resizeOutputEnabled) {
        outputWidthLabel->hide();
        outputHeightLabel->hide();

        outputImageWidthLineEdit->hide();
        outputImageHeightLineEdit->hide();

        applyOutputImageResizeSettingsButton->hide();
    }

    resizeOutputGroup->addWidget(outputWidthLabel, 1, 0);
    resizeOutputGroup->addWidget(outputImageWidthLineEdit, 1, 1, Qt::AlignLeft);

    resizeOutputGroup->addWidget(outputHeightLabel, 1, 2);
    resizeOutputGroup->addWidget(outputImageHeightLineEdit, 1, 3, Qt::AlignLeft);

    resizeOutputGroup->addWidget(applyOutputImageResizeSettingsButton, 3, 0, 1, 4);

    checkForUpdatesOnStartupCheckbox = new QCheckBox("Check for updates when launched");
    auto launchOnStartup = settings.value("check_for_updates_on_launch", true).toBool();
    checkForUpdatesOnStartupCheckbox->setChecked(launchOnStartup);

    settingsWidget = new QWidget();
    settingsLayout = new QVBoxLayout(settingsWidget);
    settingsLayout->setMargin(0);

    dragDropWidget = new QWidget();
    dragDropLayout = new DragDropLayout(dragDropWidget);
    dragDropLayout->setMargin(0);
    dragDropWidget->setVisible(false);

    settingsLayout->addLayout(hotkeyLayout);
#ifdef WIN32
    launchOnStartupCheckbox = new QCheckBox("Launch on system startup");
    launchOnStartupCheckbox->setChecked(settings.value("launch_on_startup").toInt());
    settingsLayout->addWidget(launchOnStartupCheckbox);
#endif
    // mainLayout->addWidget(resizeForWhatsappCheckbox);
    settingsLayout->addWidget(libraryDirectoryLabel);
    settingsLayout->addWidget(changeDirectoryButton);
    settingsLayout->addWidget(resizeOutputImageCheckbox);
    settingsLayout->addLayout(resizeOutputGroup);
    settingsLayout->addWidget(startMinimizedCheckbox);
    settingsLayout->addWidget(checkForUpdatesOnStartupCheckbox);
    // mainLayout->addWidget(dragImagesLabel);
    // mainLayout->addWidget(resizeOutputGroup);

    settingsLayout->setSizeConstraint(QLayout::SetFixedSize);

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(settingsWidget);
    mainLayout->addWidget(dragDropWidget);

    dragDropWidget->setMaximumWidth(settingsWidget->width());

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
    auto hotkey = settings.value("hotkey_sequence", "ctrl+shift+x").toString();

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
    if (event->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(event);
    } else if (!changingHotkey) {
        close();
    }

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
        auto oldHotkey = settings.value("hotkey_sequence", "ctrl+shift+x").toString();

        if (oldHotkey == hotkeyAccumulator) {
            hotkeyChangeButton->setEnabled(true);
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
        libraryDirectoryLabel->setText("Library path: " + newDir);
    }
}

void SettingsWindow::resizeOutputChanged(int state) {
    if (state) {
        outputWidthLabel->show();
        outputHeightLabel->show();

        outputImageWidthLineEdit->show();
        outputImageHeightLineEdit->show();
        outputImageWidthLineEdit->setText("");
        outputImageHeightLineEdit->setText("");

        applyOutputImageResizeSettingsButton->show();
    } else {
        outputWidthLabel->hide();
        outputHeightLabel->hide();

        outputImageWidthLineEdit->hide();
        outputImageHeightLineEdit->hide();

        applyOutputImageResizeSettingsButton->hide();
        settings.setValue("resize_output_image", false);
    }
}

void SettingsWindow::outputSizeChanged(const QString &text) {
    auto width = outputImageWidthLineEdit->text().toInt();
    auto height = outputImageHeightLineEdit->text().toInt();

    applyOutputImageResizeSettingsButton->setEnabled(width > 0 && height > 0);
}

void SettingsWindow::onApplyOutputImageResizeSettingsButton() {
    auto width = outputImageWidthLineEdit->text().toInt();
    auto height = outputImageHeightLineEdit->text().toInt();

    settings.setValue("resize_output_image", true);
    settings.setValue("resize_output_image_width", width);
    settings.setValue("resize_output_image_height", height);

    applyOutputImageResizeSettingsButton->setEnabled(false);

    QMessageBox::information(this, "Success", "Resize settings saved successfully!");

    ConfigManager::applyConfigFromQSettings();
}

void SettingsWindow::checkForUpdatesOnStartupChanged(int state) {
    settings.setValue("check_for_updates_on_launch", state);
}

void SettingsWindow::dragEnterEvent(QDragEnterEvent *event) {
    QDialog::dragEnterEvent(event);

    if (!event->mimeData()->hasUrls()) {
        return; 
    }

    settingsWidget->setVisible(false);
    dragDropWidget->setVisible(true);

    event->acceptProposedAction();
}

void SettingsWindow::dragLeaveEvent(QDragLeaveEvent *event) {
    QDialog::dragLeaveEvent(event);
    hideDragDropLayout();
}

void SettingsWindow::hideDragDropLayout() {
    setAcceptDrops(true);

    settingsWidget->setVisible(true);
    dragDropWidget->setVisible(false);
}

void SettingsWindow::dropEvent(QDropEvent *event) {
    this->setAcceptDrops(false);
    QDialog::dropEvent(event);

    QList<QUrl> filesToImport;

    foreach(auto url, event->mimeData()->urls()) {
        if (url.fileName().toLower().endsWith(".zip")) {
            filesToImport.append(url);
            continue;
        }

        for (const auto &ext : Config::getImageExtensions()) {
            if (url.fileName().toLower().endsWith(QString(".") + ext.c_str())) {
                filesToImport.append(url);
                continue;
            }
        }
    }

    if (filesToImport.empty()) {
        dragDropLayout->noSuitableFilesDropped();
        return;
    }

    dragDropLayout->importStarted();

    auto libraryPath = settings.value("library_path", Application::defaultLibraryDirectory()).toString();

    importer = new EmojiImporter(libraryPath, filesToImport);

    connect(importer, &EmojiImporter::imported, dragDropLayout, &DragDropLayout::importFinished);
    connect(importer, &EmojiImporter::failed, dragDropLayout, &DragDropLayout::importFailed);

    importer->start();
}

void SettingsWindow::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    setFixedSize(width(), height());
}

