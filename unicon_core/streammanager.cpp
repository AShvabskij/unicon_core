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

public slots:
    void process() {
        int iCnt = 0;
        while (true) {
            QMutexLocker locker(&m_mutex);
            if (m_queue.isEmpty()) {
                qDebug() << "sent cnt" << iCnt;
                iCnt = 0;
                m_waitCondition.wait(&m_mutex);
                continue;
            }

            iCnt = (iCnt == 0) ? m_queue.count() : iCnt;
            QJsonObject obj = m_queue.dequeue();
            locker.unlock();

            QCborValue v = QCborValue::fromJsonValue(obj);
            QByteArray dataToSend = v.toCbor(QCborValue::UseFloat);

            emit sendMessage(dataToSend);

            int bytes = dataToSend.size();
            qDebug() << " bytes to write = " << bytes << "\n" ;

            if (bytes > 10000) {
                int delay = bytes / 10000;
                QThread::msleep(delay);
            }
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

    connect(m_workerThread, &QThread::started, m_worker, &StreamWorker::process);
    connect(m_worker, &StreamWorker::sendMessage, this, [this](const QByteArray& data){
        if (!m_clients.isEmpty()) {
            QWebSocket* client = m_clients.last();
            client->sendBinaryMessage(data);
            client->flush();

            qint64 bytes = client->bytesToWrite();
            //              qDebug() << " bytes to write = " << bytes << "\n" ;
            m_totalBytes += bytes;
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

    qDebug() << "Streaming data, count = " << valueList.count();

    Q_ASSERT(m_worker);
    if (!m_worker) return -1;

    if (!m_clients.isEmpty()) {
        m_worker->enqueue(valueList);
    }

    return 0;
}

void StreamManager::stop_stream() {
    if (m_worker) {
        m_worker->clearQueue();
    }
    qDebug() << "Streaming stopped";
}

void StreamManager::registerClient(QWebSocket* client) {
    if (!m_clients.contains(client)) {
        m_clients.enqueue(client);
    }
}

void StreamManager::unregisterClient(QWebSocket* client) {
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
