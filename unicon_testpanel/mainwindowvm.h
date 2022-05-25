#ifndef MAINWINDOWVM_H
#define MAINWINDOWVM_H

#include <QObject>

#include <QtWebSockets>
#include <qstring.h>

enum SysType
{
    Undefined = 0,
    FILE_IO,
    UAV_CAN,
    CAN_OPEN,
    MOD_BUS,
    MOD_BUS_FO
};

class MainWindowVM : public QObject
{
    Q_OBJECT

public:
    struct CompositeId {
        CompositeId() {}
        int moduleId = 0;
        int paramId = 0;
    };

    explicit MainWindowVM(QObject *parent = nullptr);

    Q_PROPERTY(QString host MEMBER m_host NOTIFY hostChanged)

    Q_PROPERTY(int deviceId MEMBER m_deviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString deviceDescr MEMBER m_deviceDescr NOTIFY deviceDescrChanged)
    Q_PROPERTY(QString oscDescr MEMBER m_oscDescr NOTIFY oscDescrChanged)
    Q_PROPERTY(QString oscChannelValue MEMBER m_oscChannelValue NOTIFY oscChannelValueChanged)
    Q_PROPERTY(QString moduleDescr MEMBER m_moduleDescr NOTIFY moduleDescrChanged)

    Q_PROPERTY(QString paramId1 MEMBER m_paramComposId1)
    Q_PROPERTY(QString paramId2 MEMBER m_paramComposId2)
    Q_PROPERTY(QString paramInfo1 MEMBER m_paramInfo1 NOTIFY paramInfo1Changed)
    Q_PROPERTY(QString paramInfo2 MEMBER m_paramInfo2 NOTIFY paramInfo2Changed)
    Q_PROPERTY(QString paramValue1 MEMBER m_paramValue1 NOTIFY paramValue1Changed)
    Q_PROPERTY(QString paramValue2 MEMBER m_paramValue2 NOTIFY paramValue2Changed)
    Q_PROPERTY(QString paramValueInfo1 MEMBER m_paramValueInfo1 NOTIFY valueInfo1Changed)
    Q_PROPERTY(QString paramValueInfo2 MEMBER m_paramValueInfo2 NOTIFY valueInfo2Changed)

    Q_INVOKABLE void start();
    Q_INVOKABLE void close();
    Q_INVOKABLE void requestSystemInfo();
    Q_INVOKABLE void requestDeviceInfo(int moduleId = 0);
    Q_INVOKABLE void requestOscInfo(int oscId);
    Q_INVOKABLE void requestChannelInfo(int oscId, int chNum);
    Q_INVOKABLE void requestParamInfo(QString arg);
    Q_INVOKABLE void requestParamValues(QString arg);
    Q_INVOKABLE void startStreamParamValues(QString arg);
    Q_INVOKABLE void stopStreamParamValues(QString arg);
    Q_INVOKABLE void startOscParamValues(int oscId, int chNum);
    Q_INVOKABLE void stopOscParamValues(QString oscId);
    Q_INVOKABLE void changeParamValue(QString paramArg, QVariant paramValue);

    void setDeviceDescr(const QJsonObject &obj);
    void setSystemInfo(QJsonArray objects);
    void setOscDescr(const QJsonObject &obj);
    void setModuleDescr(const QJsonObject &obj);
    void setParamInfo(const QJsonObject &obj);
    void setParamValue(const QJsonObject& obj, int errorCode = 0);
    void setStreamParamValue(const QJsonObject& obj);
    void setOscChannelInfo(const QJsonObject &obj);
    void setOscChannelValue(const QJsonObject &obj);

signals:
    void paramInfo1Changed(QString arg);
    void paramInfo2Changed(QString arg);
    void valueInfo1Changed(QString arg);
    void valueInfo2Changed(QString arg);
    void paramValue1Changed(QString arg);
    void paramValue2Changed(QString arg);
    void hostChanged();
    void deviceIdChanged();
    void deviceDescrChanged();
    void oscDescrChanged(QString descr);
    void oscChannelValueChanged(QString descr);
    void moduleDescrChanged(QString descr);

    void dataReceived(QString msg);
    void dataRequest(QString msg);
    void closed();
    void connected();

private slots:
    void onConnected();
    void onDisconnected();

    void onTextMessageReceived(QString message);
    void onBinaryMessageReceived(QByteArray message);
    void onStreamTextMessageReceived(QString message);
    void onStreamBinaryMessageReceived(QByteArray message);
    QJsonObject createCmd(QString name);
    QJsonObject createParamCmdBody(QString name, int deviceId, CompositeId elemId = CompositeId(), QVariant value = QVariant());
    QJsonObject createDeviceCmdBody(QString cmd, int deviceId, int moduleId = 0);
    QJsonObject createOscCmdBody(QString cmd, int deviceId, int oscId, int chNum = -1);

private:
    CompositeId parse(QString arg) const;
    void doProccessDataReceived(QJsonObject data);
    void doProccessStreamDataReceived(QJsonObject data);
    void sendRequest(QJsonObject req, bool checkPerformance = false);
    void emitParamValue(int moduleId, int paramId, QString sVal, QString info);
    QString sysTypeToString(SysType sysType);

    QString m_host = "";

    int m_deviceId = 0;
    SysType m_currSysType = SysType::Undefined;

    QString m_deviceDescr;
    QString m_oscDescr;
    QString m_oscChannelValue;
    QString m_moduleDescr;

    QString m_paramComposId1;
    QString m_paramComposId2;
    QString m_paramInfo1;
    QString m_paramInfo2;
    QString m_paramValue1;
    QString m_paramValue2;
    QString m_paramValueInfo1;
    QString m_paramValueInfo2;

    QWebSocket m_webSocket;
    QWebSocket m_streamWebSocket;

    QElapsedTimer m_perfomanceTimer;
    int m_valCounter = 0;
};

// QML_DECLARE_TYPE(MainWindowVM);
#endif // MAINWINDOWVM_H
