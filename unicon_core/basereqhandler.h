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
    BaseReqHandler(IDDE_Dispatcher* ddeDispatcher, SysType sysType);
    ~BaseReqHandler() {};

    virtual int handle(const QJsonObject& request);
    virtual bool canHandle(const QJsonObject &request);
    virtual void setNext(IReqHandler* next);
    void setResponseManager(ResponseManager* response);
    virtual void handleClose() {};

protected:
    SysType sysTypeId(const QJsonObject& request);

    IReqHandler* m_next = nullptr;
    ResponseManager* m_response = nullptr;
    IDDE_Dispatcher* m_dde = nullptr;
    SysType m_sysType = SysType::Unknown;

};

#endif // BASEREQHANDLER_H
