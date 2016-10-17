#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
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

    void alignToCenter();
    void updateList();
    QProgressBar* createProgressBarForDevice(StorageDevice * device);
    bool fileSelected();
    bool deviceSelected();
    bool flashingInProgress();

private slots:
    void setupDeviceListWiget();
    void setCancelStartButtonState();
    void setRefreshButtonState();
    void appendStatusLog(const QString& text, bool critical = 0);
    void updateProgressBar(int newValue, int progressBarIndex);
    void on_refreshButton_clicked();
    void on_browseButton_clicked();
    void on_startButton_clicked();
    void on_cancelButton_clicked();
    void on_leFileName_textChanged();
    void on_lwDeviceList_itemSelectionChanged();
    void on_logButton_clicked();
    void clearListAndBarFocus();
};

#endif // MAINWINDOW_H
