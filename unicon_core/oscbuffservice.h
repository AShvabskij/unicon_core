#ifndef OSCREPOSITORY_H
#define OSCREPOSITORY_H

#include <QTimer>
#include <QMap>

#include "DDE_TYPES.h"
#include "DDE_OSC_TYPES.h"

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
        int lastDataPos = 0;
        OscChannelValues ch[OSC_MAX_VARS + 1];
        qlonglong timestamp = 0;
        bool eof = false;
        bool sof = false;
    };
}

class IOscBufferService
{
public:
    ~IOscBufferService() {};
    virtual OscData::OscDataBuffer* get (uint16_t id) = 0;
    virtual void clear(uint16_t id) = 0;
    virtual OscData::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) = 0;
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) = 0;
    virtual QJsonObject getSerialisedData(quint16 id, QVector<int> vars, int &cnt) = 0;
    virtual long saveToFile(const DDE_OSC_HEADER& hdr) = 0;
};

class OscBufferService: public IOscBufferService // todo: make it as singleton
{
public:

    static OscBufferService* instanse() {
        static OscBufferService m_instanse;

        return &m_instanse;
    }

    virtual OscData::OscDataBuffer* get(uint16_t id);
    virtual void clear(uint16_t id);
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat);
    virtual QJsonObject getSerialisedData(quint16 id, QVector<int> vars, int &cnt);
    virtual long saveToFile(const DDE_OSC_HEADER &hdr);

private:
    OscBufferService() = default;
    ~OscBufferService() = default;

    virtual OscData::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr);
    long saveData(const DDE_OSC_HEADER& header, const OscData::OscDataBuffer& data);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);
    QJsonObject dataToJson(const OscData::OscDataBuffer &buff, QVector<int> vars, int startPos);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    static QString colorToString(const int &c);

    QMap<uint16_t, OscData::OscDataBuffer*> m_repository;
};

#endif // OSCREPOSITORY_H
