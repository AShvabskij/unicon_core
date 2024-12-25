#include <QCoreApplication>
#include "core.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Core core;
    core.init();

#ifdef __WIN32__
    core.start(SysType::FILE_IO);
#else
    core.start(SysType::UAVCAN);

#endif

    return a.exec();
}
