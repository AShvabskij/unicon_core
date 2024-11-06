#include "oschistoryservice.h"

const int MAX_DAYS_COUNT = 365;
OscHistoryService::OscHistoryService(IOscDataService *dataSrv, IOscFileStorageService *dataSaver)
{
    m_dataSrv = dataSrv;
    m_dataSaver = dataSaver;
}

long OscHistoryService::requestData(const DDE_OSC_HEADER& hdr)
{
    long res = m_dataSrv->load(hdr);
    return res;
}

long OscHistoryService::requestHeader(const DevID& devID, QDate dateDate, int step, DDE_OSC_HEADER& header)
{
    int s = 0;
    QDate startDate = dateDate.isValid() ? dateDate : QDateTime::currentDateTime().date();

    int days = 0;
    long res = _return_FAIL;

    while (res != _return_OK && days < MAX_DAYS_COUNT) {
        QDate date = startDate.addDays(-days);
        header.device_id = devID.id;
        auto headers = m_dataSaver->headerList(date); // todo: optimization is needed
        for (const DDE_OSC_HEADER& h: headers) {
            if (h.device_id == devID.id ) // and what about sysType ? todo: May be we need to store systype in the header
            {
                if (s == step) {
                    header = h;
                    res = _return_OK;
                    break;
                }
                s++;
            }
        }

        days++;
        continue;
    }

    return res;
}

void OscHistoryService::reset()
{
    m_dataSrv->removeAll();
}
