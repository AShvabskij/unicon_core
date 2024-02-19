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

int StreamManager::stream(const QList<QJsonObject>& valueList)
{
    for (QWebSocket *client : m_clients) {
        if (client == m_clients.last()) {
            for (const QJsonObject& value : valueList) {
                QCborValue v = QCborValue::fromJsonValue(value);
                QByteArray dataToSend = v.toCbor(QCborValue::UseFloat);
                client->sendBinaryMessage(dataToSend);
                qint64 bytes = client->bytesToWrite();
                // qDebug << " bytes to write = " << bytes << "\n" ;
                // qDebug() << "Response:" << value;
                m_totalBytes += bytes;
            }
        } else {
            // only one client have a right to receive stream messages, other - denied
            QJsonObject answer;
            answer["type"] = "sys";
            answer["status"] = "2"; // disable web client

            QJsonDocument doc(answer);

            QString strJson(doc.toJson(QJsonDocument::Compact));
            client->sendTextMessage(strJson);
        }


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
    QMetaObject::Connection con = connect(handler, &IReqHandler::stream, this, &StreamManager::stream, Qt::DirectConnection);
    if (!con) {
        QTextStream(stdout) << "The stream connection is failed! " << '\n';
        return -1;
    }

    return 0;
}

