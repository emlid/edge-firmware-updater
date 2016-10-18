#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QProgressBar>
#include <firmwareupgradecontroller.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void show();

signals:
    void windowShown();

private:
    Ui::MainWindow *ui;
    FirmwareUpgradeController* _upgradeController;

    void connectGuiSignalsToSlots();
    void alignToCenter();
    void updateList();
    QProgressBar* createProgressBarForDevice(StorageDevice * device);
    void selectFirstDeviceIfNothingElseConnected();
    bool fileSelected();
    bool deviceSelected();
    bool flashingInProgress();

private slots:
    void setupDeviceListWidget();
    void setCancelStartButtonState();
    void setRefreshButtonState();
    void appendStatusLog(const QString& text, bool critical = 0);
    void updateProgressBar(int newValue, int progressBarIndex);
    void onRefreshButtonClicked();
    void onBrowseButtonClicked();
    void onStartButtonClicked();
    void onCancelButtonClicked();
    void onFileNameTextChanged();
    void onDeviceListItemSelectionChanged();
    void onLogButtonClicked();
};

#endif // MAINWINDOW_H
