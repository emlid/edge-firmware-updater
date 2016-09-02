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

private slots:
    void on_refreshButton_clicked();
    void on_browseButton_clicked();
};

#endif // MAINWINDOW_H
