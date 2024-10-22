#ifndef OSCHANDLER_H
#define OSCHANDLER_H

#include "basereqhandler.h"
#include "DDE_TOP.h"
#include "osc_types.h"
#include "oscdataservice.h"

#include "oschistoryservice.h"
#include <QTimer>

class OscHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    OscHandler(IDDE_Dispatcher* , SysType sysType, IOscDataService* buffSrv);
    virtual int handle(const QJsonObject& request);
    void setService(OscHistoryService* s);

signals:
    void requestStreamValue();

private slots:
    void onReceivedData(quint16 ind);
    void onReceivedHistoryData(quint16 ind);

private:
    int handleGetHeader(const QJsonObject &request);
    int handleSetHeader(const QJsonObject &request);
    int handleGetChannel(const QJsonObject &request);
    int handleOpenStream(const QJsonObject &request);
    int handleCloseStream(const QJsonObject &request);

    long getHeader(const DevID& deviceID, OscType::OscHeader *out);
    long convertHeader(const DDE_OSC_HEADER& header, OscType::OscHeader *out);
    long setHeader(const DevID& deviceID, const OscType::OscSettings &settings);

    QJsonObject createHeaderObj(int requestId, const OscType::OscHeader& header);
    QJsonObject createChannelObj(int requestId, const OscType::OscChannelDescr& ch);
    QJsonObject createAnswerObj(int requestId, DevID deviceID, const QJsonObject &body = QJsonObject(), int error = 0);
    OscType::OscChannelDescr createChannelDescr(const OSC_CHANNEL &channel);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);

    long startStreamData(const DevID& devID, QVector<int> oscVars, const int &oscId);
    long getData(const DevID &devID, QVector<int> oscVars);
    long startHistoryData(const DevID &devID, QVector<int> oscVars, QDate historyDate, int step);

    void stopStreamData();
    void th_streamData();
    void th_streamHistoryData();

    OscType::OscHeader m_capturedOsc;
    QVector<int> m_capturedVars;
    int m_streamValCount = 0;
    bool m_streaming = false;
    OscHistoryService* m_historySrv;
    IOscDataService* m_dataSrv;
    QFuture<void> m_future;
};

#endif // OSCHANDLER_H
