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
    Q_PROPERTY(QString valueParamIndex READ valueParamIndex WRITE setValueParamIndex NOTIFY valueParamIndexChanged)

    Q_INVOKABLE void start();
    Q_INVOKABLE void close();
    Q_INVOKABLE void receiveParamInfo();
    Q_INVOKABLE void receiveDeviceInfo();
    Q_INVOKABLE void receiveParamValues();
    Q_INVOKABLE void streamParamValues();
    Q_INVOKABLE void oscParamValues();

    QString deviceId() const;
    void setDeviceId(QString deviceId);

    QString paramIndex() const;
    void setParamIndex(QString paramIndex);
    QString valueParamIndex() const;
    void setValueParamIndex(QString arg);

public slots:
    QString paramObjToString(const QJsonObject &obj);
    QString deviceObjToString(const QJsonObject &obj);
    QString paramValueObjToString(const QJsonObject &obj);
    QString streamParamValueObjToString(const QJsonObject &obj);
    QString oscDataObjToString(const QJsonObject &obj);

signals:
    void paramIndexChanged(QString paramIndex);
    void valueParamIndexChanged(QString arg);
    void deviceIdChanged(QString deviceId);

    void dataReceived(QString msg);
    void closed();
    void connected();

private slots:
    void onConnected();
    void onDisconnected();


    void onTextMessageReceived(QString message);
    void onStreamTextMessageReceived(QString message);
    QJsonObject createCmd(QString name);
    QJsonObject createBody(QString name);

private:
    QString m_deviceId;
    QString m_paramIndex;
    QString m_valueParamIndex;

    QWebSocket m_webSocket;
    QWebSocket m_streamWebSocket;

    QElapsedTimer m_perfomanceTimer;
    int m_valCounter = 0;

};

// QML_DECLARE_TYPE(MainWindowVM);
#endif // MAINWINDOWVM_H
