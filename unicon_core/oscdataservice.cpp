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
    using result_type = float;

    MultiplyFunctor(float scale, float offset) : scale(scale), offset(offset) {}

    float operator()(const float value) const {
            return (value * scale) + offset;
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

    m_mutex.lock();
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

    for (OscChannelValues& chValues : buff->ch) {
        chValues.clear();
    }
}

OscDataBuffer* OscDataService::createDataBuffer(const DDE_OSC_HEADER &hdr)
{
    OscDataBuffer* buff = new OscDataBuffer();

    buff->id = hdr.device_id;
    buff->timestamp = hdr.settings.trig_time;

    for (int chInd = 0; chInd < hdr.settings.channel_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];

        OscChannelValues& chValues = buff->ch[chInd];
        chValues.channelNum = channel.chNum;
        chValues.varId = channel.var.id;
        chValues.scale = channel.gain;
        chValues.offset = channel.offset;
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

    if (dat.data_length == 0) {
        m_mutex.unlock();
        return _return_OK;
    }

    buff->valueCount += dat.data_length;

    for (int chInd = 0; chInd < hdr.settings.channel_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];

        OscChannelValues& chValues = buff->ch[chInd];

        const OSC_DATA& chData = dat.data[channel.chNum];

        switch (channel.var.type) {
        case OSC_VAR_TYPE::OSC_VAR_INT: {
            chValues.numValues.reserve(buff->valueCount + 1);
            Number32 val;
            for (int i = 0; i < dat.data_length; i++) {
                val.i = chData.i_buff[i];
                chValues.numValues.append(val);
            }
        } break;
        case OSC_VAR_TYPE::OSC_VAR_FLOAT: {
            chValues.numValues.reserve(buff->valueCount + 1);
            Number32 val;
            for (int i = 0; i < dat.data_length; i++) {
                val.f = chData.f_buff[i];
                chValues.numValues.append(val);
            }
        } break;

        case OSC_VAR_TYPE::OSC_VAR_DISCRETE: {
            chValues.discrValues.reserve(buff->valueCount + 1);
            for (int i = 0; i < dat.data_length; i++) {
                int32_t rawValue = chData.i_buff[i];
                qint8 dVal = discreteValue(rawValue, channel.firstBit, channel.lastBit);
                chValues.discrValues.append(dVal);
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

void multiplyArrayByCoefficient(QList<float>& numberArray, float scale, float offset) {
    QList<float> res = QtConcurrent::blockingMapped(numberArray, MultiplyFunctor(scale, offset));
    numberArray = res;
}

template<typename T> QJsonArray convertToJSonArray(QList<T> values)
{
    QJsonArray res;
    for (const auto &val : values) {
        res << val;
    }

    return res;
}

QJsonObject OscDataService::dataToJson(const OscType::OscDataBuffer& data, QVector<int> vars, int startPos) const
{
    QJsonObject res;
    QJsonArray valuesArr;
    QJsonArray varIdListObj;

    res["d_id"] = data.id;
    res["time"] = data.timestamp;
    res["eof"] = data.eof ? "1" : "0";
    res["sof"] = data.sof ? "1" : "0";

    for (const OscChannelValues& chVal : data.ch) {
        if (chVal.varId == 0) continue;
        if (!vars.empty() && !vars.contains(chVal.varId)) {
            continue;
        }

        varIdListObj << chVal.varId;

        switch (chVal.type) {
        case OscChannelValues::IntegerType: {
            if (chVal.scale != 0.0 && chVal.scale != 1.0) {
                auto values = chVal.fltValues(startPos);
                multiplyArrayByCoefficient(values, chVal.scale, chVal.offset);
                valuesArr << convertToJSonArray(values);
            } else {
                auto values = chVal.intValues(startPos);
                valuesArr << convertToJSonArray(values);
            }
        } break;
        case OscChannelValues::FloatType: {
            if (chVal.scale != 0.0 && chVal.scale != 1.0) {
                auto values = chVal.fltValues(startPos);
                multiplyArrayByCoefficient(values, chVal.scale, chVal.offset);
                valuesArr << convertToJSonArray(values);
            } else {
                auto values = chVal.intValues(startPos);
                valuesArr << convertToJSonArray(values);
            }

        } break;
        case OscChannelValues::DiscreteType: {
            auto values = chVal.dscrValues(startPos);
            valuesArr << convertToJSonArray(values);
        }
        }
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
