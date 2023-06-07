#ifndef OSCREPOSITORY_H
#define OSCREPOSITORY_H

#include <QTimer>
#include <QMap>

#include "osc_types.h"

class IOscBufferService
{
public:
    ~IOscBufferService() {};
    virtual OscType::OscDataBuffer* get (DevInd id) = 0;
    virtual void clear(DevInd id) = 0;
    virtual OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) = 0;
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) = 0;
    virtual QJsonObject getSerialisedData(DevInd id, QVector<int> vars, int &cnt) = 0;
    virtual long saveToFile(const DDE_OSC_HEADER& hdr) = 0;
};

class OscBufferService: public IOscBufferService
{
public:

    static OscBufferService* instanse() {
        static OscBufferService m_instanse;

        return &m_instanse;
    }

    virtual OscType::OscDataBuffer* get(DevInd id);
    virtual void clear(DevInd id);
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat);
    virtual QJsonObject getSerialisedData(DevInd id, QVector<int> vars, int &cnt);
    virtual long saveToFile(const DDE_OSC_HEADER &hdr);

private:
    OscBufferService() = default;
    ~OscBufferService() = default;

    virtual OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr);
    long saveData(const DDE_OSC_HEADER& header, const OscType::OscDataBuffer& data);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);
    QJsonObject dataToJson(const OscType::OscDataBuffer &buff, QVector<int> vars, int startPos);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    static QString colorToString(const int &c);

    QMap<DevInd, OscType::OscDataBuffer*> m_repository;
};

#endif // OSCREPOSITORY_H
