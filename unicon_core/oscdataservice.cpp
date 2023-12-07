#include "oscdataservice.h"

#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCborValue>

#include <QCoreApplication>

using namespace OscType;

OscType::OscDataBuffer* OscDataService::get(DevInd ind)
{
    return m_repository.value(ind, nullptr);
}

void OscDataService::clear(DevInd ind)
{
    OscType::OscDataBuffer* buff = m_repository.value(ind);
    if (!buff)
        return;

    m_mutex.lock();
    clearDataBuffer(buff);
    m_mutex.unlock();
}

void OscDataService::clearDataBuffer(OscType::OscDataBuffer* buff)
{
    Q_ASSERT(buff);
    buff->eof = false;
    buff->sof = false;
    buff->valueCount= 0;
    buff->timestamp = 0;
    buff->lastDataPos = 0;

    for (OscChannelValues& chVal : buff->ch) {
        chVal.values.clear();
    }
}

OscDataBuffer* OscDataService::createDataBuffer(const DDE_OSC_HEADER &hdr)
{
    OscDataBuffer* buff = new OscDataBuffer();

    buff->id = hdr.device_id;

    for (int chInd = 0; chInd < hdr.settings.channel_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];
        if (channel.var.id <= 0) {
            continue;
        }

        OscChannelValues& chValues = buff->ch[chInd];
        chValues.channelNum = channel.chNum;
        chValues.varId = channel.var.id;
        chValues.scale = channel.var.scale;
    }

    return buff;
}

long OscDataService::appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat)
{
    OscType::OscDataBuffer* buff = m_repository.value(hdr.device_id, nullptr);
    if (!buff) {
        buff = createDataBuffer(hdr);
        m_repository[hdr.device_id] = buff;
    }

    Q_ASSERT(buff);

    m_mutex.lock();

    if (buff->eof) {
        clearDataBuffer(buff); // prepare buffer to append a new data
    }

    buff->eof = dat.eof;
    buff->sof = dat.sof;
    buff->timestamp = 0;

    if (dat.data_length == 0) {
        m_mutex.unlock();
        return _return_OK;
    }

    buff->valueCount += dat.data_length;

    for (int chInd = 0; chInd < hdr.settings.channel_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];
        if (channel.var.id <= 0) {
            continue;
        }

        OscChannelValues& chValues = buff->ch[chInd];

        const OSC_DATA& chData = dat.data[channel.chNum];

        chValues.values.reserve(buff->valueCount + 1);

//      buff->valueDensity = buff->valueCount / DATA_YELD_INTERVAL_MSC;
        for (int i = 0; i < dat.data_length; i++) {
            if (channel.var.type == OSC_VAR_TYPE::DIGITAL) {
                int32_t rawValue = chData.i_buff[i];
                chValues.values.append(rawValue);
            } else if (channel.var.type == OSC_VAR_TYPE::DISCRETE) {
                int32_t rawValue = chData.i_buff[i];
                chValues.values.append(discreteValue(rawValue, channel.firstBit, channel.lastBit));
            } else {
                chValues.values.append(chData.f_buff[i]);
            }
        }
    }

    int resolution = static_cast<int>(hdr.settings.time_resolution_us);
    buff->timestamp = buff->valueCount  * resolution;

    m_mutex.unlock();

    emit dataReceived(buff->id);
    return _return_OK;
}

qint32 OscDataService::discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit)
{
    uint32_t mask = 0x0001;
    qint32 ret = rawValue >> firstBit;

    bool isBit = (firstBit == lastBit);
    if (isBit) {
        ret &= mask;
        return ret;
    }

    uint16_t tmpVal = 0x000;
    for (int i = 0; i <= lastBit - firstBit; i++) {
        tmpVal |= mask;
        mask = mask << 1;
    }

    ret &= tmpVal;

    return ret;
}

QJsonObject OscDataService::getSerialisedData(DevInd ind, QVector<int> vars, int &cnt)
{
    OscType::OscDataBuffer* buff = m_repository.value(ind);
    Q_ASSERT(buff);

    m_mutex.lock();

    if (buff->lastDataPos == buff->valueCount) {
        cnt = 0;
        m_mutex.unlock();
        return QJsonObject();
    }

    QJsonObject res = buff->toJson(vars, buff->lastDataPos);

    cnt = buff->valueCount - buff->lastDataPos;
    buff->lastDataPos = buff->valueCount;

    m_mutex.unlock();
    return res;
}

long OscDataService::save(const DDE_OSC_HEADER& hdr)
{
    OscType::OscDataBuffer* datBuff = m_repository.value(hdr.device_id);
    Q_ASSERT(datBuff);
    m_mutex.lock();
    long res = m_dataSaver->save(hdr, *datBuff);
    m_mutex.unlock();

    return res;
}
