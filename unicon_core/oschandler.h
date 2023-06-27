#ifndef OSCHANDLER_H
#define OSCHANDLER_H

#include "basereqhandler.h"
#include "DDE_TOP.h"
#include "osc_types.h"
#include "oscbuffservice.h"

#include <QTimer>

class OscHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    OscHandler(IDDE_Dispatcher* , IOscBufferService* buffSrv);
    virtual int handle(const QJsonObject& request);

signals:
    void requestStreamValue();

private slots:
    void onReceivedData(quint16 ind);

private:
    int handleGetHeader(const QJsonObject &request);
    int handleGetChannel(const QJsonObject &request);
    int handleOpenStream(const QJsonObject &request);
    int handleCloseStream(const QJsonObject &request);

    long getHeader(const DevID& deviceID, int oscId, OscType::OscHeader *out);

    QJsonObject createHeaderObj(int requestId, const OscType::OscHeader& header);
    QJsonObject createChannelObj(int requestId, const OscType::OscChannelDescr& ch);
    QJsonObject createAnswerObj(int requestId, DevID deviceID, const QJsonObject &body = QJsonObject(), int error = 0);
    OscType::OscChannelDescr createChannelDescr(const OSC_CHANNEL &channel);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);

    void startStreamData(const OscType::OscHeader& header, QVector<int> oscVars);
    void stopStreamData();
    void streamData();

    OscType::OscHeader m_capturedOsc;
    QVector<int> m_capturedVars;
    int m_streamValCount = 0;

    IOscBufferService* m_buffSrv;
};

#endif // OSCHANDLER_H
