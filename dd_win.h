#ifndef DD_WIN_H
#define DD_WIN_H

enum Status {
    STATUS_IDLE=0,
    STATUS_READING,
    STATUS_WRITING,
    STATUS_EXIT,
    STATUS_CANCELED
};

int flashDevice(struct FlashingParameters);

#endif // DD_WIN_H
