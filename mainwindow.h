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
    void setCancelStartButtonState();
    bool fileSelected();
    bool deviceSelected();

private slots:
    void appendStatusLog(const QString& text, bool critical = 0);
    void updateProgressBar(int curr, int total) {ui->progressBar->setValue(100 * (float)curr / (float)total);;}
    void on_refreshButton_clicked();
    void on_browseButton_clicked();
    void on_startButton_clicked();
    void on_cancelButton_clicked();
    void on_leFileName_textChanged();
    void on_lwDeviceList_itemSelectionChanged();
};

#endif // MAINWINDOW_H
