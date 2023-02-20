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
    ~StreamManager() override;

    static StreamManager* instance() {
        static StreamManager i;
        return &i;
    }

    void registerClient(QWebSocket* client) override;
    void unregisterClient(QWebSocket* client) override;

    int registerHandler(IReqHandler* handler) override;

public slots:
    int stream(const QList<QJsonObject> &valueList);

private:
    void threadProcess();

    QQueue<QWebSocket*> m_clients;
    qint64 m_totalBytes = 0;
};

#endif // STREAM_MANAGER_H
