#ifndef MAINWINDOWVM_H
#define MAINWINDOWVM_H

#include <QObject>

#include <QtWebSockets>

class MainWindowVM : public QObject
{
    Q_OBJECT

public:
    explicit MainWindowVM(QObject *parent = nullptr);

    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString paramIndex READ paramIndex WRITE setParamIndex NOTIFY paramIndexChanged)
    Q_PROPERTY(QString paramName READ paramName WRITE setParamName NOTIFY paramNameChanged)
    Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString paramValue READ paramValue  NOTIFY paramValueChanged)

    Q_INVOKABLE void start();
    Q_INVOKABLE void close();
    Q_INVOKABLE void receiveParamInfo();
    Q_INVOKABLE void receiveDeviceInfo();
    Q_INVOKABLE void receiveParamValues();

    QString deviceId() const;
    void setDeviceId(QString deviceId);

    QString paramIndex() const;
    void setParamIndex(QString paramIndex);
    void setParamValue(double value);

    QString paramName() const;
    QString deviceName() const;
    QString paramValue() const;

public slots:
    void setParamName(QString paramName);
    void setDeviceName(QString deviceName);

signals:
    void paramIndexChanged(QString paramIndex);
    void deviceIdChanged(QString deviceId);
    void dataReceived(QString msg);
    void closed();
    void connected();

    void paramNameChanged(QString paramName);
    void deviceNameChanged(QString deviceName);
    void paramValueChanged(QString paramValue);

private slots:
    void onConnected();
    void onDisconnected();

    void onTextMessageReceived(QString message);
    QJsonObject createCmd(QString name);
    QJsonObject createBody(QString name);

private:
    QString m_deviceId;
    QString m_paramIndex;
    QString m_paramName;
    QString m_deviceName;
    double m_paramValue = 0.0;

    QWebSocket m_webSocket;
};

// QML_DECLARE_TYPE(MainWindowVM);
#endif // MAINWINDOWVM_H
