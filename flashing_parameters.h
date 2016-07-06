#ifndef FLASHINGPARAMETERS_H
#define FLASHINGPARAMETERS_H

#include <QString>

struct FlashingParameters{
    QString blockSize;
    QString inputFile;
    QString outputFile;

    FlashingParameters(): blockSize("bs=1000000"), inputFile("if="), outputFile("of=") {}
};

#endif

