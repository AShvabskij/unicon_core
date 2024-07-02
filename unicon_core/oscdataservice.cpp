#include "oscdataservice.h"

#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCborValue>
#include <QtConcurrent/QtConcurrent>

#include <QCoreApplication>


class MultiplyFunctor {
public:
    using result_type = QVariant;

    MultiplyFunctor(float scale, float offset) : scale(scale), offset(offset) {}

    QVariant operator()(const QVariant& value) const {
            return (value.toFloat() * scale) + offset;
    }


private:
    float scale;
    float offset;
};

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

void OscDataService::reset(DevInd ind)
{
    OscType::OscDataBuffer* buff = m_repository.value(ind);
    if (!buff)
        return;

    qDebug() << "Clearing buffer" << ", value count = " << buff->valueCount;
    m_mutex.lock();
//  clearDataBuffer(buff);

    delete buff;
    m_repository.remove(ind);
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

    for (OscChannelValues& chValues : buff->chArray) {
        chValues.clear();
    }
}

OscDataBuffer* OscDataService::createDataBuffer(const DDE_OSC_HEADER &hdr)
{
    OscDataBuffer* buff = new OscDataBuffer();

    buff->id = hdr.device_id;
    buff->timestamp = 0;
    buff->trig_time = hdr.settings.trig_time;
    buff->reason = hdr.settings.reason;

    for (int chInd = 0; chInd < hdr.settings.channels_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];

        if (channel.var.id == 0) continue;

        OscChannelValues& chValues = buff->chArray[chInd];
        chValues.channelNum = channel.chNum;
        chValues.varId = channel.var.id;
        chValues.scale = channel.gain;
        chValues.offset = channel.offset;
        chValues.type = channel.var.type;

        switch (channel.var.type) {
        case OSC_VAR_TYPE::OSC_VAR_INT:
        case OSC_VAR_TYPE::OSC_VAR_FLOAT: {
            chValues.numValues.reserve(MAX_DATA_COUNT);
        } break;
        case OSC_VAR_TYPE::OSC_VAR_DISCRETE: {
            chValues.discrValues.reserve(MAX_DATA_COUNT);
        } break;
        case UNDEFINED: {
            qWarning() << "Undefined var type" << ", id = " << channel.var.id << ", name = " << channel.var.name;
        }
        }
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

    if (buff->isOversized()) {
        qWarning() << "Osc buffer is oversized!" <<  " Count = " << buff->valueCount << "\n";
        clearDataBuffer(buff);
    }

    buff->eof = dat.eof;
    buff->sof = dat.sof;
    buff->timestamp = 0;
    buff->trig_time = hdr.settings.trig_time;
    buff->reason = hdr.settings.reason;

    if (dat.data_length == 0) {
        m_mutex.unlock();
        return _return_OK;
    }

    buff->valueCount += dat.data_length;

    for (int chInd = 0; chInd < hdr.settings.channels_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];

        if (channel.var.id == 0) continue;

        OscChannelValues& chValues = buff->chArray[chInd];

        const OSC_DATA& chData = dat.data[channel.chNum];

        switch (channel.var.type) {
        case OSC_VAR_TYPE::OSC_VAR_INT: {
            for (int i = 0; i < dat.data_length; i++) {
                auto val = chData.i_buff[i];
                chValues.append(val);
            }
        } break;
        case OSC_VAR_TYPE::OSC_VAR_FLOAT: {
            for (int i = 0; i < dat.data_length; i++) {
                auto val = chData.f_buff[i];
                chValues.append(val);
            }
        } break;

        case OSC_VAR_TYPE::OSC_VAR_DISCRETE: {
            chValues.discrValues.reserve(buff->valueCount + 1);
            for (int i = 0; i < dat.data_length; i++) {
                int32_t rawValue = chData.i_buff[i];
                qint8 val = discreteValue(rawValue, channel.firstBit, channel.lastBit);
                chValues.append(val);
            }
        } break;
        case UNDEFINED: {
            qWarning() << "Undefined var type" << ", id = " << channel.var.id << ", name = " << channel.var.name;
        }
        }
    }

    int resolution = static_cast<int>(hdr.settings.time_resolution_us);
    buff->timestamp = buff->valueCount  * resolution;

    m_mutex.unlock();

    emit dataReceived(buff->id);
    return _return_OK;
}

qint8 OscDataService::discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit)
{
    uint32_t mask = 0x0001;
    qint32 res = rawValue >> firstBit;

    bool isBit = (firstBit == lastBit);
    if (isBit) {
        res &= mask;
        return res;
    }

    uint16_t tmpVal = 0x000;
    for (int i = 0; i <= lastBit - firstBit; i++) {
        tmpVal |= mask;
        mask = mask << 1;
    }

    res &= tmpVal;

    return static_cast<qint8>(res);
}

QJsonObject OscDataService::serialisedData(DevInd ind, QVector<int> vars, int &cnt)
{
    OscType::OscDataBuffer* buff = m_repository.value(ind);
    Q_ASSERT(buff);

    m_mutex.lock();

    if (buff->lastDataPos == buff->valueCount) {
        cnt = 0;
        m_mutex.unlock();
        return QJsonObject();
    }

    QJsonObject res = dataToJson(*buff, vars, buff->lastDataPos);

    cnt = buff->valueCount - buff->lastDataPos;
    buff->lastDataPos = buff->valueCount;

    m_mutex.unlock();
    return res;
}

QVariantList multiplyArrayByCoefficient(QVariantList& numberArray, float scale, float offset) {
    QVariantList res = QtConcurrent::blockingMapped(numberArray, MultiplyFunctor(scale, offset));
    return res;
}

QJsonObject OscDataService::dataToJson(const OscType::OscDataBuffer& data, QVector<int> vars, int startPos) const
{
    QJsonObject res;
    QJsonArray valuesArr;
    QJsonArray varIdListObj;

    res["d_id"] = data.id;
    res["time"] = data.timestamp;
    res["trig_time"] = data.trig_time;
    res["reason"] = data.reason;
    res["eof"] = data.eof ? "1" : "0";
    res["sof"] = data.sof ? "1" : "0";

    for (const OscChannelValues& chVal : data.chArray) {
        if (chVal.varId == 0) continue;
        if (!vars.empty() && !vars.contains(chVal.varId)) {
            continue;
        }

        varIdListObj << chVal.varId;

        auto values = chVal.values(startPos);
        if (chVal.scale != 0.0 && chVal.scale != 1.0) {
            values= multiplyArrayByCoefficient(values, chVal.scale, chVal.offset);
        }

        valuesArr << QJsonArray::fromVariantList(values);
    }

    res["values"] = valuesArr;
    res["vars"] = varIdListObj;

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
