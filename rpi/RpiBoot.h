#ifndef IRPIBOOT_H
#define IRPIBOOT_H

namespace usb {
    class IRpiBoot;
}

class usb::IRpiBoot {
public:
    int bootAsMassStorage(void) { return bootAsMassStorage_core(); }
    virtual ~IRpiBoot(void) = default;

private:
    virtual int bootAsMassStorage_core(void) = 0;
};



#endif // IRPIBOOT_H
