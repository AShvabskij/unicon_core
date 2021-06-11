#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <QtWebSockets>
#include "ireqhandler.h"
#include "responsemanager.h"

class StreamManager : public ResponseManager
{
    Q_OBJECT

public:
    StreamManager();

    static StreamManager* instance() {
        static StreamManager i;
        return &i;
    }

    void registerClient(QWebSocket* client) override;
    void unregisterClient(QWebSocket* client) override;

    int registerHandler(IReqHandler* handler) override;

public slots:
    int stream(QJsonObject value);

private:
    void threadProcess();

    QQueue<QWebSocket*> m_clients;
};

#endif // STREAM_MANAGER_H
