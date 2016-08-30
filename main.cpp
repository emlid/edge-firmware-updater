#include <QApplication>
#include <QThread>
#include "libusb-1.0/libusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef Q_OS_LINUX
#include <dd.h>
#include <handledevice_linux.h>
#elif defined Q_OS_WIN32
#include <dd_win.h>
#include <handledevice_win.h>
#endif

#include <rpiboot.h>
#include <flashing_parameters.h>
#include <mainwindow.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <iostream>
using namespace std;


int parseCommandLineArguments(const QStringList & arguments, struct FlashingParameters* param){
    bool parseResult;
    QCommandLineParser parser;
    parser.addHelpOption();

    QCommandLineOption pathToImage(QStringList() << "I" << "image", QCoreApplication::translate("main", "Path to image") \
                                   ,QCoreApplication::translate("main", "path"));
    parser.addOption(pathToImage);

    parseResult = parser.parse(arguments);
    if (!parseResult) {
        cerr << qPrintable(parser.errorText()) << endl;
        cout << qPrintable(parser.helpText()) << endl;
        return 1;
    }

    bool gotPath = parser.isSet(pathToImage);

    if (!gotPath) {
        cerr << "Error: an argument is required." << endl;
        cout << qPrintable(parser.helpText()) << endl;
        return 1;
    }

    QString path = parser.value(pathToImage);
    QRegExp rx("*.img");
    rx.setPatternSyntax(QRegExp::Wildcard);

    if (!rx.exactMatch(path)) {
        cerr  << qPrintable(path) << endl;
        cerr << "Error: invalid path or file type. Should be /path/to/image.img" << endl;
        return 1;
    }

    QFile file(path);

    if( !file.exists() )
    {
        cerr << "Error: file does not exist" << endl;
        return 1;
    }

    param->inputFile.append(path);
    return 0;
}



int main(int argc, char *argv[])
{
    int ret = 0;
    struct FlashingParameters flashParams;

    QApplication a(argc, argv);

    MainWindow mainwindow;
    mainwindow.show();
    a.exec();

    ret = parseCommandLineArguments(a.arguments(), &flashParams);
    if (ret) {
        cerr << "Failed to parse arguments" << endl;
        exit(1);
    }

    ret = getDeviceNodePath(&flashParams.outputFile);
    if (ret){
        cerr << "Failed to get device node path" << endl;
        exit(1);
    }

    ret = flashDevice(flashParams);

    if (ret) {
        cerr << "Failed to flash device" << endl;
        exit(1);
    }

    a.quit();
}


