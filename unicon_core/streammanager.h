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

class StreamWorker;
// class StreamWorker : public QObject {
//     Q_OBJECT
// public:
//     explicit StreamWorker(QAtomicInt* flag, qint64* totalBytes, QObject* parent = nullptr);

//     void enqueue(const QList<QJsonObject>& list);
//     void clearQueue();

// public slots:
//     void process();

// signals:
//     void sendMessage(const QByteArray& data);

// private:
//     QQueue<QJsonObject> m_queue;
//     QMutex m_mutex;
//     QWaitCondition m_waitCondition;
//     QAtomicInt* m_streamingFlag;
//     qint64* m_totalBytes;
// };

class StreamManager : public ResponseManager
{
    Q_OBJECT

public:
    StreamManager();
    ~StreamManager() override;

    static StreamManager* instance() {
        static StreamManager i;
        return &i;
    }

    void registerClient(QWebSocket* client) override;
    void unregisterClient(QWebSocket* client) override;

    int registerHandler(IReqHandler* handler) override;

public slots:
    int stream(const QList<QJsonObject>& valueList);
    void stop_stream();

private:
    QQueue<QWebSocket*> m_clients;
    qint64 m_totalBytes;
    bool m_isStarted = true;

    StreamWorker* m_worker;
    QThread* m_workerThread;
};

#endif // STREAM_MANAGER_H
