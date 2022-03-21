#include "basereqhandler.h"

BaseReqHandler::BaseReqHandler(IDDE *dde)
{
    m_dde = dde;
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
