#ifndef BASEREQHANDLER_H
#define BASEREQHANDLER_H

#include "ireqhandler.h"
#include "responsemanager.h"
#include "DDE_TOP.h"
#include "dde_dispatcher.h"

class BaseReqHandler : public IReqHandler
{
    Q_OBJECT
public:
    BaseReqHandler(IDDE_Dispatcher* ddeDispatcher);
    ~BaseReqHandler() {};

    virtual int handle(const QJsonObject& request);
    virtual void setNext(IReqHandler* next);
    void setResponseManager(ResponseManager* response);

protected:
    SysType sysTypeId(const QJsonObject& request);

    IReqHandler* m_next = nullptr;
    ResponseManager* m_response = nullptr;
    IDDE_Dispatcher* m_dde = nullptr;
};

#endif // BASEREQHANDLER_H
