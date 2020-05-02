#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QtWidgets>
#include <QHotkey>
#include <memory>
#include "MainWindow.h"
#include "../util/EmojiImporter.h"
#include "DragDropLayout.h"

class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit SettingsWindow(MainWindow* window);
    void setVisible(bool visible) override;
    bool isChangingHotkey() { return changingHotkey; }

signals:
    void hotkeyBindingChange(QString newBinding);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onHotkeyChangeButton();
    void onHotkeyChangeCancelButton();
    void onApplyOutputImageResizeSettingsButton();

    void resizeForWhatsappChanged(int state);
    void startMinimizedChanged(int state);
    void launchOnStartupChanged(int state);
    void resizeOutputChanged(int state);

    void outputSizeChanged(const QString &text);

    void onChangeDirectoryButton();

    void checkForUpdatesOnStartupChanged(int state);

    void failedToRegisterHotkey(QString hotkey);
    void successfullyRegisteredHotkey(QString hotkey);

private:
    QVBoxLayout *settingsLayout;
    DragDropLayout *dragDropLayout;
    QWidget *settingsWidget;
    QWidget *dragDropWidget;

    void createTrayIcon();
    void createActions();
    void createUI();
    void connectUI();

    void hideDragDropLayout();

    std::unique_ptr<EmojiImporter> importer;

    QString translateKey(int key);

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
    QPushButton* hotkeyChangeCancelButton;

    QCheckBox* launchOnStartupCheckbox;
    QCheckBox* startMinimizedCheckbox;

    QCheckBox* resizeForWhatsappCheckbox;

    QLabel* outputWidthLabel;
    QLabel* outputHeightLabel;
    QCheckBox* resizeOutputImageCheckbox;
    QLineEdit* outputImageWidthLineEdit;
    QLineEdit* outputImageHeightLineEdit;
    QPushButton* applyOutputImageResizeSettingsButton;

    QLabel* libraryDirectoryLabel;
    QPushButton* changeDirectoryButton;

    QCheckBox* checkForUpdatesOnStartupCheckbox;
};



