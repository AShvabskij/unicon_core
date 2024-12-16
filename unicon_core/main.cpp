#include <QCoreApplication>
#include "core.h"

#ifndef __WIN32__
#include "datausbcopier.h"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Core core;
    core.init();

#ifdef __WIN32__
    core.start(SysType::FILE_IO);
#else
    core.start(SysType::UAVCAN);

    QString sourceDirectory = "/path/to/source/directory";
    QString usbMountDirectory = "/media";

    DataUsbCopier copier(sourceDirectory, usbMountDirectory);
#endif

    return a.exec();
}
