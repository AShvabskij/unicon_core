#ifndef BASEREQHANDLER_H
#define BASEREQHANDLER_H

#include "ireqhandler.h"
#include "responsemanager.h"
#include "../DDE_CAN_LIB/DDE_CAN.h"

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
    IReqHandler* m_next;
    ResponseManager* m_response;
    DDE_CAN* m_dde;
};

#endif // BASEREQHANDLER_H
