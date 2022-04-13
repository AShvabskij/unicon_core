#include "mainwindowvm.h"

const int DEVICE_REQUEST_ID = 1;
const int OSC_REQUEST_ID = 10;
const int OSC_CHANNEL_REQUEST_ID = 11;
const int OSC_DATA_REQUEST_ID = 12;
const int PARAM_REQUEST_ID = 20;
const int PARAM_VALUE_REQUEST_ID = 21;
const int PARAM_VALUE_SET_ID = 22;

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
    m_webSocket.setReadBufferSize(10000);

    url.setPort(1237);
    m_streamWebSocket.open(QUrl(url));
}

void MainWindowVM::close()
{
    m_webSocket.close();
    m_streamWebSocket.close();
}

void MainWindowVM::requestParamInfo(int num, QString arg)
{
    QJsonObject req;
    req["request_id"] = PARAM_REQUEST_ID;
    req["cmd"] = createCmd("GET_PARAM_HEADER");

    m_params[num] = parse(arg);
    req["body"] = createParamCmdBody("GET_PARAM_HEADER", m_deviceId, m_params[num]);

    QJsonDocument doc(req);
//  QByteArray bytes = doc.toJson();
    QString strJson(doc.toJson(QJsonDocument::Compact));

    m_currParamNum = num;
    m_webSocket.sendTextMessage(strJson);
}

void MainWindowVM::requestDeviceInfo()
{
    QJsonObject req;
    req["request_id"] = DEVICE_REQUEST_ID;
    req["cmd"] = createCmd("GET_DEVICE_HEADER");
    req["body"] = createDeviceCmdBody("GET_DEVICE_HEADER", m_deviceId);

    sendRequest(req);
}

void MainWindowVM::requestOscInfo(int oscId)
{
    int deviceId = oscId;

    QJsonObject req;
    req["request_id"] = OSC_REQUEST_ID;
    req["cmd"] = createCmd("GET_OSC_HEADER");
    req["body"] = createOscCmdBody("GET_OSC_HEADER", deviceId, oscId);

    sendRequest(req, true);
}

void MainWindowVM::requestChannelInfo(int oscId, int chNum)
{
    int deviceId = oscId;

    QJsonObject req;
    req["request_id"] = OSC_CHANNEL_REQUEST_ID;
    req["cmd"] = createCmd("GET_OSC_CHANNEL");
    req["body"] = createOscCmdBody("GET_OSC_CHANNEL", deviceId, oscId, chNum);

    sendRequest(req, true);
}

void MainWindowVM::requestParamValues(QString arg)
{
    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["cmd"] = createCmd("GET_PARAM_DATA");

    ElemId elem = parse(arg);
    req["body"] = createParamCmdBody("GET_PARAM_DATA", m_deviceId, elem);

    sendRequest(req);
}

void MainWindowVM::changeParamValue(int num, QString paramArg, QVariant paramValue)
{
    QJsonObject req;
    req["request_id"] = PARAM_VALUE_SET_ID;
    req["cmd"] = createCmd("SET_PARAM_DATA");

    ElemId elem = parse(paramArg);
    req["body"] = createParamCmdBody("SET_PARAM_DATA", m_deviceId, elem, paramValue);

    sendRequest(req);
}

void MainWindowVM::startStreamParamValues(int num, QString arg)
{
    m_currParamNum = num;
    ElemId elem = parse(arg);

    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["cmd"] = createCmd("STREAM_PARAM_DATA");
    req["body"] = createParamCmdBody("STREAM_PARAM_DATA", m_deviceId, elem);

    sendRequest(req, true);
}

void MainWindowVM::stopStreamParamValues(QString paramArg)
{
    ElemId elem = parse(paramArg);

    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["cmd"] = createCmd("STREAM_STOP_PARAM_DATA");
    req["body"] = createParamCmdBody("STREAM_PARAM_DATA", m_deviceId, elem);

    sendRequest(req);
}

void MainWindowVM::startOscParamValues(int oscId, int chNum)
{
    QJsonObject req;
    req["request_id"] = OSC_DATA_REQUEST_ID;
    req["cmd"] = createCmd("OSC_PARAM_DATA");
    req["body"] = createOscCmdBody("OSC_PARAM_DATA", oscId, oscId, chNum);

    sendRequest(req, true);
}

void MainWindowVM::stopOscParamValues(QString oscId)
{
    QJsonObject req;
    req["request_id"] = OSC_DATA_REQUEST_ID;
    req["cmd"] = createCmd("OSC_STOP_PARAM_DATA");
    req["body"] = createOscCmdBody("OSC_PARAM_DATA", oscId.toInt(), oscId.toInt());

    sendRequest(req, true);
}

void MainWindowVM::setDeviceDescr(QString arg)
{
    m_deviceDescr = arg;
    emit deviceDescrChanged(arg);
}

void MainWindowVM::setOscDescr(QString arg)
{
    m_oscDescr = arg;
    emit oscDescrChanged(arg);
}

void MainWindowVM::setModuleDescr(QString arg)
{
    m_moduleDescr = arg;
    emit moduleDescrChanged(arg);
}

QJsonObject MainWindowVM::createCmd(QString name)
{
    QJsonObject res;
    if (name == "GET_PARAM_HEADER") {
        res["name"] = "param_header";
        res["type"] = "get";
    } else if (name == "GET_DEVICE_HEADER") {
        res["name"] = "device_header";
        res["type"] = "get";
    } else if (name == "GET_PARAM_DATA") {
        res["name"] = "param_data";
        res["type"] = "get";
    } else if (name == "SET_PARAM_DATA") {
        res["name"] = "param_data";
        res["type"] = "set";
    } else if (name == "STREAM_PARAM_DATA") {
        res["name"] = "param_data";
        res["type"] = "open_stream";
    } else if (name == "STREAM_STOP_PARAM_DATA") {
        res["name"] = "param_data";
        res["type"] = "close_stream";
    } else if (name == "GET_OSC_HEADER") {
        res["name"] = "osc_header";
        res["type"] = "get";
    } else if (name == "GET_OSC_CHANNEL") {
        res["name"] = "osc_channel";
        res["type"] = "get";
    } else if (name == "OSC_PARAM_DATA") {
        res["name"] = "osc_data";
        res["type"] = "open_stream";
    }  else if (name == "OSC_STOP_PARAM_DATA") {
        res["name"] = "osc_data";
        res["type"] = "close_stream";
    }

    return res;
}

QJsonObject MainWindowVM::createParamCmdBody(QString name, int deviceId, ElemId paramId, QVariant value)
{
    QJsonObject res;
    if (name == "GET_PARAM_HEADER" ) {
        res["device_id"] = deviceId;
        res["module_id"] = paramId.moduleIndex;
        res["param_id"] = paramId.paramIndex;
    } else if (name == "GET_PARAM_DATA" || name == "STREAM_PARAM_DATA") {
        res["device_id"] = deviceId;
        res["module_id"] = paramId.moduleIndex;
        res["param_id"] = paramId.paramIndex;
    } else if (name == "SET_PARAM_DATA") {
        res["device_id"] = deviceId;
        res["module_id"] = paramId.moduleIndex;
        res["param_id"] = paramId.paramIndex;
        res["value"] = QJsonValue::fromVariant(value);
    }

    return res;
}

QJsonObject MainWindowVM::createDeviceCmdBody(QString cmd, int deviceId)
{
    QJsonObject res;
    if (cmd == "GET_DEVICE_HEADER") {
        res["device_id"] = deviceId;
    }

    return res;
}

QJsonObject MainWindowVM::createOscCmdBody(QString cmd, int deviceId, int oscId, int chNum)
{
    QJsonArray channels;
    if (chNum > 0) {
        channels.append(QJsonValue(chNum));
    }

    QJsonObject res;
    if (cmd == "GET_OSC_HEADER") {
        res["device_id"] = deviceId;
        res["osc_id"] = oscId;
    } else if (cmd == "GET_OSC_CHANNEL") {
        res["device_id"] = deviceId;
        res["osc_id"] = oscId;
        res["channel_num"] = chNum;
    } else if (cmd == "OSC_PARAM_DATA") {
        res["device_id"] = deviceId;
        res["osc_id"] = oscId;
        if (!channels.isEmpty()) {
            res["channels"] = channels;
        }
    }

    return res;
}

MainWindowVM::ElemId MainWindowVM::parse(QString arg) const
{
    if (arg.isEmpty()) return ElemId();

    QStringList args = arg.split(".");

    if (args.count()!=2) return ElemId();

    ElemId res;
    res.moduleIndex = args.value(0).toInt();
    res.paramIndex = args.value(1).toInt();

    return res;
}

void MainWindowVM::setParamInfo(QString arg)
{
    if (m_currParamNum == 1) {
        m_paramInfo1 = arg;
        emit paramInfo1Changed(m_paramInfo1);
    } else {
        m_paramInfo2 = arg;
        emit paramInfo2Changed(m_paramInfo2);
    }
}

void MainWindowVM::setParamValue(QVariant val, double time)
{
    QString svalue = val.canConvert(QMetaType::Float) ? QString::number(val.toFloat(), 'f', 2) : val.toString();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(time);
    svalue = svalue.leftJustified(8) + " " + dt.time().toString("mm:ss.zzz"); // toString(Qt::ISODateWithMs);

    if (m_currParamNum == 1) {
        m_paramValue1 = svalue;
        emit paramValue1Changed(svalue);
    } else {
        m_paramValue2 = svalue;
        emit paramValue2Changed(svalue);
    }
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

QString MainWindowVM::oscObjToString(const QJsonObject &obj)
{
    int deviceId = obj.value("device_id").toInt();
    QString name = obj.value("name").toString();
    int analogCount = obj.value("channels").toArray().count();
    int descreteCount = obj.value("discretes").toArray().count();
    QString res = QString("Osc: device id = %1, name = %2, analog = %3  discrete = %4 channels")
            .arg(deviceId)
            .arg(name)
            .arg(analogCount)
            .arg(descreteCount);

    return res;
}

QString MainWindowVM::oscChannelObjToString(const QJsonObject &obj)
{
    int chNum = obj.value("num").toInt();
    QString name = obj.value("name").toString();
    int varId = obj.value("var_id").toInt();
    double scale = obj.value("scale").toDouble();
    QString analogable = obj.value("isDiscrete").toBool() ? "discrete" : "analog";

    QString res = QString("Channel: num = %1, name = %2, var id = %3, %4, scale = %5")
            .arg(chNum)
            .arg(name)
            .arg(varId)
            .arg(analogable)
            .arg(scale);

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
            res += QString(", actuality = %1ms").arg(valueActuality);
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

QString MainWindowVM::oscDataObjToString(const QJsonObject &obj)
{
    QJsonArray values = obj.value("values").toArray();
    QStringList dvalList;
    for (const QJsonValueRef& el : values) {
        QJsonArray valBuffer = el.toArray();
        for (int i = 0; i < 10/*valBuffer.size()*/; ++i) {
            dvalList << QString("%1").arg(valBuffer[i].toDouble());
        }

//        double dval = !valBuffer.isEmpty() ? valBuffer[0].toDouble() : -1;
//        dvalList << QString("%1").arg(dval);
    }

    QString res("");
    res = dvalList.join(" ");

    return res;
}

void MainWindowVM::onConnected()
{
    QString msg = "WebSocket connected to " + m_webSocket.peerAddress().toString() + " : " + QString("%1").arg(m_webSocket.peerPort());

    emit dataReceived(msg);
    emit connected();

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onTextMessageReceived);

    connect(&m_webSocket, &QWebSocket::binaryMessageReceived,
            this, &MainWindowVM::onBinaryMessageReceived);

    connect(&m_streamWebSocket, &QWebSocket::textMessageReceived,
            this, &MainWindowVM::onStreamTextMessageReceived);

    connect(&m_streamWebSocket, &QWebSocket::binaryMessageReceived,
            this, &MainWindowVM::onStreamBinaryMessageReceived);
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
    QJsonObject dataObj = QJsonDocument::fromJson(message.toUtf8()).object();
    doProccessDataReceived(dataObj);
}

void MainWindowVM::onBinaryMessageReceived(QByteArray message)
{
    QCborValue v = QCborValue::fromCbor(message);
    QJsonObject dataObj = v.toJsonValue().toObject();
    doProccessDataReceived(dataObj);
}

void MainWindowVM::doProccessDataReceived(QJsonObject data)
{
    int reqId = data.value("request_id").toInt();

    QString output;

    QJsonObject bodyObj;
    if (reqId == PARAM_REQUEST_ID) {
        QJsonArray body = data.value("body").toArray();
        bodyObj = body.first().toObject();
        output = paramObjToString(bodyObj);
        setParamInfo(output);

    } else if (reqId == DEVICE_REQUEST_ID) {
        QJsonArray body = data.value("body").toArray();
        bodyObj = body.first().toObject();
        output = deviceObjToString(bodyObj);
        setDeviceDescr(output);
    } else if (reqId == OSC_REQUEST_ID) {
        bodyObj = data.value("body").toObject();
        output = oscObjToString(bodyObj);
        setOscDescr(output);
    } else if (reqId == OSC_CHANNEL_REQUEST_ID) {
        bodyObj = data.value("body").toObject();
        output = oscChannelObjToString(bodyObj);
        m_oscChannelValue = output;
        emit oscChannelValueChanged(m_oscChannelValue);
    } else if (reqId == PARAM_VALUE_REQUEST_ID || reqId == PARAM_VALUE_SET_ID) {
        bodyObj = data.value("body").toObject();
        output = paramValueObjToString(bodyObj);

        QJsonObject valueObj = bodyObj.value("value").toObject();
        QVariant value = valueObj.value("value").toVariant();
        double time = valueObj.value("time").toDouble();

        setParamValue(value, time);
    }

    QJsonDocument doc(bodyObj);
    QString strObj(doc.toJson(QJsonDocument::Indented));
    emit dataReceived(strObj);
}

void MainWindowVM::onStreamTextMessageReceived(QString message)
{
    QJsonObject obj = QJsonDocument::fromJson(message.toUtf8()).object();
    doProccessStreamDataReceived(obj);
}

void MainWindowVM::onStreamBinaryMessageReceived(QByteArray message)
{
    QCborValue v = QCborValue::fromCbor(message);
    QJsonObject obj = v.toJsonValue().toObject();
    doProccessStreamDataReceived(obj);
}

void MainWindowVM::doProccessStreamDataReceived(QJsonObject data)
{
    QString output("");
    if (data.contains("values")) {
        m_oscChannelValue = oscDataObjToString(data);
        output = m_oscChannelValue;
        emit oscChannelValueChanged(m_oscChannelValue);
    } else {
        output = streamParamValueObjToString(data);
        QJsonObject valueObj = data.value("value").toObject();
        double value = valueObj.value("value").toDouble();
        double time = valueObj.value("time").toDouble();

        setParamValue(value, time);
    }

    QTextStream(stdout) << output << "\n" ;
}

void MainWindowVM::sendRequest(QJsonObject req, bool checkPerformance)
{
    QJsonDocument doc(req);
    QString strReq(doc.toJson(QJsonDocument::Compact));

    if (checkPerformance) {
        m_perfomanceTimer.start();
    }

    emit dataRequest(strReq);
    m_webSocket.sendTextMessage(strReq);
}
