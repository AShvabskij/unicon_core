#ifndef OSCREPOSITORY_H
#define OSCREPOSITORY_H

#include <QObject>
#include <QDebug>

#include <QTimer>
#include <QMap>
#include <QMutex>

#include "osc_types.h"

class IOscBufferService
{

public:
    virtual ~IOscBufferService() {}
    virtual OscType::OscDataBuffer* get (DevInd ind) = 0;
    virtual void clear(DevInd ind) = 0;
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) = 0;
    virtual QJsonObject getSerialisedData(DevInd ind, QVector<int> vars, int &cnt) = 0;
    virtual long saveToFile(const DDE_OSC_HEADER& hdr) = 0;

// signals:
    virtual void dataReceived(quint16 ind) = 0;

};

class OscBufferService: public QObject,
                        public IOscBufferService
{
     Q_OBJECT
public:
    OscBufferService() = default;
    ~OscBufferService() override {}

    static OscBufferService* instanse() {
        static OscBufferService m_instanse;

        return &m_instanse;
    }

    OscType::OscDataBuffer* get(DevInd ind) override;
    void clear(DevInd id) override;
    long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) override;
    QJsonObject getSerialisedData(DevInd ind, QVector<int> vars, int &cnt) override;
    long saveToFile(const DDE_OSC_HEADER &hdr) override;

signals:
    void dataReceived(quint16 ind) override;

private:

    OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr);
    void  clearDataBuffer(OscType::OscDataBuffer* buff);
    long saveData(const DDE_OSC_HEADER& header, const OscType::OscDataBuffer& data);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);
    QJsonObject dataToJson(const OscType::OscDataBuffer &buff, QVector<int> vars, int startPos);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    static QString colorToString(const int &c);

    QMap<DevInd, OscType::OscDataBuffer*> m_repository;
    QMutex m_mutex;
};

#endif // OSCREPOSITORY_H
