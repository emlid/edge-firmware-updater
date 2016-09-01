#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QStandardPaths>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->alignToCenter();
}

void MainWindow::alignToCenter()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image"),QStandardPaths::displayName(QStandardPaths::DesktopLocation), tr("Image Files (*.img);;All files (*.*)"));
    if (!fileName.isEmpty()) {
        ui->leFileName->setText(fileName);
    }
}
