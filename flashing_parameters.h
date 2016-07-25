#include <QString>
#include <QList>

#ifndef RPIDEVICEFLASHING_H
#define RPIDEVICEFLASHING_H

#define VENDOR_ID       0xa5c
#define VENDOR_ID_CHAR   "0a5c"
#define PRODUCT_ID1     0x2763
#define PRODUCT_ID2     0x2764

struct FlashingParameters{
    QString blockSize;
    QString inputFile;
    QString outputFile;

    FlashingParameters(): blockSize("bs=1000000"), inputFile("if="), outputFile("of=") {}
};


#endif

