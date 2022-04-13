#ifndef MAINWINDOWVM_H
#define MAINWINDOWVM_H

#include <QObject>

#include <QtWebSockets>
#include <qstring.h>

class MainWindowVM : public QObject
{
    Q_OBJECT

public:
    struct ElemId {
        ElemId() {}
        int moduleIndex = 0;
        int paramIndex = 0;
    };

    explicit MainWindowVM(QObject *parent = nullptr);

    Q_PROPERTY(int deviceId MEMBER m_deviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString deviceDescr MEMBER m_deviceDescr NOTIFY deviceDescrChanged)
    Q_PROPERTY(QString oscDescr MEMBER m_oscDescr NOTIFY oscDescrChanged)
    Q_PROPERTY(QString oscChannelValue MEMBER m_oscChannelValue NOTIFY oscChannelValueChanged)
    Q_PROPERTY(QString moduleDescr MEMBER m_moduleDescr NOTIFY moduleDescrChanged)
    Q_PROPERTY(QString paramInfo1 MEMBER m_paramInfo1 NOTIFY paramInfo1Changed)
    Q_PROPERTY(QString paramInfo2 MEMBER m_paramInfo2 NOTIFY paramInfo2Changed)
    Q_PROPERTY(QString paramValue1 MEMBER m_paramValue1 NOTIFY paramValue1Changed)
    Q_PROPERTY(QString paramValue2 MEMBER m_paramValue2 NOTIFY paramValue2Changed)

    Q_INVOKABLE void start();
    Q_INVOKABLE void close();
    Q_INVOKABLE void requestDeviceInfo();
    Q_INVOKABLE void requestOscInfo(int oscId);
    Q_INVOKABLE void requestChannelInfo(int oscId, int chNum);
    Q_INVOKABLE void requestParamInfo(int num, QString arg);
    Q_INVOKABLE void requestParamValues(QString arg);
    Q_INVOKABLE void startStreamParamValues(int num, QString arg);
    Q_INVOKABLE void stopStreamParamValues(QString arg);
    Q_INVOKABLE void startOscParamValues(int oscId, int chNum);
    Q_INVOKABLE void stopOscParamValues(QString oscId);
    Q_INVOKABLE void changeParamValue(int num, QString paramArg, QVariant paramValue);

    void setDeviceDescr(QString arg);
    void setOscDescr(QString arg);
    void setModuleDescr(QString arg);
    void setParamInfo(QString arg);
    void setParamValue(QVariant val, double time);

public slots:
    QString paramObjToString(const QJsonObject &obj);
    QString deviceObjToString(const QJsonObject &obj);
    QString oscObjToString(const QJsonObject &obj);
    QString oscChannelObjToString(const QJsonObject &obj);
    QString paramValueObjToString(const QJsonObject &obj);
    QString streamParamValueObjToString(const QJsonObject &obj);
    QString oscDataObjToString(const QJsonObject &obj);

signals:
    void paramInfo1Changed(QString arg);
    void paramInfo2Changed(QString arg);
    void paramValue1Changed(QString arg);
    void paramValue2Changed(QString arg);
    void deviceIdChanged(QString deviceId);
    void deviceDescrChanged(QString descr);
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
    QJsonObject createParamCmdBody(QString name, int deviceId, ElemId param = ElemId(), QVariant value = QVariant());
    QJsonObject createDeviceCmdBody(QString cmd, int deviceId);
    QJsonObject createOscCmdBody(QString cmd, int deviceId, int oscId, int chNum = -1);

private:
    ElemId parse(QString arg) const;
    void doProccessDataReceived(QJsonObject data);
    void doProccessStreamDataReceived(QJsonObject data);
    void sendRequest(QJsonObject req, bool checkPerformance = false);

    int m_deviceId;
    QString m_deviceDescr;
    QString m_oscDescr;
    QString m_oscChannelValue;
    QString m_moduleDescr;

    QString m_paramInfo1;
    QString m_paramInfo2;
    QString m_paramValue1;
    QString m_paramValue2;

    QWebSocket m_webSocket;
    QWebSocket m_streamWebSocket;

    QElapsedTimer m_perfomanceTimer;
    int m_valCounter = 0;

    QMap<int/*num*/, ElemId> m_params;
    int m_currParamNum = 0;
};

// QML_DECLARE_TYPE(MainWindowVM);
#endif // MAINWINDOWVM_H
