#include "responsemanager.h"

#include <cstdio>

ResponseManager::ResponseManager()
{

}

int ResponseManager::send(QJsonObject response)
{
    QJsonDocument doc(response);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    for (QWebSocket *client : m_clients) {
        client->sendTextMessage(strJson);
    }

    return 0;
}

void ResponseManager::registerClient(QWebSocket *client)
{
    if (m_clients.contains(client)) {
        return;
    }

    m_clients.append(client);
}

void ResponseManager::unregisterClient(QWebSocket *client)
{
    if (!m_clients.contains(client)) {
        return;
    }

    m_clients.removeOne(client);
}

int ResponseManager::registerHandler(IReqHandler *handler)
{
    connect(handler, &IReqHandler::send, this, &ResponseManager::send, Qt::QueuedConnection);
}

