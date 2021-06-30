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

    url.setPort(1237);
    m_streamWebSocket.open(QUrl(url));
}

void MainWindowVM::close()
{
    m_webSocket.close();
    m_streamWebSocket.close();
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
    req["cmd"] = createCmd("GET_PARAM_DATA");
    req["body"] = createBody("GET_PARAM_DATA");

    QJsonDocument doc(req);
//  QByteArray bytes = doc.toJson();
    QString strJson(doc.toJson(QJsonDocument::Compact));

    m_webSocket.sendTextMessage(strJson);
}

void MainWindowVM::streamParamValues()
{
    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["cmd"] = createCmd("STREAM_PARAM_DATA");
    req["body"] = createBody("STREAM_PARAM_DATA");

    QJsonDocument doc(req);
//  QByteArray bytes = doc.toJson();
    QString strJson(doc.toJson(QJsonDocument::Compact));

    m_perfomanceTimer.start();
    m_webSocket.sendTextMessage(strJson);
}

QJsonObject MainWindowVM::createCmd(QString name)
{
    QJsonObject res;
    if (name == "GET_PARAMS") {
        res["name"] = "param_header";
        res["type"] = "get";
    } else if (name == "GET_DEVICE") {
        res["name"] = "device_header";
        res["type"] = "get";
    } else if (name == "GET_PARAM_DATA") {
        res["name"] = "param_data";
        res["type"] = "get";
    } else if (name == "STREAM_PARAM_DATA") {
        res["name"] = "param_data";
        res["type"] = "open_stream";
    }

    return res;
}

QJsonObject MainWindowVM::createBody(QString name)
{
    QJsonObject res;
    if (name == "GET_PARAMS" ) {
        res["device_id"] = m_deviceId.toInt();
        res["param_id"] = m_paramIndex.toInt();
    } else if (name == "GET_PARAM_DATA" || name == "STREAM_PARAM_DATA") {
        res["device_id"] = m_deviceId.toInt();
        res["param_id"] = m_valueParamIndex.toInt();
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
    if (m_deviceId == deviceId) {
        return;
    }

    m_deviceId = deviceId;

    emit deviceIdChanged(m_deviceId);
}

QString MainWindowVM::paramIndex() const
{
    return m_paramIndex;
}

void MainWindowVM::setParamIndex(QString paramIndex)
{
    if (m_paramIndex == paramIndex) {
        return;
    }

    m_paramIndex = paramIndex;

    emit paramIndexChanged(m_paramIndex);
}

QString MainWindowVM::valueParamIndex() const
{
    return m_valueParamIndex;
}

void MainWindowVM::setValueParamIndex(QString arg)
{
    if (m_valueParamIndex == arg)
        return;

    m_valueParamIndex = arg;
    emit valueParamIndexChanged(m_valueParamIndex);
}

QString MainWindowVM::paramObjToString(const QJsonObject &obj)
{
    int deviceId = obj.value("device_id").toInt();
    int moduleId = obj.value("module_id").toInt();
    int paramId = obj.value("param_id").toInt();

    QString name = obj.value("name").toString();
    QString res = QString("Param: id = %1, name = %2, device id = %3, module id = %4")
            .arg(paramId)
            .arg(name)
            .arg(deviceId)
            .arg(moduleId);

    return res;
}

QString MainWindowVM::deviceObjToString(const QJsonObject &obj)
{
    int deviceId = obj.value("id").toInt();
    QString name = obj.value("name").toString();
    int modulesCount = obj.value("modules").toArray().count();
    QString res = QString("Device: id = %1, name = %2, modules count = %3")
            .arg(deviceId)
            .arg(name)
            .arg(modulesCount);

    return res;
}

QString MainWindowVM::paramValueObjToString(const QJsonObject &obj)
{
    int devId = obj.value("device_id").toInt();
    int paramId = obj.value("param_id").toInt();
    QJsonValue value = obj.value("value");
    double dval = value.toObject().value("value").toDouble();

    QString res = QString("Param: id = %1, device id = %2, value = %3")
            .arg(paramId)
            .arg(devId)
            .arg(dval);

    return res;
}

QString MainWindowVM::streamParamValueObjToString(const QJsonObject &obj)
{
    int devId = obj.value("d_id").toInt();
    int paramId = obj.value("p_id").toInt();
    QJsonValue value = obj.value("value");
    double dval = value.toObject().value("value").toDouble();

    QString res("");
    if (dval != -1) {
        res = QString("Param: id = %1, device id = %2, value = %3")
            .arg(paramId)
            .arg(devId)
            .arg(dval);

        int valueTime = value.toObject().value("time").toVariant().toLongLong();
        int currMSec = QDateTime::currentMSecsSinceEpoch();
        int valueActuality = currMSec - valueTime;
        if (valueActuality > 10) {
            res += QString("actuality = %1ms").arg(valueActuality);
        }

        m_valCounter++;
    } else {
        res = QString("Received %1 items per %2ms")
                .arg(m_valCounter)
                .arg(m_perfomanceTimer.elapsed());

        m_valCounter = 0;
    }

    return res;
}

void MainWindowVM::onConnected()
{
    QString msg = "WebSocket connected to " + m_webSocket.peerAddress().toString() + " : " + QString("%1").arg(m_webSocket.peerPort());

    emit dataReceived(msg);
    emit connected();

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onTextMessageReceived);

    connect(&m_streamWebSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onStreamTextMessageReceived);

}

void MainWindowVM::onDisconnected()
{
    QString msg = "WebSocket closed!";

    emit dataReceived(msg);
    emit closed();

    disconnect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onTextMessageReceived);

    disconnect(&m_streamWebSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onStreamTextMessageReceived);
}


void MainWindowVM::onTextMessageReceived(QString message)
{
    QJsonObject response = QJsonDocument::fromJson(message.toUtf8()).object();
    int reqId = response.value("request_id").toInt();

    QString output;
    if (reqId == PARAM_REQUEST_ID) {
        QJsonArray body = response.value("body").toArray();
        QJsonObject bodyObj = body.first().toObject();
        output = paramObjToString(bodyObj);
    } else if (reqId == DEVICE_REQUEST_ID) {
        QJsonArray body = response.value("body").toArray();
        QJsonObject bodyObj = body.first().toObject();
        output = deviceObjToString(bodyObj);
    } else if (reqId == PARAM_VALUE_REQUEST_ID) {
        QJsonObject body = response.value("body").toObject();
        output = paramValueObjToString(body);
    }

    emit dataReceived(output);
}

void MainWindowVM::onStreamTextMessageReceived(QString message)
{
    QJsonObject obj = QJsonDocument::fromJson(message.toUtf8()).object();
    QString output = streamParamValueObjToString(obj);

    emit dataReceived(output);
}
