#include "mainwindowvm.h"

const int SYSTEM_REQUEST_ID = 1;
const int DEVICE_REQUEST_ID = 2;
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
    QUrl url;

    url.setHost(m_host);
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

void MainWindowVM::requestParamInfo(QString arg)
{
    QJsonObject req;
    req["request_id"] = PARAM_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("GET_PARAM_HEADER");

    CompositeId paramId = parse(arg);
    req["body"] = createParamCmdBody("GET_PARAM_HEADER", m_deviceId, paramId);

    sendRequest(req);
}

void MainWindowVM::requestSystemInfo()
{
    QJsonObject req;
    req["request_id"] = SYSTEM_REQUEST_ID;
    req["cmd"] = createCmd("GET_DEVICE_HEADER");
    req["body"] = createDeviceCmdBody("GET_DEVICE_HEADER", 0);

    sendRequest(req);
}

void MainWindowVM::requestDeviceInfo(int moduleId)
{
    if (m_deviceId == 0) {
        requestSystemInfo();
        return;
    }

    QJsonObject req;
    req["request_id"] = DEVICE_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("GET_DEVICE_HEADER");
    req["body"] = createDeviceCmdBody("GET_DEVICE_HEADER", m_deviceId, moduleId);

    sendRequest(req);
}

void MainWindowVM::requestOscInfo(int oscId)
{
    int deviceId = oscId;

    QJsonObject req;
    req["request_id"] = OSC_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("GET_OSC_HEADER");
    req["body"] = createOscCmdBody("GET_OSC_HEADER", deviceId, oscId);

    sendRequest(req, true);
}

void MainWindowVM::requestChannelInfo(int oscId, int chNum)
{
    int deviceId = oscId;

    QJsonObject req;
    req["request_id"] = OSC_CHANNEL_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("GET_OSC_CHANNEL");
    req["body"] = createOscCmdBody("GET_OSC_CHANNEL", deviceId, oscId, chNum);

    sendRequest(req);
}

void MainWindowVM::requestParamValues(QString arg)
{
    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("GET_PARAM_DATA");

    CompositeId elem = parse(arg);
    req["body"] = createParamCmdBody("GET_PARAM_DATA", m_deviceId, elem);

    sendRequest(req);
}

void MainWindowVM::changeParamValue(QString paramArg, QVariant paramValue)
{
    QJsonObject req;
    req["request_id"] = PARAM_VALUE_SET_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("SET_PARAM_DATA");

    CompositeId elem = parse(paramArg);
    req["body"] = createParamCmdBody("SET_PARAM_DATA", m_deviceId, elem, paramValue);

    sendRequest(req);
}

void MainWindowVM::startStreamParamValues(QString arg)
{
    CompositeId elem = parse(arg);

    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("STREAM_PARAM_DATA");
    req["body"] = createParamCmdBody("STREAM_PARAM_DATA", m_deviceId, elem);

    sendRequest(req, true);
}

void MainWindowVM::stopStreamParamValues(QString paramArg)
{
    CompositeId elem = parse(paramArg);

    QJsonObject req;
    req["request_id"] = PARAM_VALUE_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("STREAM_STOP_PARAM_DATA");
    req["body"] = createParamCmdBody("STREAM_PARAM_DATA", m_deviceId, elem);

    sendRequest(req);
}

void MainWindowVM::startOscParamValues(int oscId, int varId)
{
    QJsonObject req;
    req["request_id"] = OSC_DATA_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("OSC_PARAM_DATA");
    req["body"] = createOscCmdBody("OSC_PARAM_DATA", oscId, oscId, varId);

    sendRequest(req, true);
}

void MainWindowVM::stopOscParamValues(QString oscId)
{
    QJsonObject req;
    req["request_id"] = OSC_DATA_REQUEST_ID;
    req["sys_type"] = m_currSysType;
    req["cmd"] = createCmd("OSC_STOP_PARAM_DATA");
    req["body"] = createOscCmdBody("OSC_PARAM_DATA", oscId.toInt(), oscId.toInt());

    sendRequest(req);
}

void MainWindowVM::setSystemInfo(QJsonArray objects)
{
/*
    QString res = QString("devices count = %1")
            .arg(objects.count());


    m_systemInfo = res;
    emit systemInfoChanged();
*/

    if (m_deviceId == 0) {
        QJsonObject deviceInfo = objects.first().toObject();
        setDeviceDescr(deviceInfo);
    }
}

void MainWindowVM::setDeviceDescr(const QJsonObject &obj)
{
    int deviceId = obj.value("id").toInt();
    QString name = obj.value("name").toString();
    int modulesCount = obj.value("modules").toArray().count();
    int sysType = obj.value("channel").toInt();
    QString descr = obj.value("desc").toString();

    QString output = QString("Device: id = %1, name = %2, modules = %3, system type = %4")
            .arg(deviceId)
            .arg(name)
            .arg(modulesCount)
            .arg(sysTypeToString((SysType)sysType));

    m_deviceDescr = output;
    emit deviceDescrChanged();

    m_deviceId = deviceId;
    m_currSysType = (SysType)sysType;
    emit deviceIdChanged();
}

void MainWindowVM::setOscDescr(const QJsonObject &obj)
{
    int deviceId = obj.value("device_id").toInt();
    QString name = obj.value("name").toString();
    int analogCount = obj.value("analog_channels").toArray().count();
    int descreteCount = obj.value("discrete_channels").toArray().count();

    QString output = QString("Osc: device id = %1, name = %2, analog = %3  discrete = %4 channels")
            .arg(deviceId)
            .arg(name)
            .arg(analogCount)
            .arg(descreteCount);

    m_oscDescr = output;
    emit oscDescrChanged(output);
}

void MainWindowVM::setModuleDescr(const QJsonObject &obj)
{
    int moduleId = obj.value("id").toInt();
    QString name = obj.value("name").toString();
    QString descr = obj.value("desc").toString();
    int paramsCount = obj.value("params").toArray().count();
    QString output = QString("Module: id = %1, name = %2, params = %3, descr = %4")
            .arg(moduleId)
            .arg(name)
            .arg(paramsCount)
            .arg(descr);


    m_moduleDescr = output;
    emit moduleDescrChanged(output);
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

QJsonObject MainWindowVM::createParamCmdBody(QString name, int deviceId, CompositeId elemId, QVariant value)
{
    QJsonObject res;
    if (name == "GET_PARAM_HEADER" ) {
        res["device_id"] = deviceId;
        res["module_id"] = elemId.moduleId;
        res["param_id"] = elemId.paramId;
    } else if (name == "GET_PARAM_DATA" || name == "STREAM_PARAM_DATA") {
        res["device_id"] = deviceId;
        res["module_id"] = elemId.moduleId;
        res["param_id"] = elemId.paramId;
    } else if (name == "SET_PARAM_DATA") {
        res["device_id"] = deviceId;
        res["module_id"] = elemId.moduleId;
        res["param_id"] = elemId.paramId;
        res["value"] = QJsonValue::fromVariant(value);
    }

    return res;
}

QJsonObject MainWindowVM::createDeviceCmdBody(QString cmd, int deviceId, int moduleId)
{
    QJsonObject res;
    if (cmd == "GET_DEVICE_HEADER") {
        res["device_id"] = deviceId;
        res["module_id"] = moduleId;
    }

    return res;
}

QJsonObject MainWindowVM::createOscCmdBody(QString cmd, int deviceId, int oscId, int chArg)
{
    QJsonObject res;
    if (cmd == "GET_OSC_HEADER") {
        res["device_id"] = deviceId;
        res["osc_id"] = oscId;
    } else if (cmd == "GET_OSC_CHANNEL") {
        res["device_id"] = deviceId;
        res["osc_id"] = oscId;
        res["channel_num"] = chArg;
    } else if (cmd == "OSC_PARAM_DATA") {
        res["device_id"] = deviceId;
        res["osc_id"] = oscId;

        if (chArg > 0) {
            QJsonArray channels;
            channels.append(QJsonValue(chArg));
            res["osc_vars"] = channels;
        }
    }

    return res;
}

MainWindowVM::CompositeId MainWindowVM::parse(QString arg) const
{
    if (arg.isEmpty()) return CompositeId();

    QStringList args = arg.split(".");

    if (args.count()!=2) return CompositeId();

    CompositeId res;
    res.moduleId = args.value(0).toInt();
    res.paramId = args.value(1).toInt();

    return res;
}

void MainWindowVM::setParamInfo(const QJsonObject& obj)
{
    if (obj.isEmpty()) return;

    int moduleId = obj.value("module_id").toInt();
    int paramId = obj.value("param_id").toInt();
    QString dim = obj.value("value_unit").toString();
    QString name = obj.value("name").toString();
    QString desc = obj.value("desc").toString();
    QString rw = obj.value("rw").toString();

    QString s_name = (name.contains("(") && name.contains(")")) ? name
                                                                : name + "(" + dim + ")";
    QString s_rw = (rw == "R") ? "readable"
                             : (rw == "W") ? "writable" : rw;

    QString info("");
    info = QString("[%1.%2]: name = %3, %4 ")
            .arg(moduleId)
            .arg(paramId)
            .arg(s_name)
            .arg(s_rw);

    if (!desc.isEmpty()) {
        info = info + ", desc = " + desc;
    }

    CompositeId param1 = parse(m_paramComposId1);
    CompositeId param2 = parse(m_paramComposId2);

    if (param1.paramId == paramId && param1.moduleId == moduleId) {
        m_paramInfo1 = info;
        emit paramInfo1Changed(m_paramInfo1);
    } else if (param2.paramId == paramId && param2.moduleId == moduleId) {
        m_paramInfo2 = info;
        emit paramInfo2Changed(m_paramInfo2);
    }
}

void MainWindowVM::setParamValue(const QJsonObject& obj, int errorCode)
{
    if (obj.isEmpty()) return;

    int paramId = obj.value("param_id").toInt();
    int moduleId = obj.value("module_id").toInt();
    QJsonObject valObj = obj.value("value").toObject();
    qint8 format = obj.value("format").toInt();
    double scale = obj.value("scale").toDouble();

    QJsonValue value = valObj.value("value");

    QString s_value = value.toVariant().toString();
    s_value = (format == 3) ? QString::number(value.toDouble(), 'f', 2) : s_value;

    double time = valObj.value("time").toDouble();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(time);

    QString info = QString("value = %1, scale = %2, time = %3 %4")
            .arg(s_value)
            .arg(scale)
            .arg(dt.time().toString("HH:mm:ss.zzz"))
            .arg(errorCode != 0 ? QString(", error = %1").arg(errorCode) : "");

    emitParamValue(moduleId, paramId, s_value, info);
}

void MainWindowVM::setStreamParamValue(const QJsonObject& obj)
{
    int paramId = obj.value("p_id").toInt();
    int moduleId = obj.value("m_id").toInt();
    QVariant value = obj.value("value").toObject().value("value");
    double time = obj.value("value").toObject().value("time").toDouble();
    QString svalue = value.canConvert(QMetaType::Float) ? QString::number(value.toFloat(), 'f', 2) : value.toString();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(time);
    int errorCode = obj.value("error").toInt();

    QString info = QString("value = %1 %2 %3")
            .arg(svalue)
            .arg(dt.time().toString("mm:ss.zzz"))
            .arg(errorCode != 0 ? QString(", error = %1").arg(errorCode) : "");

    emitParamValue(moduleId, paramId, svalue, info);
}

void MainWindowVM::emitParamValue(int moduleId, int paramId, QString sVal, QString info)
{
    CompositeId param1 = parse(m_paramComposId1);
    CompositeId param2 = parse(m_paramComposId2);

    if (param1.paramId == paramId && param1.moduleId == moduleId) {
        m_paramValue1 = sVal;
        emit paramValue1Changed(sVal);
        m_paramValueInfo1 = info;
        emit valueInfo1Changed(info);
    } else if (param2.paramId == paramId && param2.moduleId == moduleId) {
        m_paramValue2 = sVal;
        emit paramValue2Changed(sVal);
        m_paramValueInfo2 = info;
        emit valueInfo2Changed(info);
    }
}

QString MainWindowVM::sysTypeToString(SysType sysType)
{
    switch (sysType) {
    case Undefined: return "Undefined";
    case FILE_IO: return "FILE_IO";
    case CANOPEN: return "CANOPEN";
    case MODBUS: return "MODBUS";
    case UAVCAN: return "UAVCAN";
    case CONNEX_MVCP: return "CONNEX_MVCP";
    }

    return "";
}

void MainWindowVM::setOscChannelInfo(const QJsonObject &obj)
{
    int chNum = obj.value("ch_num").toInt();
    QString name = obj.value("name").toString();
    int varId = obj.value("var_id").toInt();
    double scale = obj.value("scale").toDouble();
    QString analogable = obj.value("isDiscrete").toBool() ? "discrete" : "analog";

    QString output = QString("Channel: num = %1, name = %2, var id = %3, %4, scale = %5")
            .arg(chNum)
            .arg(name)
            .arg(varId)
            .arg(analogable)
            .arg(scale);

    m_oscChannelValue = output;
    emit oscChannelValueChanged(m_oscChannelValue);
}

void MainWindowVM::setOscChannelValue(const QJsonObject &obj)
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

    QString output = dvalList.join(" ");

    m_oscChannelValue = output;
    emit oscChannelValueChanged(m_oscChannelValue);
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

    requestSystemInfo();
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

    QJsonDocument doc;
    if (reqId == PARAM_REQUEST_ID) {
        QJsonArray body = data.value("body").toArray();
        QJsonObject bodyObj = body.first().toObject();
        doc = QJsonDocument(bodyObj);
        setParamInfo(bodyObj);

    } else if (reqId == SYSTEM_REQUEST_ID) {
        QJsonArray body = data.value("body").toArray();
        doc = QJsonDocument(body);
        setSystemInfo(body);
    } else if (reqId == DEVICE_REQUEST_ID) {
        QJsonValue body = data.value("body");
        if (body.isArray()) {
            QJsonObject bodyObj = body.toArray().first().toObject();
            doc = QJsonDocument(bodyObj);
            setDeviceDescr(bodyObj);
        } else {
            QJsonObject bodyObj = body.toObject();
            doc = QJsonDocument(bodyObj);
            setModuleDescr(bodyObj);
        }
    } else if (reqId == OSC_REQUEST_ID) {
        QJsonObject bodyObj = data.value("body").toObject();
        doc = QJsonDocument(bodyObj);
        setOscDescr(bodyObj);
    } else if (reqId == OSC_CHANNEL_REQUEST_ID) {
        QJsonObject bodyObj = data.value("body").toObject();
        doc = QJsonDocument(bodyObj);
        setOscChannelInfo(bodyObj);
    } else if (reqId == PARAM_VALUE_REQUEST_ID || reqId == PARAM_VALUE_SET_ID) {
        QJsonObject bodyObj = data.value("body").toObject();
        int error = data.value("error").toInt();

        doc = QJsonDocument(bodyObj);
        setParamValue(bodyObj, error);
    }

    if (doc.isEmpty()) return;

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
        setOscChannelValue(data);
    } else {
        setStreamParamValue(data);
    }

    QTextStream(stdout) << output << "\n" ;
}

void MainWindowVM::sendRequest(QJsonObject req, bool checkPerformance)
{
    QJsonDocument doc(req);
    QString strReq(doc.toJson(QJsonDocument::Compact));

    if (checkPerformance) {
        m_perfomanceTimer.restart();
    }

    emit dataRequest(strReq);
    m_webSocket.sendTextMessage(strReq);
}
