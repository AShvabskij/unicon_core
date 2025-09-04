#ifndef OSCDATASERVICE_H
#define OSCDATASERVICE_H

#include <QObject>
#include <QDebug>

#include <QTimer>
#include <QMultiMap>
#include <QMutex>

#include "osc_types.h"

class OscDataService: public QObject,
                        public IOscDataService
{
     Q_OBJECT
public:
    OscDataService(IOscDataLogger* s) {
        Q_ASSERT(s);
        m_dataLogger = s;
    };

    ~OscDataService() override {}

    long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) override;
    long dataCount(const DevID& deviceID, qlonglong trig_time, int &cnt) override;
    QJsonObject jsonData(const DevID& deviceID, qlonglong trig_time, QVector<int> vars, int &cnt, bool &isEof) override;
    void clear(DevInd device_id) override;
    void remove(DevInd device_id) override;
    void removeAll() override;
    long save(const DDE_OSC_HEADER &hdr, SysType sysType) override;
    long load(const DDE_OSC_HEADER& hdr, SysType sysType) override;

signals:
    void dataReceived(quint16 device_id) override;
    void dataSaved(quint16 device_id) override;

private:
    OscType::OscDataBuffer* get(DevInd device_id, qlonglong time = 0);

    long appendBuffer(OscType::OscDataBuffer &&buff);
    OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr);
    void  clearDataBuffer(OscType::OscDataBuffer* buff);
    long saveData(const DDE_OSC_HEADER& header, const OscType::OscDataBuffer& data);
//  inline qint8 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);

    QMultiMap<DevInd, OscType::OscDataBuffer*> m_repository;
    QMutex m_mutex;
    IOscDataLogger* m_dataLogger = nullptr;
};

#endif // OSCDATASERVICE_H
