#include "basereqhandler.h"

BaseReqHandler::BaseReqHandler(IDDE_Dispatcher *dde, SysType sysType)
{
    m_dde = dde;
    m_sysType = sysType;
}

int BaseReqHandler::handle(const QJsonObject &request)
{
    if (m_next != nullptr) {
        m_next->handle(request);
    }

    return 1;
}

bool BaseReqHandler::canHandle(const QJsonObject &request)
{
    if (sysTypeId(request) != m_sysType) {
        return false;
    }

    int requestId = request.value("request_id").toInt();

    if (requestId <= 0) {
        return false;
    }

    return true;
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
    const int TIME_STAMP_MAX = 1000*60*60*24; // Milisec per round the clock
    int requestId = request.value("request_id").toInt();

    int res;
    if (request.contains("sys_type_id")) {
        res = request.value("sys_type_id").toInt();
    } else {
        res = requestId / TIME_STAMP_MAX;
    }

    SysType sysType = (SysType)res;

    if (sysType <= SysType::Undefined || sysType >= SysType::Unknown) {
        QTextStream(stdout) << "undefined system type in request id = " << requestId << "\n";

        return SysType::Undefined;
    }

    return sysType;
}
