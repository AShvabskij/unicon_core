#ifndef OSCHISTORYSERVICE_H
#define OSCHISTORYSERVICE_H

#include "DDE_INTERFACES.h"
#include "DDE_OSC_TYPES.h"

#include "osc_types.h"

#include <QTimer>
#include <QJsonObject>
#include <QMap>

class OscHistoryService: public QObject
{
     Q_OBJECT
public:
    OscHistoryService(IOscDataService* dataSrv, IOscFileStorageService* dataSaver);

    long requestData(const DDE_OSC_HEADER &hdr);
    long requestHeader(const DevID &devID, QDate dateDate, int step, DDE_OSC_HEADER& header);
    void reset();

    IOscDataService* getDataSrv() {
        return m_dataSrv;
    }

private:
    IDDE* m_dde;
    IOscDataService* m_dataSrv;
    IOscFileStorageService* m_dataSaver = nullptr;

    DDE_OSC_HEADER m_header;

signals:
    void historyReceived(quint16 ind);

};

#endif // OSCHISTORYSERVICE_H
