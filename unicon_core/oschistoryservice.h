#ifndef OSCHISTORYSERVICE_H
#define OSCHISTORYSERVICE_H

#include "DDE_INTERFACES.h"
#include "DDE_OSC_TYPES.h"

#include "osc_types.h"

#include <QTimer>
#include <QJsonObject>
#include <QMap>

class OscHistoryService
{
public:
    OscHistoryService(IOscDataService* dataSrv, IOscDataStorageService* dataSaver);

    long loadData(const DDE_OSC_HEADER &hdr);
    long getHeader(const DevID &devID, QDate dateDate, int step, DDE_OSC_HEADER& header);

private:
    IDDE* m_dde;
    IOscDataService* m_dataSrv;
    IOscDataStorageService* m_dataSaver = nullptr;

    DDE_OSC_HEADER m_header;

};

#endif // OSCHISTORYSERVICE_H
