#ifndef OSC_DATA_H
#define OSC_DATA_H

#include "DDE_INTERFACES.h"
#include "DDE_OSC_TYPES.h"

#include "osc_types.h"

#include <QTimer>
#include <QJsonObject>
#include <QMap>

class OscStateMachine;
class OscStateService
{
public:
    OscStateService(IDDE* dde, IOscDataService* dataSrv);
    void init(QList<DevInd> devList);
    void update();

private:
    IDDE* m_dde;
    QMap<quint16, OscStateMachine*> m_oscState;
    IOscDataService* m_dataSrv;
    QList<DevInd> m_devIdList;
};

class OscStateMachine
{
    enum STATE {
        Normal,
        Idle,
        Getting,
        Busy,
        Eof,
        Saving,
        Updated,
        Finished,
        Error
    };

public:
    OscStateMachine(IDDE *dde, IOscDataService *dataSrv);
    void update(DevInd devId);
    void init(DevInd devId);

private:

    long getData(const DDE_OSC_HEADER& hdr, DDE_GET_OSC_DATA &getDat);

    IDDE* m_dde = nullptr;
    STATE m_state = Normal;
    DDE_OSC_HEADER m_header;
    DDE_GET_OSC_DATA m_ddeData;
    bool m_sof = false;
    bool m_eof = false;
    int m_errCounter = 0;
    int m_busyCounter = 0;
    int m_idleCounter = 0;

    IOscDataService* m_dataSrv = nullptr;
};

#endif //OSC_DATA_H
