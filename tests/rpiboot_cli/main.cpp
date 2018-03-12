#include <QCoreApplication>
#include "rpi.h"

constexpr int vid(void) { return 0x0a5c; }
constexpr int pid(void) { return 0x2764; }

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    auto rpiboot = usb::makeRpiBoot(vid(), QList<int>{pid()});

    rpiboot->bootAsMassStorage();

    return a.exec();
}
