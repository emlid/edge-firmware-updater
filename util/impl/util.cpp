#include "../util.h"
#include "CRCServiceImpl.h"
#include "FlashingServiceImpl.h"


auto util::makeFlashingService(void)
    -> std::unique_ptr<util::IFlashingService>
{
    return std::make_unique<util::FlashingServiceImpl>();
}


auto util::makeCRCService(void)
    -> std::unique_ptr<util::ICRCService>
{
    return std::make_unique<util::CRCServiceImpl>();
}

