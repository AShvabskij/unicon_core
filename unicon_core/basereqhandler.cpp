#include "basereqhandler.h"

BaseReqHandler::BaseReqHandler(IDDE_Dispatcher *dde)
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

SysType BaseReqHandler::sysTypeId(const QJsonObject& request)
{
    SysType sysType = SysType::Undefined;
    int res;
    const int TIME_STAMP_MAX = 1000*60*60*24; // Milisec per round the clock
    int requestId = request.value("request_id").toInt();

    if (request.contains("sys_type")) {
        res = request.value("sys_type").toInt();
    } else {
        res = requestId / TIME_STAMP_MAX;
    }

    if (res <= SysType::Undefined || res >= SysType::Unknown) {
        QTextStream(stdout) << "undefined system type in request id = " << requestId << "\n";

        return SysType::Undefined;
    }

    return (SysType)res;
}
