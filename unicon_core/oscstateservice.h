#ifndef OSC_DATA_H
#define OSC_DATA_H

#include "DDE_TOP.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_DEVICES_TYPE.h"
#include "oscbuffservice.h"

#include <QTimer>
#include <QJsonObject>
#include <QMap>

class OscStateMachine;
class OscStateService
{
public:
    OscStateService(IDDE* dde, IOscBufferService* buffSrv);
    void init(QList<quint16> devList);
    void update();

private:
    IDDE* m_dde;
    QMap<quint16, OscStateMachine*> m_oscState;
    IOscBufferService* m_buffSrv;
    QList<quint16> m_devIdList;
};

class OscStateMachine
{
    enum STATE {
        Normal,
        Getting,
        Eof,
        Saving,
        Finished,
        Error
    };

public:
    OscStateMachine(IDDE *dde, IOscBufferService *buffSrv);
    void update(quint16 devId);

private:

    long retrieveData(const DDE_OSC_HEADER& hdr, DDE_GET_OSC_DATA* getDat);

    IDDE* m_dde = nullptr;
    STATE m_state = Normal;
    DDE_OSC_HEADER* m_header;
    DDE_GET_OSC_DATA* m_ddeData;
    int m_errCounter = 0;

    IOscBufferService* m_buffSrv = nullptr;

};

#endif //OSC_DATA_H
