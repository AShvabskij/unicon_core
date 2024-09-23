#include "oschistoryservice.h"

const int MAX_DAYS_COUNT = 365;
OscHistoryService::OscHistoryService(IOscDataService *dataSrv, IOscDataStorageService *dataSaver)
{
    m_dataSrv = dataSrv;
    m_dataSaver = dataSaver;
}

long OscHistoryService::loadData(const DDE_OSC_HEADER& hdr)
{
    OscType::OscDataBuffer* dat = m_dataSrv->get(hdr.device_id);

    if (dat != nullptr && dat->trig_time == hdr.settings.trig_time) {
        return _return_OK;
    }

    dat = m_dataSrv->createDataBuffer(hdr);
    long res = m_dataSaver->loadData(hdr, *dat);

    if (res != _return_OK) {
        return res;
    }

    m_dataSrv->reset(hdr.device_id);

    dat->eof = true; // ??
    res = m_dataSrv->appendToHistoryData(hdr, *dat);

    delete dat;

    return res;
}

long OscHistoryService::getHeader(const DevID& devID, QDate dateDate, int step, DDE_OSC_HEADER& header)
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

