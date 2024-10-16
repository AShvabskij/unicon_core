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
    void reset(DevInd ind) override;
    long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) override;
    QJsonObject serialisedData(DevInd ind, QVector<int> vars, int &cnt) override;
    long save(const DDE_OSC_HEADER &hdr) override;
    OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) override;

    // methods to move to another service
    long appendToHistoryData(const DDE_OSC_HEADER& hdr, OscType::OscDataBuffer &&dat) override;
    QJsonObject historyData(DevInd ind, QVector<int> vars, int &cnt) override;
    virtual OscType::OscDataBuffer* getHistoryData(const DDE_OSC_HEADER& hdr) override;

signals:
    void dataReceived(quint16 ind) override;

private:
    void  clearDataBuffer(OscType::OscDataBuffer* buff);
    long saveData(const DDE_OSC_HEADER& header, const OscType::OscDataBuffer& data);
    qint8 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);
    QJsonObject dataToJson(OscType::OscDataBuffer &data, QVector<int> vars, int cnt) const;
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);

    QMap<DevInd, OscType::OscDataBuffer*> m_repository;
    QMutex m_mutex;
    QMutex m_historyMutex;
    IOscDataStorageService* m_dataSaver = nullptr;
    OscType::OscDataBuffer m_historyBuff;
};

#endif // OSCDATASERVICE_H
