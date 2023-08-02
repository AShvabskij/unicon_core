#include "oscstateservice.h"

#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QCoreApplication>

OscStateService::OscStateService(IDDE *dde, IOscBufferService *buffSrv)
{
    m_dde = dde;
    m_buffSrv = buffSrv;
}

void OscStateService::init(QList<DevInd> devList)
{
    for (DevInd devId: devList) {
        if (devId == DDE_DEV0_MASTER_IND)
            continue;

        if (!m_devIdList.contains(devId)) {
            m_oscState[devId] = new OscStateMachine(m_dde, m_buffSrv);
            m_devIdList.append(devId);
        }
    }
}

void OscStateService::update()
{
    for (DevInd id: m_devIdList) {
        Q_ASSERT(m_oscState.keys().contains(id));
        if (!m_oscState.keys().contains(id))
            return;

        m_oscState[id]->update(id);
    }
}

OscStateMachine::OscStateMachine(IDDE* dde, IOscBufferService *buffSrv)
{
    m_dde = dde;
    m_header = new DDE_OSC_HEADER();
    m_ddeData = new DDE_GET_OSC_DATA();
    m_state = STATE::Normal;
    m_buffSrv = buffSrv;
}

void OscStateMachine::update(DevInd devId)
{
    STATE prevState = m_state;
    switch (m_state) {
        case Normal: {
            memset(m_header, 0, sizeof(DDE_OSC_HEADER));
            m_header->device_id = devId;
            auto res = m_dde->get_osc_header(*m_header);
            if (res == _return_FAIL) {
                qWarning() << "Error getting header from osc, id = " << m_header->device_id;
                m_state = Error;
                return;
            }

            if (res != _return_OK) return;

            if (m_header->settings.trig_time > 0 && m_header->settings.reason > 0) {
                m_sof = false;
                m_state = Getting;
            }

            break;
        }

        case Getting: {
            auto res = getData(*m_header, m_ddeData);

            if (res == _return_Busy) {
                m_state = Busy;
                return;
            }

            if (res != _return_OK) {
                qWarning() << "Error getting data from the osc, id = " << m_header->device_id;
                m_state = Error;
                return;
            }

            res = m_buffSrv->appendData(*m_header, *m_ddeData);

            if (res != _return_OK) {
                qWarning() << "Error getting buffer for the osc, id = " << m_header->device_id;
                m_state = Error;
                return;
            }

            if (m_ddeData->sof) {
                m_sof = m_ddeData->sof;
            }

            if (m_ddeData->eof) {
                if (m_sof) {
                    m_state = Saving;
                } else {
                    m_state = Normal;
                }
            }

            break;
        }
        case Busy: {
            m_busyCounter++;
            if (m_busyCounter % 4 == 0) {
                m_busyCounter = 0;
                m_state = Getting;
            }
            break;
        }

        case Saving: {
            m_buffSrv->saveToFile(*m_header);
            m_state = Normal;
          break;
        }
        case Finished: {
            break;
        }
        case Error: {
            m_errCounter++;
            m_state = Normal;
            break;
        }
        default: break;
    }

    if (m_state == STATE::Normal) {
        qDebug() << "OscStateMachine: Swithed to state = " << "NORMAL" << ", id = " << m_header->device_id;
    }

    return;
}

long OscStateMachine::getData(const DDE_OSC_HEADER &hdr, DDE_GET_OSC_DATA* getDat)
{
    Q_ASSERT(getDat);

    memset(getDat, 0, sizeof(DDE_GET_OSC_DATA));
    getDat->device_id = hdr.device_id;

    _dde_func_return_t res = m_dde->get_osc_data(*getDat);

    return res;
}
