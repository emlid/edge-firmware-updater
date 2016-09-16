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
    bool fileSelected();
    bool deviceSelected();
    bool flashingInProgress();

private slots:
    void setCancelStartButtonState();
    void setRefreshButtonState();
    void appendStatusLog(const QString& text, bool critical = 0);
    void updateProgressBar(uint32_t curr, uint32_t total) {ui->progressBar->setValue(100 * (float)curr / (float)total);}
    void on_refreshButton_clicked();
    void on_browseButton_clicked();
    void on_startButton_clicked();
    void on_cancelButton_clicked();
    void on_leFileName_textChanged();
    void on_lwDeviceList_itemSelectionChanged();
    void on_logButton_clicked();
};

#endif // MAINWINDOW_H
