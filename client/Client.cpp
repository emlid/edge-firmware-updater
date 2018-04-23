#include "Client.h"
#include "impl/UpdaterConnectionImpl.h"
#include "impl/UpdaterProcess.h"


static auto makeProcess(void)
    -> std::unique_ptr<client::UpdaterProcess>
{
    return std::unique_ptr<client::UpdaterProcess>(new client::UpdaterProcess);
}


auto client::makeConnection(void)
    -> std::unique_ptr<UpdaterConnection>
{
    return std::unique_ptr<UpdaterConnection>(
        new UpdaterConnectionImpl(makeProcess)
    );
}
