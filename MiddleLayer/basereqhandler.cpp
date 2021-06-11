#include "basereqhandler.h"

BaseReqHandler::BaseReqHandler()
{
    m_dde = new DDE_CAN();
    m_dde->init(0); //run thread
}

int BaseReqHandler::handle(const QJsonObject &request)
{
    if (m_next != nullptr) {
        m_next->handle(request);
    }

    return 0;
}

void BaseReqHandler::setNext(IReqHandler *next)
{
    m_next = next;
}

void BaseReqHandler::setResponseManager(ResponseManager* response)
{
    m_response = response;
}
