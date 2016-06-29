#ifndef RPIDEVICEFLASHING_H
#define RPIDEVICEFLASHING_H


class FlashingParameters{
public:
    FlashingParameters();

    void setBs(char *);
    void setIf(char *);
    void setOf(char *);

    char *getBs();
    char *getOf();
    char *getIf();
    void getAllParams(char **);

private:
    char _blockSize[11];
    char _inputFile[200];
    char _outputFile[100];
};

#endif

