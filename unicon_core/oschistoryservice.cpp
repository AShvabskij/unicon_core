#include "oschistoryservice.h"

const int MAX_DAYS_COUNT = 365;
OscHistoryService::OscHistoryService(IOscDataService *dataSrv, IOscDataStorageService *dataSaver)
{
    m_dataSrv = dataSrv;
    m_dataSaver = dataSaver;
}

long OscHistoryService::requestData(const DDE_OSC_HEADER& hdr)
{
    long res = _return_OK;

    OscType::OscDataBuffer* dat = m_dataSrv->get(hdr);

    if (dat == nullptr) {
        dat = m_dataSrv->createDataBuffer(hdr);
        res = m_dataSaver->loadData(hdr, *dat);
        if (res != _return_OK) {
            delete dat;
            return res;
        }
    }

    Q_ASSERT(dat);
    if (dat == nullptr) {
        return _return_FAIL;
    }

    dat->eof = true; // ??
    dat->resetPos();
    dat->timestamp = dat->valueCount  * dat->resolution_us;

    res = m_dataSrv->appendBuffer(std::move(*dat));

    if (res == _return_OK) {
        emit historyReceived(hdr.device_id);
    }

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

void OscHistoryService::reset()
{
    m_dataSrv->resetAll();
}
