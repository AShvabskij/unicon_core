#include "oscdatastateservice.h"
#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QCborValue>

#include <QCoreApplication>

using namespace OscData;

OscDataStateService::OscDataStateService(IDDE *dde)
{
    m_dde = dde;
}

QJsonObject OscDataStateService::getData(quint16 id, QVector<int> vars, int& cnt)
{
    if (m_oscData.keys().contains(id)) {
        return m_oscData[id]->serialisedData(vars, cnt);
    }

    return QJsonObject();
}

void OscDataStateService::init(QList<quint16> devList)
{
    for (quint16 devId: devList) {
        if (devId == DDE_DEV0_MASTER_IND)
            continue;
        if (!m_oscData.keys().contains(devId)) {
            m_oscData[devId] = new OscDataState(m_dde);
            //m_oscData[devId]->start()
        }
    }
}

void OscDataStateService::update()
{
    for (quint16 id: m_oscData.keys()) {
        m_oscData[id]->update(id);
    }
}

OscDataState::OscDataState(IDDE* dde)
{
    m_dde = dde;
    m_header = new DDE_OSC_HEADER();
    m_ddeData = new DDE_GET_OSC_DATA();
    m_state = STATE::Normal;
}

void OscDataState::update(quint16 devId)
{
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
                if (m_buff) {
                    clearBuffer();
                }
                m_state = Getting;
            }

            break;
        }

        case Getting: {
            if (!m_buff) {
                m_buff = createDataBuffer(*m_header);
            }
            auto res = getData(*m_header, m_buff);
            if (res == _return_FAIL) {
                qWarning() << "Error getting data from osc, id = " << m_header->device_id;
                m_state = Error;
                return;
            }

            if (res != _return_OK) return;

            m_buff->timestamp = m_buff->valueCount  * (m_header->settings.time_resolution_us);

            if (m_buff->sof) {
                m_sof = true;
            }

            if (m_buff->eof) {
                if (m_sof) {
                    m_state = Saving;
                } else {
                    m_state = Normal;
                }
            }

            break;
        }

        case Saving: {
            m_sof = false;
            saveData(*m_header, *m_buff);
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

    return;
}

QJsonObject OscDataState::serialisedData(QVector<int> vars, int& res)
{
    // todo insert in into lock section
    QJsonObject data = dataToJson(*m_buff, vars, m_lastDataPos);
    res = m_buff->valueCount - m_lastDataPos;
    m_lastDataPos = m_buff->valueCount;

    return data;
}

OscDataBuffer* OscDataState::buff()
{
    return m_buff;
}

void OscDataState::clearBuffer()
{
    Q_ASSERT(m_buff);

    m_buff->eof = false;
    m_buff->sof = false;
    m_buff->valueCount= 0;
    m_buff->timestamp = 0;

    for (OscChannelValues& chVal : m_buff->ch) {
        chVal.values.clear();
    }

    m_lastDataPos = 0;
}

OscDataBuffer *OscDataState::createDataBuffer(const DDE_OSC_HEADER &hdr)
{
   OscDataBuffer* buff = new OscDataBuffer();

   buff->id = hdr.device_id;
   buff->eof = false;
   buff->sof = false;

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

   m_lastDataPos = 0;

   return buff;
}

long OscDataState::getData(const DDE_OSC_HEADER &hdr, OscDataBuffer *buff)
{
    Q_ASSERT(buff);
    Q_ASSERT(m_ddeData);

    memset(m_ddeData, 0, sizeof(DDE_GET_OSC_DATA));
    m_ddeData->device_id = buff->id;

    _dde_func_return_t res = m_dde->get_osc_data(*m_ddeData);

    if (res != _return_OK) return res;

    // if (m_ddeData->data_length == 0) return res;

    for (int chInd = 0; chInd < hdr.settings.channel_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];
        if (channel.var.id <= 0) {
            continue;
        }

        OscChannelValues& chValues = buff->ch[chInd];

        const OSC_DATA& chData = m_ddeData->data[channel.chNum];

        if (m_ddeData->data_length > 0) {
            buff->valueCount += m_ddeData->data_length;
            chValues.values.reserve(buff->valueCount + 1);
        }

//      buff->valueDensity = buff->valueCount / DATA_YELD_INTERVAL_MSC;
        for (int i = 0; i < m_ddeData->data_length; i++) {
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

    buff->eof = m_ddeData->eof;
    buff->sof = m_ddeData->sof;

    return res;
}

long OscDataState::saveData(const DDE_OSC_HEADER &header, const OscDataBuffer &data)
{
    QString path = qApp->applicationDirPath() + "\\data\\";
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
    QString datFileName = QString("%1_%2.dat").arg(header.device_id).arg(header.settings.reason);
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

qint32 OscDataState::discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit)
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

QJsonObject OscDataState::dataToJson(const OscDataBuffer &buff, QVector<int> vars, int startPos)
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


    QTextStream(stdout) << "values count" << "=" << valuesObj.count() <<  ", time = " << buff.timestamp << "\n" ;
    return res;
}

QString OscDataState::colorToString(const int &c)
{
    QString ret = QString("#%1")
            .arg(QString::number(c, 16).rightJustified(6, '0'));

    return ret;

}

QJsonObject OscDataState::headerToJson(const DDE_OSC_HEADER &h)
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

