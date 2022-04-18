#ifndef APPLICATION_H
#define APPLICATION_H

#include "socketserver.h"

#include "DDE_PARAMS_TYPE.h"

class IDDE;

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

    IDDE* m_dde;
};

#endif // APPLICATION_H
