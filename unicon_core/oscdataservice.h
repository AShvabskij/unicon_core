#ifndef OSCDATASERVICE_H
#define OSCDATASERVICE_H

#include <QObject>
#include <QDebug>

#include <QTimer>
#include <QMap>
#include <QMutex>

#include "osc_types.h"

class OscDataService: public QObject,
                        public IOscDataService
{
     Q_OBJECT
public:
    OscDataService(IOscDataStorageService* s) {
        Q_ASSERT(s);
        m_dataSaver = s;
    };

    ~OscDataService() override {}

    OscType::OscDataBuffer* get(DevInd ind) override;
    void clear(DevInd id) override;
    long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) override;
    QJsonObject getSerialisedData(DevInd ind, QVector<int> vars, int &cnt) override;
    long save(const DDE_OSC_HEADER &hdr) override;
    OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr);

signals:
    void dataReceived(quint16 ind) override;

private:

    void  clearDataBuffer(OscType::OscDataBuffer* buff);
    long saveData(const DDE_OSC_HEADER& header, const OscType::OscDataBuffer& data);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);
    QJsonObject dataToJson(const OscType::OscDataBuffer &buff, QVector<int> vars, int startPos);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    static QString colorToString(const int &c);

    QMap<DevInd, OscType::OscDataBuffer*> m_repository;
    QMutex m_mutex;
    IOscDataStorageService* m_dataSaver = nullptr;
};

#endif // OSCDATASERVICE_H
