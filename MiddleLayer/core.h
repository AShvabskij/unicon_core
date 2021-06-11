#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>

#include "socketserver.h"

struct DDE_GET_PARAMS_HEADER;
class DDE_CAN;

class Core
{
public:
    Core();
    ~Core();

    void start();

private:
    SocketServer* m_cmdServer;
    SocketServer* m_streamServer;

// test methods
    int test();
    void print_modules(const DDE_GET_PARAMS_HEADER& p);
    void print_params(int device_ID, int module_ID, const DDE_GET_PARAMS_HEADER& p);

    DDE_CAN* m_dde;
};

#endif // APPLICATION_H
