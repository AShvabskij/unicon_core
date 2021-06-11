#ifndef RESPONSEMANAGER_H
#define RESPONSEMANAGER_H

#include <QtWebSockets>
#include "ireqhandler.h"

class ResponseManager : public QObject
{
    Q_OBJECT

public:
    ResponseManager();

    static ResponseManager* instance() {
        static ResponseManager i;
        return &i;
    }

    virtual void registerClient(QWebSocket* client);
    virtual void unregisterClient(QWebSocket* client);

    virtual int registerHandler(IReqHandler* handler);

public slots:
    int send(QJsonObject response);

private:

    QQueue<QWebSocket*> m_clients;
};

#endif // RESPONSEMANAGER_H
