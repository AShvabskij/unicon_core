#include "oscdataservice.h"
#include "DDE_TYPES.h"

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


// class MultiplyFunctor {
// public:
//     using result_type = QVariant;

//     MultiplyFunctor(float scale, float offset) : scale(scale), offset(offset) {}

//     QVariant operator()(const QVariant& value) const {
//         return (value.toFloat() * scale) + offset;
//     }


// private:
//     float scale;
//     float offset;
// };

using namespace OscType;

OscType::OscDataBuffer* OscDataService::get(DevInd device_id, qlonglong time)
{
    QList<OscDataBuffer*> dev_buffers = m_repository.values(device_id);
    if (time == 0) {
        return dev_buffers.count() > 0 ? dev_buffers.first() : nullptr;
    }

    if (QDateTime::fromMSecsSinceEpoch(time).date().year() <= 1980) {
        time = time * 1000; // assume time is in seconds, need to convert to msec
    }

    for (OscDataBuffer* b: dev_buffers) {
        if (b->trig_time == time) {
            return b;
        }
    }

    return nullptr;
}

long OscDataService::load(const DDE_OSC_HEADER &hdr)
{
    OscType::OscDataBuffer* buff = get(hdr.device_id, hdr.settings.trig_time);
    long res = _return_OK;
    if (buff) {

        m_mutex.lock();
            buff->eof = true; // ??
            buff->resetPos(); // prepare to get data again
            buff->timestamp = buff->valueCount  * buff->resolution_us;
        m_mutex.unlock();

        res = _return_OK;

    } else {
        buff = createDataBuffer(hdr);
        res = m_dataSaver->loadData(hdr, *buff);
        if (res != _return_OK) {
            delete buff;
            return res;
        }

        res = appendBuffer(std::move(*buff));
    }


    emit dataReceived(hdr.device_id);
    return res;
}

void OscDataService::clear(DevInd device_id)
{
    OscType::OscDataBuffer* buff = get(device_id);
    if (!buff)
        return;

    m_mutex.lock();
    clearDataBuffer(buff);
    m_mutex.unlock();
}

void OscDataService::remove(DevInd device_id)
{
    OscType::OscDataBuffer* buff = get(device_id);
    if (!buff) {
        return;
    }

    qDebug() << "Clearing buffer" << ", value count = " << buff->valueCount;
    m_mutex.lock();
    //  clearDataBuffer(buff);

    m_repository.remove(device_id, buff);
    delete buff;
    m_mutex.unlock();
}

void OscDataService::removeAll()
{
    m_mutex.lock();

    for (auto ptr: m_repository.values()) {
        //  clearDataBuffer(buff);
        delete ptr;
    }

    m_repository.clear();
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
    buff->reason = hdr.settings.reason;
    buff->resolution_us = hdr.settings.time_resolution_us;

    std::time_t time = hdr.settings.trig_time;
    if (QDateTime::fromMSecsSinceEpoch(time).date().year() <= 1980) {
        time = time * 1000; // assume time is in seconds, need to convert to msec
    }
    buff->trig_time = time;

    for (int chInd = 0; chInd < hdr.settings.channels_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];

        if (channel.var.id == 0) continue;

        OscChannelValues& chValues = buff->chArray[chInd];
        chValues.channelNum = channel.chNum;
        chValues.varId = channel.var.id;
        chValues.scale = channel.gain;
        chValues.offset = channel.offset;
        chValues.type = channel.var.type;

        chValues.reserve(MAX_DATA_COUNT);
    }

    return buff;
}

long OscDataService::appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat)
{
    QElapsedTimer timer;
    timer.start();

    Q_ASSERT(dat.device_id == hdr.device_id);
    if (dat.device_id != hdr.device_id) {
        return _return_FAIL;
    }

    OscType::OscDataBuffer* buff = get(hdr.device_id, hdr.settings.trig_time);
    if (!buff) {
        buff = createDataBuffer(hdr);
        m_repository.insert(hdr.device_id, buff);
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
    // buff->trig_time = hdr.settings.trig_time;
    // buff->reason = hdr.settings.reason;

    if (dat.data_length == 0) {
        m_mutex.unlock();
        return _return_OK;
    }

    buff->valueCount += dat.data_length;
    qint8 ival_arr[dat.data_length + 1];

    for (int chInd = 0; chInd < hdr.settings.channels_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];

        if (channel.var.id == 0) continue;

        const OSC_DATA& chData = dat.data[channel.chNum];
        OscChannelValues& chValues = buff->chArray[chInd];

        switch (channel.var.type) {
        case OSC_VAR_TYPE::OSC_VAR_INT: {
            chValues.append(chData.i_buff, dat.data_length);
        } break;
        case OSC_VAR_TYPE::OSC_VAR_FLOAT: {
            chValues.append(chData.f_buff, dat.data_length);
        } break;

        case OSC_VAR_TYPE::OSC_VAR_DISCRETE: {
            for (int i = 0; i < dat.data_length; i++) {
                int32_t rawValue = chData.i_buff[i];
                ival_arr[i] = discreteValue(rawValue, channel.firstBit, channel.lastBit);
            }
            chValues.append(ival_arr, dat.data_length);
        } break;
        case UNDEFINED: {
            qWarning() << "Undefined var type" << ", id = " << channel.var.id << ", name = " << channel.var.name;
        }
        }

    }

    int resolution = static_cast<int>(hdr.settings.time_resolution_us);
    buff->timestamp = buff->valueCount  * resolution;

    m_mutex.unlock();

    qDebug() << "AppendData:"
             << "channels =" << hdr.settings.channels_count
             << "length =" << dat.data_length
             << "took" << timer.elapsed() << "ms";

    emit dataReceived(buff->id);
    return _return_OK;
}

long OscDataService::appendBuffer(OscType::OscDataBuffer&& buff)
{
    OscType::OscDataBuffer* dev_buff = nullptr;
    QList<OscDataBuffer*> dev_buffers = m_repository.values(buff.id);
    for (OscDataBuffer* b: dev_buffers) {
        if (b->trig_time == buff.trig_time && b->reason == buff.reason) {
            dev_buff = b;
            break;
        }
    }

    if (!dev_buff) {
        dev_buff = new OscType::OscDataBuffer();
        m_repository.insert(buff.id, dev_buff);
    }

    Q_ASSERT(dev_buff);

    m_mutex.lock();
    *dev_buff = std::move(buff);
    m_mutex.unlock();

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

QJsonObject OscDataService::jsonData(OscHeader &header, QVector<int> vars, int &cnt, bool& isEof)
{
    QElapsedTimer timer;
    timer.start();

    qlonglong trig_time = header.settings.trigDTime.toMSecsSinceEpoch();
    OscType::OscDataBuffer* buff = get(header.deviceID.id, trig_time);

    if (!buff) {
        return QJsonObject();
    }

    m_mutex.lock();


    QJsonObject res = dataToJson(*buff, vars, cnt, isEof);

    if (res.isEmpty()) {
        cnt = 0;
    }

    buff->lastDataPos = buff->lastDataPos + cnt;

    m_mutex.unlock();

//  qDebug() << "The serialisedData operation took" << timer.elapsed() << "milliseconds";
    return res;
}

// QVariantList multiplyArrayByCoefficient(QVariantList& numberArray, float scale, float offset) {
//     QVariantList res = QtConcurrent::blockingMapped(numberArray, MultiplyFunctor(scale, offset));
//     return res;
// }

QJsonObject OscDataService::dataToJson(OscType::OscDataBuffer& data, QVector<int> vars, int& cnt, bool &isEof) const
{
    QJsonArray valuesArr;
    QJsonArray varIdListObj;

    int timestamp = 0;
    for (OscChannelValues& chVal : data.chArray) {
        if (chVal.varId == 0) continue;
        if (!vars.isEmpty() && !vars.contains(chVal.varId)) {
            continue;
        }

        int startPos = chVal.lastDataPos;
        int ch_val_count = chVal.count();
        if (startPos >= ch_val_count) {
            continue;
        }

        // QVariantList values = chVal.values(startPos, cnt);
        // if (values.isEmpty()) {
        //     continue;
        // }

        // if (chVal.scale != 0.0 && chVal.scale != 1.0) {
        //     values= multiplyArrayByCoefficient(values, chVal.scale, chVal.offset);
        // }

        QJsonArray values = chVal.jsnValues(startPos, cnt);
        if (values.isEmpty()) {
            continue;
        }

        cnt = values.count();

        chVal.lastDataPos = startPos + cnt;
        isEof = (chVal.lastDataPos >= ch_val_count);
        timestamp = chVal.lastDataPos * data.resolution_us;

        varIdListObj << chVal.varId;
        // valuesArr << QJsonArray::fromVariantList(values);
        valuesArr << values;

    }


    if (varIdListObj.isEmpty()) {
        return QJsonObject();
    }

    QJsonObject res;
    res["d_id"] = data.id;
    res["time"] = timestamp;
    res["trig_time"] = data.trig_time;
    res["reason"] = data.reason;
    res["eof"] = data.eof && isEof ? "1" : "0";
    res["sof"] = data.sof ? "1" : "0";
    res["values"] = valuesArr;
    res["vars"] = varIdListObj;

    return res;
}

long OscDataService::save(const DDE_OSC_HEADER& hdr)
{
    OscType::OscDataBuffer* datBuff = get(hdr.device_id, hdr.settings.trig_time);
    Q_ASSERT(datBuff);
    m_mutex.lock();
    long res = m_dataSaver->save(hdr, *datBuff);
    m_mutex.unlock();

    return res;
}
