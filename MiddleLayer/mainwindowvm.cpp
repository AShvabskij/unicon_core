#include "mainwindowvm.h"

const int DEVICE_REQUEST_ID = 1;
const int PARAM_REQUEST_ID = 2;
const int PARAM_VALUE_REQUEST_ID = 3;

MainWindowVM::MainWindowVM(QObject* parent) : QObject(parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &MainWindowVM::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &MainWindowVM::onDisconnected);

}

void MainWindowVM::start()
{
    QString host = "127.0.0.1";
    QUrl url;
    url.setHost("127.0.0.1");
    url.setScheme("ws");
    url.setPort(1235);

    m_webSocket.open(QUrl(url));
}

void MainWindowVM::close()
{
    m_webSocket.close();
}

void MainWindowVM::receiveParamInfo()
{
    QJsonObject req;
    req["request_id"] = PARAM_REQUEST_ID;
    req["cmd"] = createCmd("GET_PARAMS");
    req["body"] = createBody("GET_PARAMS");

    QJsonDocument doc(req);
//  QByteArray bytes = doc.toJson();
    QString strJson(doc.toJson(QJsonDocument::Compact));

    m_webSocket.sendTextMessage(strJson);
}

void MainWindowVM::receiveDeviceInfo()
{
    QJsonObject req;
    req["request_id"] = DEVICE_REQUEST_ID;
    req["cmd"] = createCmd("GET_DEVICE");
    req["body"] = createBody("GET_DEVICE");

    QString strJson(QJsonDocument(req).toJson(QJsonDocument::Compact));

    m_webSocket.sendTextMessage(strJson);
}

void MainWindowVM::receiveParamValues()
{
    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["cmd"] = createCmd("GET_PARAMS_DATA");
    req["body"] = createBody("GET_PARAMS_DATA");

    QJsonDocument doc(req);
//  QByteArray bytes = doc.toJson();
    QString strJson(doc.toJson(QJsonDocument::Compact));

    m_webSocket.sendTextMessage(strJson);
}

QJsonObject MainWindowVM::createCmd(QString name)
{
    QJsonObject res;
    if (name == "GET_PARAMS") {
        res["name"] = "param";
        res["type"] = "get";
    } else if (name == "GET_DEVICE") {
        res["name"] = "device";
        res["type"] = "get";
    } else if (name == "GET_PARAMS_DATA") {
        res["name"] = "param_data";
        res["type"] = "get";
    }

    return res;
}

QJsonObject MainWindowVM::createBody(QString name)
{
    QJsonObject res;
    if (name == "GET_PARAMS" || name == "GET_PARAMS_DATA") {
        res["device_id"] = m_deviceId.toInt();
        res["index"] = m_paramIndex.toInt();
    } else if (name == "GET_DEVICE") {
        res["device_id"] = m_deviceId.toInt();
    }

    return res;
}

QString MainWindowVM::deviceId() const
{
    return m_deviceId;
}

void MainWindowVM::setDeviceId(QString deviceId)
{
    if (m_deviceId == deviceId)
        return;

    m_deviceId = deviceId;
    emit deviceIdChanged(m_deviceId);
}

QString MainWindowVM::paramIndex() const
{
    return m_paramIndex;
}

void MainWindowVM::setParamIndex(QString paramIndex)
{
    if (m_paramIndex == paramIndex)
        return;

    m_paramIndex = paramIndex;
    emit paramIndexChanged(m_paramIndex);
}

QString MainWindowVM::paramName() const
{
    return m_paramName;
}

QString MainWindowVM::deviceName() const
{
    return m_deviceName;
}

QString MainWindowVM::paramValue() const
{
    return QString::number(m_paramValue);
}

void MainWindowVM::setParamName(QString paramName)
{
    if (m_paramName == paramName)
        return;

    m_paramName = paramName;
    emit paramNameChanged(m_paramName);
}

void MainWindowVM::setDeviceName(QString deviceName)
{
    if (m_deviceName == deviceName)
        return;

    m_deviceName = deviceName;
    emit deviceNameChanged(m_deviceName);
}

void MainWindowVM::setParamValue(double value)
{
    if (m_paramValue == value)
        return;

    m_paramValue = value;
    emit paramValueChanged(QString::number(m_paramValue));
}

void MainWindowVM::onConnected()
{
    QString msg = "WebSocket connected to " + m_webSocket.peerAddress().toString() + " : " + QString("%1").arg(m_webSocket.peerPort());
    emit dataReceived(msg);
    emit connected();

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onTextMessageReceived);

}

void MainWindowVM::onDisconnected()
{
    QString msg = "WebSocket closed!";
    emit dataReceived(msg);
    emit closed();

    disconnect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onTextMessageReceived);
}

void MainWindowVM::onTextMessageReceived(QString message)
{
    emit dataReceived(message);

    QJsonObject response = QJsonDocument::fromJson(message.toUtf8()).object();
    int reqId = response.value("request_id").toInt();
    QJsonObject cmdBody = response.value("body").toObject();
    if (cmdBody.isEmpty()) {
        return ;
    }

    QString name = cmdBody.value("name").toString();
    if (reqId == PARAM_REQUEST_ID) {
        setParamName(name);
    } else if (reqId == DEVICE_REQUEST_ID) {
        setDeviceName(name);
    } else if (reqId == PARAM_VALUE_REQUEST_ID) {
        QJsonValue value = cmdBody.value("values").toArray().first();
        double dval = value.toObject().value("value").toDouble();
        setParamValue(dval);
    }
}
