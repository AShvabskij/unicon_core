// streammanager.cpp
#include "streammanager.h"

#include <QJsonDocument>
#include <QTextStream>
#include <QTimer>
#include <QDebug>
#include <QCborValue>

class StreamWorker : public QObject {
    Q_OBJECT
public:
    StreamWorker(QObject* parent = nullptr)
        : QObject(parent) {}

    void enqueue(const QList<QJsonObject>& list) {
        QMutexLocker locker(&m_mutex);
        for (const QJsonObject& obj : list) {
            m_queue.enqueue(obj);
        }
        m_waitCondition.wakeAll();
    }

    void clearQueue() {
        if (m_queue.empty()) return;

        QMutexLocker locker(&m_mutex);
        m_queue.clear();
    }

    void pause(int delay) {
        if (m_queue.empty()) return;

        QMutexLocker locker(&m_mutex);
        QThread::msleep(delay);
    }

public slots:
    void process() {
        while (true) {
            QMutexLocker locker(&m_mutex);
            if (m_queue.isEmpty()) {
                m_waitCondition.wait(&m_mutex);
                continue;
            }

            QJsonObject obj = m_queue.dequeue();
            locker.unlock();

            QCborValue v = QCborValue::fromJsonValue(obj);
            QByteArray dataToSend = v.toCbor(QCborValue::UseFloat);

            emit sendMessage(dataToSend);

            // int bytes = dataToSend.size();
            // qDebug() << " bytes to write = " << bytes << "\n" ;

            // if (bytes > 10000) {
            //     int delay = bytes / 1000;
            //     QThread::msleep(delay);
            // }
        }
    }

signals:
    void sendMessage(const QByteArray& data);

private:
    QQueue<QJsonObject> m_queue;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
};

#include "streammanager.moc"

StreamManager::StreamManager()
    : m_totalBytes(0)
{
    m_worker = new StreamWorker();
    m_workerThread = new QThread();
    m_worker->moveToThread(m_workerThread);

    m_speedMeasurementTimer = new QTimer(this);
    connect(m_speedMeasurementTimer, &QTimer::timeout, this, &StreamManager::handlePing);
    m_speedMeasurementTimer->start(10000);

    connect(m_workerThread, &QThread::started, m_worker, &StreamWorker::process);
    connect(m_worker, &StreamWorker::sendMessage, this, [this](const QByteArray& data){
        if (!m_isStarted) return;
        if (m_clients.isEmpty()) return;

        QWebSocket* client = m_clients.last();
        client->sendBinaryMessage(data);

        qint64 bytes = client->bytesToWrite();
        client->flush();

        m_totalBytes += bytes;

        // need to delay after sending big chunks, to avoid traffic overflow
        bool isLargeChunk = bytes > 1000;
        StreamManager::NetworkSpeed currSpeed = getCurrentSpeed();

        int NETWORK_COEF = (currSpeed == Fast) ? 10000 : 1000; // the delay depends on network speed: 1000 - for local connection , 10000 - for remote vpn connection

        if (isLargeChunk) {
            int delay = bytes / NETWORK_COEF;
            delay = (delay <= 100) ? delay : 100; // delay not more than 100 ms
            m_worker->pause(delay);
        }

        if (m_clients.count() > 1) {
            for (QWebSocket *client : m_clients) {
                if (client == m_clients.last()) break;

                // only one client have a right to receive stream messages, other - denied
                QJsonObject answer;
                answer["type"] = "sys";
                answer["status"] = "2"; // disable web client

                QJsonDocument doc(answer);

                QString strJson(doc.toJson(QJsonDocument::Compact));
                client->sendTextMessage(strJson);
//              client->close(QWebSocketProtocol::CloseCodePolicyViolated); // don't close the connection please
            }
        }
    });

    m_workerThread->start();
}

StreamManager::~StreamManager() {
    m_workerThread->quit();
    m_workerThread->wait();
    delete m_worker;
    QTextStream(stdout) << " Total bytes sended = " << m_totalBytes << "\n";
}

int StreamManager::stream(const QList<QJsonObject>& valueList) {
    if (valueList.empty()) return 0;

    m_isStarted = true;

    Q_ASSERT(m_worker);
    if (!m_worker) return -1;

    if (!m_clients.isEmpty()) {
        m_worker->enqueue(valueList);
    }

    return 0;
}

void StreamManager::stop_stream() {
    m_isStarted = false;
    if (m_worker) {
        m_worker->clearQueue();
    }
//  qDebug() << "Streaming stopped";
}

void StreamManager::registerClient(QWebSocket* client) {
    if (!m_clients.contains(client)) {
        m_clients.enqueue(client);

        connect(client, &QWebSocket::pong, this, &StreamManager::handlePong);
    }
}

void StreamManager::unregisterClient(QWebSocket* client) {
    disconnect(client, &QWebSocket::pong, this, &StreamManager::handlePong);
    m_clients.removeAll(client);
}

int StreamManager::registerHandler(IReqHandler* handler) {
    QMetaObject::Connection con_stream = connect(handler, &IReqHandler::stream, this, &StreamManager::stream, Qt::QueuedConnection);
    QMetaObject::Connection con_stop_stream = connect(handler, &IReqHandler::stop_stream, this, &StreamManager::stop_stream, Qt::QueuedConnection);

    if (!con_stream || !con_stop_stream) {
        QTextStream(stdout) << "The stream connection is failed! " << '\n';
        return -1;
    }

    return 0;
}

void StreamManager::handlePing()
{
    if (m_clients.isEmpty()) return;

    QWebSocket *client = m_clients.last();
    if (client->state() != QAbstractSocket::ConnectedState) return;

    const int payloadSize = 120; // 120 B
    QByteArray payload;
    payload.reserve(payloadSize);

    // Fill the payload with some data (could be random or pattern-based)
    for (int i = 0; i < payloadSize; ++i) {
        payload.append(static_cast<char>(i % 256)); // Simple repeating pattern
    }

    client->ping(payload);
}

void StreamManager::handlePong(quint64 elapsedTime, const QByteArray& payload) {
    Q_UNUSED(payload);
    if (m_currentPingTime != elapsedTime) {
        if ((m_currentPingTime - elapsedTime) /elapsedTime > 0.1) {
            qDebug() << "Streaming ping time" << elapsedTime << "ms";
        }

        m_currentPingTime = elapsedTime;
    }
}

StreamManager::NetworkSpeed StreamManager::getCurrentSpeed() const {
    if (m_currentPingTime <= 10) return Fast;
    if (m_currentPingTime > 10 && m_currentPingTime <=100) return Medium;
    if (m_currentPingTime > 100) return Slow;

    return Unknown;
}
