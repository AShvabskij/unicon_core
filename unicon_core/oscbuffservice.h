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
    virtual OscType::OscDataBuffer* get (DevInd id) = 0;
    virtual void clear(DevInd id) = 0;
    virtual OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) = 0;
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) = 0;
    virtual QJsonObject getSerialisedData(DevInd id, QVector<int> vars, int &cnt) = 0;
    virtual long saveToFile(const DDE_OSC_HEADER& hdr) = 0;
/*
protected:
    virtual void dataReceived() = 0;
*/
};

// Q_DECLARE_INTERFACE(IOscBufferService, "IOscBufferService")

class OscBufferService: public IOscBufferService,
                        public QObject
{
//   Q_OBJECT
//   Q_INTERFACES(IOscBufferService)
public:
    OscBufferService() = default;
    virtual ~OscBufferService() {}

    static OscBufferService* instanse() {
        static OscBufferService m_instanse;

        return &m_instanse;
    }

    virtual OscType::OscDataBuffer* get(DevInd id);
    virtual void clear(DevInd id);
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat);
    virtual QJsonObject getSerialisedData(DevInd id, QVector<int> vars, int &cnt);
    virtual long saveToFile(const DDE_OSC_HEADER &hdr);
/*
signals:
    void dataReceived();
*/
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


class OscBufferService2: public QObject
{
      Q_OBJECT
//   Q_INTERFACES(IOscBufferService)
public:
    OscBufferService2() = default;
    virtual ~OscBufferService2() {}

    static OscBufferService2* instanse() {
        static OscBufferService2 m_instanse;

        return &m_instanse;
    }

/*
signals:
    void dataReceived();
*/
};


class IClock
{
public:
    ~IClock() {}
    virtual void doSomething() = 0;
/*
    virtual OscType::OscDataBuffer* get (DevInd id) = 0;
    virtual void clear(DevInd id) = 0;
    virtual OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) = 0;
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) = 0;
    virtual QJsonObject getSerialisedData(DevInd id, QVector<int> vars, int &cnt) = 0;
    virtual long saveToFile(const DDE_OSC_HEADER& hdr) = 0;
*/
    // Not a signal (but can be used as a signal), because IClock is not QObject
    virtual void alarm() = 0;
};

class DigitalClock : public QObject, public IClock
{
    Q_OBJECT

public:
    DigitalClock() {}

    void doSomething() Q_DECL_OVERRIDE {
        qDebug() << "Do something...";
    }
/*
    virtual OscType::OscDataBuffer* get (DevInd id) {return nullptr;}
    virtual void clear(DevInd id) {}
    virtual OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER& hdr) {return nullptr;}
    virtual long appendData(const DDE_OSC_HEADER& hdr, const DDE_GET_OSC_DATA& dat) { return 0; }
    virtual QJsonObject getSerialisedData(DevInd id, QVector<int> vars, int &cnt) { return QJsonObject(); }
    virtual long saveToFile(const DDE_OSC_HEADER& hdr) { return 0; }
*/
signals:
    // Implementation is done by moc
    void alarm() Q_DECL_OVERRIDE;
};

#endif // OSCREPOSITORY_H
