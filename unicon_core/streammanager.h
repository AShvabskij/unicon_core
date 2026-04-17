// streammanager.h
#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <QtWebSockets>
#include <QQueue>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QAtomicInt>
#include "ireqhandler.h"
#include "responsemanager.h"

class StreamWorker : public QObject
{
    Q_OBJECT
public:
    StreamWorker(QObject* parent = nullptr)
        : QObject(parent) {}

    void enqueue(const QList<QJsonObject>& list);

    void clearQueue();

    void pause(int delay);

public slots:
    void process();

signals:
    void sendMessage(const QByteArray& data);

private:
    QQueue<QJsonObject> m_queue;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
};

class StreamManager : public ResponseManager
{
    Q_OBJECT

public:
    enum NetworkSpeed {
        Unknown,
        Slow,
        Medium,
        Fast
    };

    StreamManager();
    ~StreamManager() override;

    static StreamManager* instance() {
        static StreamManager i;
        return &i;
    }

    void registerClient(QWebSocket* client) override;
    void unregisterClient(QWebSocket* client) override;

    int registerHandler(IReqHandler* handler) override;

    void handlePing();
    void handlePong(quint64 elapsedTime, const QByteArray& payload);
    NetworkSpeed checkCurrentSpeed();
    void checkSingleConnection();

public slots:
    int stream(const QList<QJsonObject>& valueList);
    void stop_stream();

private:
    QQueue<QWebSocket*> m_clients;
    quint64 m_totalBytes;
    quint64 m_currentPingTime = 0;
    bool m_isStarted = true;

    QTimer* m_speedMeasurementTimer;
    StreamWorker* m_worker;
    QThread* m_workerThread;
    StreamManager::NetworkSpeed m_currSpeed;
};

#endif // STREAM_MANAGER_H
