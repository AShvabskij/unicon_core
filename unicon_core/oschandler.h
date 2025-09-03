#ifndef OSCHANDLER_H
#define OSCHANDLER_H

#include <QAtomicInt>

#include "basereqhandler.h"
#include "DDE_TOP.h"
#include "osc_types.h"
#include "oscdataservice.h"

#include "oschistoryservice.h"

class OscHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    OscHandler(IDDE_Dispatcher* , SysType sysType, IOscDataService* buffSrv);
    virtual int handle(const QJsonObject& request);
    void setService(OscHistoryService* s);
    virtual void handleClose();

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
    long convertHeader(const DDE_OSC_HEADER& header, OscType::OscHeader *res);
    long setHeader(const DevID& deviceID, const OscType::OscSettings &settings);

    QJsonObject createHeaderObj(int requestId, const OscType::OscHeader& header);
    QJsonObject createChannelDescr(int requestId, const OscType::OscChannelVar &chVar);
    QJsonObject createAnswerObj(int requestId, DevID deviceID, const QJsonObject &body = QJsonObject(), int error = 0);
    OscType::OscChannelVar createChannelVar(const OSC_VAR &var);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit);

    long startStreamData(const DevID& devID, QVector<int> oscVars);
    long getAllData(const DevID &devID, QVector<int> oscVars);
    long startHistoryData(const DevID &devID, QVector<int> oscVars, QDate historyDate, int step);

    void stopStreamData();
    void th_streamData();
    void th_streamHistoryData();
    void streamDataInternal(IOscDataService* dataService, DevID deviceID, qlonglong trig_time, QVector<int> capturedVars,
                            int initChunkSize, int maxChunkSize);

    OscType::OscHeader m_capturedOsc;
    QVector<int> m_capturedVars;
    int m_streamValCount = 0;
    QAtomicInt m_streamingFlag;
    OscHistoryService* m_historySrv;
    IOscDataService* m_dataSrv;
    QFuture<void> m_future;
};

#endif // OSCHANDLER_H
