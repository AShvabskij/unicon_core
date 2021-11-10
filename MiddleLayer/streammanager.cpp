#include "streammanager.h"

#include <QTextStream>
#include <cstdio>
#include <QCborValue>

StreamManager::StreamManager()
{
}

StreamManager::~StreamManager()
{
    QTextStream(stdout) << " Total bytes sended = " << m_totalBytes << "\n" ;
}

int StreamManager::stream(QJsonObject value)
{
    QJsonDocument doc(value);
//  QString strJson(doc.toJson(QJsonDocument::Compact));

    QCborValue v = QCborValue::fromJsonValue(value);
    QByteArray dataToSend = v.toCbor(QCborValue::UseFloat);

    for (QWebSocket *client : m_clients) {

//      client->sendTextMessage(strJson);
        client->sendBinaryMessage(dataToSend);
        qint64 bytes = client->bytesToWrite();
        QTextStream(stdout) << " bytes to write = " << bytes << "\n" ;
        m_totalBytes += bytes;

        client->flush();
    }

    return 0;
}

void StreamManager::registerClient(QWebSocket *client)
{
    if (m_clients.contains(client)) {
        return;
    }

    m_clients.append(client);
}

void StreamManager::unregisterClient(QWebSocket *client)
{
    if (!m_clients.contains(client)) {
        return;
    }

    m_clients.removeAll(client);
}

int StreamManager::registerHandler(IReqHandler *handler)
{
    QMetaObject::Connection con = connect(handler, &IReqHandler::stream, this, &StreamManager::stream, Qt::QueuedConnection);
    if (!con) {
        QTextStream(stdout) << "The stream connection is failed! " << '\n';
        return -1;
    }

    return 0;
}

