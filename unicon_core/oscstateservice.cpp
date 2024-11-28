#include "oscstateservice.h"

#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QCoreApplication>

const int MAX_OSC_ERROR_COUNT = 10;
const int MAX_OSC_IDLE_COUNT = 10;

OscStateService::OscStateService(IDDE *dde, IOscDataService *dataSrv)
{
    m_dde = dde;
    m_dataSrv = dataSrv;
}

void OscStateService::init(QList<DevInd> devList)
{
    m_mutex.lock();

    for (DevInd devId: devList) {
        if (devId == DDE_DEV0_MASTER_IND)
            continue;

        if (!m_devIdList.contains(devId)) {
            auto s = new OscStateMachine(m_dde, m_dataSrv);
            s->init(devId);
            m_oscState.insert(devId, s);
            m_devIdList.append(devId);
        }

        // break; for tests only
    }

    m_mutex.unlock();
}

void OscStateService::clear()
{
    m_mutex.lock();
    m_devIdList.clear();

    for (DevInd devId: m_oscState.keys()) {
//      m_oscState[devId]->finish();
        delete m_oscState[devId];
        m_oscState.remove(devId);
    }

    m_dataSrv->removeAll();

    m_mutex.unlock();
}

void OscStateService::update()
{
    m_mutex.lock();
    for (DevInd id: m_devIdList) {
        Q_ASSERT(m_oscState.keys().contains(id));
        if (!m_oscState.keys().contains(id))
            return;

        m_oscState[id]->update(id);
    }

    m_mutex.unlock();
}

OscStateMachine::OscStateMachine(IDDE* dde, IOscDataService *dataSrv)
{
    m_dde = dde;
    m_state = STATE::Normal;
    m_dataSrv = dataSrv;

    memset(&m_ddeData, 0, sizeof(DDE_GET_OSC_DATA));
    memset(&m_header, 0, sizeof(DDE_OSC_HEADER));
}

void OscStateMachine::update(DevInd devId)
{
    switch (m_state) {
    case Normal: {
        if (m_header.settings.trig_time > 0) {
            m_dataSrv->remove(m_header.device_id); // remove old data
            m_errCounter = 0;
            m_sof = false;
            m_eof = false;
        }

        m_header = getHeader(devId);
        if (m_header.device_id != devId) {
            m_state = Error;
            break;
        }

        if (m_header.settings.trig_time > 0) {
            m_state = Getting;
        } else {
            m_idleCounter = 0;
            m_state = Idle;
        }

        break;
    }

    case Idle: {
        m_idleCounter++;
        if (m_idleCounter > MAX_OSC_IDLE_COUNT) {
            m_state = Normal;
        }
        break;
    }

    case Getting: {
        int iterations = 0;
        do {
            iterations++;
            if (iterations > 2) {
                break;
            }

            if (m_ddeData.data_length == 0) { // get data only if m_ddeData is empty
                auto res = getData(m_header, m_ddeData);

                if (res == _return_Busy) {
                    m_state = Busy;
                    break;
                }

                if (res != _return_OK) {
                    qWarning() << "Error getting data from the osc, id = " << m_header.device_id;
                    m_state = Error;
                    break;
                }

                if (m_eof) {
                    m_dataSrv->clear(m_header.device_id);
                    m_errCounter = 0;
                    m_eof = false;
                    m_sof = false;
                }

                if (m_ddeData.header_updated) {
                    m_state = Updated;
                    break;
                }
            }

            auto res = m_dataSrv->appendData(m_header, m_ddeData); // Maybe to extract it to the Additional State

            if (res != _return_OK) {
                qWarning() << "Error getting buffer for the osc, id = " << m_header.device_id;
                m_state = Error;
                break;
            }

            m_ddeData.data_length = 0; // Empty m_ddeData to get the next block of data

            if (m_ddeData.sof) {
                m_sof = m_ddeData.sof;
            }

            if (m_ddeData.eof) {
                m_eof = true;
                if (m_sof) {
                    m_state = Saving;
                    break;
                }
            }

        } while (m_state == Getting && m_ddeData.next_ready == true);

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
        m_dataSrv->save(m_header);
        m_state = Getting;
        break;
    }
    case Updated: {
        // m_dataSrv->remove(m_header.device_id);
        m_state = Normal;
        break;
    }
    case Finished: {
        break;
    }
    case Error: {
        m_errCounter++;
        if (m_errCounter > MAX_OSC_ERROR_COUNT) {
            m_state = Finished;
            break;
        }

        m_state = Normal;
        break;
    }
    default: break;
    }

    return;
}

void OscStateMachine::init(DevInd devId)
{
    m_header = DDE_OSC_HEADER();
    m_header.device_id = devId;
    auto res = m_dde->get_osc_header(m_header); // open device and prepare device to read/write osc data
    if (res == _return_FAIL) {
        qWarning() << "Error getting header from osc, id = " << m_header.device_id;
        return;
    }
}

void OscStateMachine::finish()
{
    m_state = STATE::Finished;
}

long OscStateMachine::getData(const DDE_OSC_HEADER &hdr, DDE_GET_OSC_DATA& getDat)
{
    memset(&getDat, 0, sizeof(DDE_GET_OSC_DATA)); // this command results to compiler warning
    // getDat = DDE_GET_OSC_DATA(); // you must not do it so, because it results to memory corruption
    getDat.device_id = hdr.device_id;

    _dde_func_return_t res = m_dde->get_osc_data(getDat);

    return res;
}

DDE_OSC_HEADER OscStateMachine::getHeader(DevInd devId)
{
    DDE_OSC_HEADER hdr;
    hdr.device_id = devId;
    auto res = m_dde->get_osc_header(hdr);
    if (res == _return_FAIL) {
        qWarning() << "Error getting header from osc, id = " << m_header.device_id;
        m_state = Error;
        return DDE_OSC_HEADER();
    }

    return hdr;
}
