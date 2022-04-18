#ifndef ICMDHANDLER_H
#define ICMDHANDLER_H

#include <QtWebSockets>

class IReqHandler : public QObject
{
    Q_OBJECT
public:
    virtual ~IReqHandler() {};

    virtual int handle(const QJsonObject& request) = 0;
    virtual void setNext(IReqHandler* next) = 0;

    Q_SIGNAL void send(const QJsonObject& response);
    Q_SIGNAL void stream(const QJsonObject& value);
};

#endif // ICMDHANDLER_H
