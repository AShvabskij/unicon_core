#ifndef BASEREQHANDLER_H
#define BASEREQHANDLER_H

#include "ireqhandler.h"
#include "responsemanager.h"
#include "../DDE_CAN_LIB/DDE/DDE.h"

class BaseReqHandler : public IReqHandler
{
    Q_OBJECT
public:
    BaseReqHandler();
    ~BaseReqHandler() {};

    virtual int handle(const QJsonObject& request);
    virtual void setNext(IReqHandler* next);
    void setResponseManager(ResponseManager* response);

protected:
    IReqHandler* m_next = nullptr;
    ResponseManager* m_response = nullptr;
    IDDE* m_dde = nullptr;
};

#endif // BASEREQHANDLER_H
