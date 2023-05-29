#ifndef OSC_DATA_H
#define OSC_DATA_H

#include "DDE_TOP.h"
#include "DDE_OSC_TYPES.h"
#include "DDE_DEVICES_TYPE.h"

#include <QTimer>
#include <QJsonObject>
#include <QMap>

namespace OscData {
    struct OscChannelValues
    {
        int channelNum = 0;
        uint16_t varId = 0;
        float scale = 0.0;
        QVariantList values;
    };

    struct OscDataBuffer
    {
        uint16_t id;
        int valueCount = 0; // // number of points in values buffer
        int valueDensity = 0; // number of points per millisec
        OscChannelValues ch[OSC_MAX_VARS + 1];
        qlonglong timestamp = 0;
        bool eof = false;
        bool sof = false;
    };
}

class OscDeviceData;
class OscDataService
{
public:
    OscDataService(IDDE* dde);
    QJsonObject getData(quint16 id, QVector<int> vars, int &cnt);
    void init(QList<quint16> devList);
    void update();

private:
    IDDE* m_dde;
    QMap<quint16, OscDeviceData*> m_oscData;
};

class OscDeviceData
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
    OscDeviceData(IDDE *dde);
    void update(quint16 devId);
    QJsonObject serialisedData(QVector<int> vars, int& res);
    OscData::OscDataBuffer* buff();

private:

    OscData::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr);
    long getData(const DDE_OSC_HEADER& hdr, OscData::OscDataBuffer* buff);
    long saveData(const DDE_OSC_HEADER& hdr, const OscData::OscDataBuffer& data);
    QJsonObject dataToJson(const OscData::OscDataBuffer &buff, QVector<int> vars, int startPos);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    QString colorToString(const int &c);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);
    void clearBuffer();

    IDDE* m_dde = nullptr;
    STATE m_state = Normal;
    DDE_OSC_HEADER* m_header;
    DDE_GET_OSC_DATA* m_ddeData;
    OscData::OscDataBuffer* m_buff = nullptr;
    bool m_sof = false;
    int m_lastDataPos = 0;
    int m_errCounter = 0;

};

#endif //OSC_DATA_H
