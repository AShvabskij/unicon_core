#include "oscdatajsonstorage.h"

#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QCborValue>
#include <QCborStreamReader>
#include <QDir>

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define ENDL Qt::endl
#else
#define ENDL endl
#endif

long OscDataJSonStorage::save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data)
{
    //  const char* home = getenv("HOME");
        QString path =  createPath(header);
        QJsonObject jsonObj = headerToJson(header);

        QDateTime trigTime = QDateTime::fromTime_t(static_cast<uint>(header.settings.trig_time));
        QString baseFileName = QString("%1-%2-%3").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh_mm_ss"));

        QString headerFile = path + "/" + baseFileName + ".hdr";
        _dde_func_return_t res = saveObj(headerFile, jsonObj);

        if (!res)
            return res;

        QJsonObject datjsonObj = data.toJson();

        QString datFile = path + "/" + baseFileName + ".dat";
        res = saveObj(datFile, datjsonObj, true);

        return res;
}

QString OscDataJSonStorage::createPath(const DDE_OSC_HEADER &header)
{
    QDateTime now = QDateTime::currentDateTime();
    QString year = "Y" + QString::number(now.date().year());
    QString abbreviatedMonth = now.toString("MMM");
    QString dayOfMonth = abbreviatedMonth + "_" + QString::number(now.date().day());

    QString dataLoggerPath =  QString("./DataLogger/"); // qApp->applicationDirPath()
    QString path = dataLoggerPath + year + "/" + abbreviatedMonth + "/" + dayOfMonth;

    QDir dir;
    bool res = dir.mkpath(path);

    if (!res) {
        qWarning() << "Couldn't create folder to path:" + path;
    }

    return path;
}

long OscDataJSonStorage::saveObj(const QString fileName, const QJsonObject &obj, bool useBinaryFormat)
{
    _dde_func_return_t res = _return_OK;

    QJsonDocument doc(obj);
    QByteArray bytes;
    if (useBinaryFormat) {
        QCborValue cborValue = QCborValue::fromJsonValue(obj);
        bytes = cborValue.toCbor(QCborValue::UseFloat);
    } else {
        bytes = doc.toJson(QJsonDocument::Compact);
    }

    QFile file( fileName );

    if (useBinaryFormat) {
        if( !file.open( QIODevice::WriteOnly |  QIODevice::Truncate ) ) {
            QTextStream(stdout) << "file open failed: " << fileName << ENDL;
            return _return_FAIL;
        }

        QCborStreamWriter writer(&file);

        // Serialize the QCborValue to the file
        writer.append(bytes);
    } else {
        if( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
        {
            QTextStream(stdout) << "file open failed: " << fileName << ENDL;
            return _return_FAIL;
        }

        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << bytes;
    }

    file.close();
    return res;
}

QJsonObject OscDataJSonStorage::headerToJson(const DDE_OSC_HEADER &h)
{
    QJsonObject res;

    res["device_id"] = h.device_id;
    res["id"] = h.device_id;
    res["trig_time"] = QString::number(h.settings.trig_time);
    res["resolution_us"] = QString::number(h.settings.time_resolution_us);

    QJsonArray channelsObj;
    for (int chInd = 0; chInd < h.settings.channel_count; chInd++) {
        const OSC_CHANNEL& ch = h.channels[chInd];
        QJsonObject obj;
        obj["ch_num"] = ch.chNum;
        obj["first_bit"] = ch.firstBit;
        obj["last_bit"] = ch.lastBit;
        obj["gain"] = ch.gain;
        obj["offset"] = ch.offset;


        obj["var_id"] = ch.var.id;
        obj["name"] = ch.var.name;
        obj["dim"] = ch.var.dim;
        obj["scale"] = ch.var.scale;
        obj["min"] = ch.var.min;
        obj["max"] = ch.var.max;
        obj["color"] = colorToString(ch.var.color);
        obj["isDiscrete"] = ch.var.type == OSC_VAR_TYPE::DISCRETE;
        obj["isAnalog"] = ch.var.type == OSC_VAR_TYPE::ANALOG;
        obj["isDigital"] = ch.var.type == OSC_VAR_TYPE::DIGITAL;

        channelsObj << obj;
    }

    res["channels"] = channelsObj;

    return res;
}

QString OscDataJSonStorage::colorToString(const int &c)
{
    QString ret = QString("#%1")
            .arg(QString::number(c, 16).rightJustified(6, '0'));

    return ret;

}

long OscDataJSonStorage::loadHeader(QString fileFrom, DDE_OSC_HEADER &header)
{
    QString path = QFileInfo(fileFrom).absolutePath() + QString("\\");
    QString name = QFileInfo(fileFrom).baseName();

    QString headerFile = path + name + ".hdr";
    QFile file( headerFile );

    if(!file.open( QIODevice::ReadOnly | QIODevice::Text ))
    {
        QTextStream(stdout) << "file open failed: " << headerFile << ENDL;
        return _return_FAIL;
    }

    QTextStream inStream( &file );
    QString content = inStream.readAll();

    QJsonDocument d = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject obj = d.object();

    long ret = jsonToHeader(obj, header);
    if (!ret)
        return ret;


    return _return_OK;
}

QByteArray decodeByteArray(QCborStreamReader &reader)
{
    QByteArray result;
    auto r = reader.readByteArray();
    while (r.status == QCborStreamReader::Ok) {
        result += r.data;
        r = reader.readByteArray();
    }

    if (r.status == QCborStreamReader::Error) {
        // handle error condition
        result.clear();
    }
    return result;
}

QString decodeString(QCborStreamReader &reader)
{
    QString result;
    auto r = reader.readString();
    while (r.status == QCborStreamReader::Ok) {
        result += r.data;
        r = reader.readString();
    }

    if (r.status == QCborStreamReader::Error) {
        // handle error condition
        result.clear();
    }
    return result;
}

long OscDataJSonStorage::loadData(QString fileFrom, OscType::OscDataBuffer *data)
{
    QString path = QFileInfo(fileFrom).absolutePath() + QString("\\");
    QString name = QFileInfo(fileFrom).baseName();

    QString datFile = path + name + ".dat";
    QFile file( datFile );

    if(!file.open( QIODevice::ReadOnly | QIODevice::Text ))
    {
        QTextStream(stdout) << "file open failed: " << datFile << ENDL;
        return _return_FAIL;
    }

    QTextStream inStream( &file );

    QByteArray bytes;
    inStream >> bytes;

    QString content = inStream.readAll();

    QJsonObject datjsonObj = data->toJson();
    QJsonDocument doc(datjsonObj);
    QString js_str = doc.toJson();

    QCborValue v1 = QCborValue::fromVariant(js_str);
    QCborValue v2 = QCborValue::fromJsonValue(datjsonObj);
//  bytes = v2.toCbor(QCborValue::UseFloat);

    QString encodedString = bytes;

    // Convert QString to QCborValue
    QCborValue cborValue = QCborValue::fromCbor(bytes);

    QJsonValue resValue = cborValue.toJsonValue();
    QByteArray cborData = cborValue.toByteArray();
    QJsonObject obj = resValue.toObject();

    long ret = jsonToData(obj, *data);
    if (!ret)
        return ret;


    return _return_OK;
}

long OscDataJSonStorage::jsonToHeader(const QJsonObject& obj, DDE_OSC_HEADER &h)
{
    h.device_id = obj["device_id"].toInt();
    h.settings.reason = obj["reason"].toInt();
    h.settings.trig_time = obj["trig_time"].toInt();
    h.settings.triger_mode = obj["trig_mode"].toInt();
    h.settings.time_resolution_us = obj["resolution_us"].toInt();

    QJsonArray arr = obj["channels"].toArray();
    h.settings.channel_count = arr.count();
    for (int ind = 0; ind < h.settings.channel_count; ind++) {
        QJsonObject elem = arr[ind].toObject();
        auto& ch = h.channels[ind];

        ch.chNum = elem["ch_num"].toInt();
        ch.firstBit = elem["firstBit"].toInt();
        ch.lastBit = elem["lastBit"].toInt();
        ch.gain = elem["gain"].toInt();
        ch.offset = elem["offset"].toInt();

        ch.var.id = elem["var_id"].toInt();
        strcpy(ch.var.name, elem["name"].toString().toStdString().c_str());
        strcpy(ch.var.dim, elem["dim"].toString().toStdString().c_str());
        ch.var.scale = elem["scale"].toDouble(0);
        ch.var.min = elem["min"].toDouble(0);
        ch.var.max = elem["max"].toDouble(0);
        ch.var.color = elem["color"].toInt();
        ch.var.type = (elem["isAnalog"].toBool()) ? OSC_VAR_TYPE::ANALOG : h.channels[ind].var.type;
        ch.var.type = (elem["isDiscrete"].toBool()) ? OSC_VAR_TYPE::DISCRETE : h.channels[ind].var.type;
        ch.var.type = (elem["isDigital"].toBool()) ? OSC_VAR_TYPE::DIGITAL : h.channels[ind].var.type;
    }

    return _return_OK;
}

long OscDataJSonStorage::jsonToData(const QJsonObject& obj,  OscType::OscDataBuffer &data)
{
    data.id =  obj["d_id"].toInt();
    data.timestamp = obj["time"].toVariant().toLongLong();
    data.eof = obj["eof"].toBool();
    data.sof = obj["sof"].toBool();
    QJsonArray vars = obj["vars"].toArray();
    QJsonArray values = obj["values"].toArray();

    for (int i = 0; i < vars.count(); ++i) {
        data.ch[i].varId = vars[i].toInt();
        data.ch[i].values = values[i].toArray().toVariantList();
    }

    return _return_OK;
}
