#include "oscbuffservice.h"

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

OscType::OscDataBuffer* OscBufferService::get(DevInd id)
{
    return m_repository.value(id, nullptr);
}

void OscBufferService::clear(DevInd id)
{
    OscType::OscDataBuffer* buff = m_repository.value(id);
    if (!buff)
        return;

    clearDataBuffer(buff);
}

void OscBufferService::clearDataBuffer(OscType::OscDataBuffer* buff)
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

OscDataBuffer* OscBufferService::createDataBuffer(const DDE_OSC_HEADER &hdr)
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

long OscBufferService::appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat)
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

    buff->timestamp = buff->valueCount  * (hdr.settings.time_resolution_us);

    m_mutex.unlock();
    return _return_OK;
}

qint32 OscBufferService::discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit)
{
    uint32_t mask = 0x0001;
    uint32_t ret = rawValue >> firstBit;

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

QJsonObject OscBufferService::getSerialisedData(DevInd id, QVector<int> vars, int &cnt)
{
    OscType::OscDataBuffer* buff = m_repository.value(id);
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

QJsonObject OscBufferService::dataToJson(const OscDataBuffer &buff, QVector<int> vars, int startPos)
{
    QJsonObject res;
    QJsonArray valuesObj;
    QJsonArray varIdListObj;

    for (const OscChannelValues& chVal : buff.ch) {
        if (chVal.varId == 0) continue;
        if (!vars.empty() && !vars.contains(chVal.varId)) {
            continue;
        }

        varIdListObj << chVal.varId;
        valuesObj << QJsonArray::fromVariantList(chVal.values.mid(startPos,  chVal.values.size()));
    }

    res["d_id"] = buff.id;
    res["values"] = valuesObj;
    res["vars"] = varIdListObj;
    res["time"] = buff.timestamp;
    res["eof"] = buff.eof ? "1" : "0";
    res["sof"] = buff.sof ? "1" : "0";

    return res;
}

long OscBufferService::saveToFile(const DDE_OSC_HEADER& hdr)
{
    OscType::OscDataBuffer* buff = m_repository.value(hdr.device_id);
    Q_ASSERT(buff);
    m_mutex.lock();
    long res = saveData(hdr, *buff);
    m_mutex.unlock();

    return res;
}

long OscBufferService::saveData(const DDE_OSC_HEADER &header, const OscDataBuffer &data)
{
    const char* home = getenv("HOME");
    QString path =  home + QString("/projects/data/"); // qApp->applicationDirPath()
    _dde_func_return_t res = _return_OK;
    QJsonObject jsonObj = headerToJson(header);

    QJsonDocument doc(jsonObj);
    QByteArray bytes = doc.toJson(QJsonDocument::Compact);
    QDateTime trigTime = QDateTime::fromTime_t(header.settings.trig_time);
    QString fileName = path + QString("%1_%2_%3.hdr").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh:mm:ss:zzz"));
    QFile file( fileName );

    if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << bytes;
        file.close();
    }
    else
    {
         QTextStream(stdout) << "file open failed: " << fileName << endl;
         return _return_FAIL;
    }

    QJsonObject datjsonObj = dataToJson(data, QVector<int>(), 0);
    QString datFileName = path + QString("%1_%2_%3.dat").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh:mm:ss:zzz"));
    QFile datFile(datFileName);

    QJsonDocument datDoc(datjsonObj);
    QCborValue v = QCborValue::fromJsonValue(datjsonObj);
    QByteArray datBytes = v.toCbor(QCborValue::UseFloat);

    if( datFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        QTextStream iStream( &datFile );
        iStream << datBytes;
        datFile.close();
    }
    else
    {
         QTextStream(stdout) << "file open failed: " << fileName << endl;
         return _return_FAIL;
    }

    return res;
}

QJsonObject OscBufferService::headerToJson(const DDE_OSC_HEADER &h)
{
    QJsonObject res;

    res["device_id"] = h.device_id;
    res["id"] = h.device_id;
    res["trig_time"] = QString::number(h.settings.trig_time);
    res["resolution_us"] = QString::number(h.settings.time_resolution_us);

    QJsonArray channelsObj;
    for (int chInd = 0; chInd < h.settings.channel_count; chInd++) {
        const OSC_CHANNEL& ch = h.channels[chInd];
        QJsonObject obj;
        obj["ch_num"] = ch.chNum;
        obj["var_id"] = ch.var.id;
        obj["name"] = ch.var.name;
        obj["scale"] = ch.var.scale;
        obj["min"] = ch.var.min;
        obj["max"] = ch.var.max;
        obj["color"] = colorToString(ch.var.color);
        obj["isDiscrete"] = ch.var.type == OSC_VAR_TYPE::DISCRETE;
        obj["isAnalog"] = ch.var.type == OSC_VAR_TYPE::ANALOG;
        obj["isDigital"] = ch.var.type == OSC_VAR_TYPE::DIGITAL;

        channelsObj << obj;
    }

    res["channels"] = channelsObj;

    return res;
}

QString OscBufferService::colorToString(const int &c)
{
    QString ret = QString("#%1")
            .arg(QString::number(c, 16).rightJustified(6, '0'));

    return ret;

}
