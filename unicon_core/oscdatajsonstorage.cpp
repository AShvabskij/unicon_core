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
#include <QCborStreamWriter>
#include <QDataStream>
#include <QDir>

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define ENDL Qt::endl
#else
#define ENDL "\n"
#endif

const uint8_t DATA_VERSION = 1;
const uint8_t DATA_SUBVERSION = 1;

namespace {
QString colorToString(const int &c)
{
    QString ret = QString("#%1")
            .arg(QString::number(c, 16).rightJustified(6, '0'));

    return ret;

}


int stringToColor(QString hexColor)
{
    hexColor = hexColor.remove("#");
    int retColor = hexColor.toUInt(nullptr, 16);
    return retColor;
}
}

long OscDataJSonStorage::save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data)
{
    QTextStream(stdout) << "Saving osc data, device id = " << header.device_id << ENDL;

    //  const char* home = getenv("HOME");
        QString path =  createPath(header);
        QJsonObject jsonObj = headerToJson(header);

        QDateTime trigTime = QDateTime::fromTime_t(static_cast<uint>(header.settings.trig_time));
        QString baseFileName = QString("%1-%2-%3").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh_mm_ss"));

        QString headerFile = path + "/" + baseFileName + ".hdr";
        _dde_func_return_t res = saveObj(headerFile, jsonObj);

        if (!res)
            return res;

        QJsonObject datjsonObj = data.serializeToJSon();

        QString datFile = path + "/" + baseFileName + ".dat";
        res = saveObj(datFile, datjsonObj, true);

        return res;
}

QString OscDataJSonStorage::createPath(const DDE_OSC_HEADER &)
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
        qWarning() << "Couldn't create folder:" + path;
    }

    return path;
}

long OscDataJSonStorage::saveObj(const QString fileName, const QJsonObject &obj, bool useBinaryFormat)
{
    _dde_func_return_t res = _return_OK;

    QFile file( fileName );

    if (useBinaryFormat) {
        if( !file.open( QIODevice::WriteOnly |  QIODevice::Truncate ) ) {
            QTextStream(stdout) << "File open failed: " << fileName << ENDL;
            return _return_FAIL;
        }

        QCborStreamWriter writer(&file);

        QCborValue cborValue = QCborValue::fromJsonValue(obj);
        QByteArray bytes = cborValue.toCbor(QCborValue::UseFloat);

        writer.append(bytes); // serialize the QCborValue to the file

    } else {
        if( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
        {
            QTextStream(stdout) << "File open failed: " << fileName << ENDL;
            return _return_FAIL;
        }
        QJsonDocument doc(obj);
        QByteArray bytes = doc.toJson(QJsonDocument::Compact);

        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << bytes;
    }

    file.close();
    return res;
}

QString OSC_VAR_TYPE_TO_STRING(OSC_VAR_TYPE type)
{
    switch (type) {
        case OSC_VAR_TYPE::OSC_VAR_FLOAT: return "FLT";
        case OSC_VAR_TYPE::OSC_VAR_INT: return "INT";
        case OSC_VAR_TYPE::OSC_VAR_DISCRETE: return "BIT";
    default: return "";
    }
}

OSC_VAR_TYPE OSC_VAR_TYPE_FROM_STRING(QString type)
{
    if (type == "FLT") return OSC_VAR_TYPE::OSC_VAR_FLOAT;
    if (type == "INT") return OSC_VAR_TYPE::OSC_VAR_INT;
    if (type == "BIT") return OSC_VAR_TYPE::OSC_VAR_DISCRETE;

    return OSC_VAR_TYPE::UNDEFINED;
}

QJsonObject OscDataJSonStorage::headerToJson(const DDE_OSC_HEADER &h)
{
    QJsonObject res;

    res["version"] = DATA_VERSION;
    res["sub_version"] = DATA_SUBVERSION;
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

        obj["type"] = OSC_VAR_TYPE_TO_STRING(ch.var.type);

        channelsObj << obj;
    }

    res["channels"] = channelsObj;

    return res;
}

long OscDataJSonStorage::loadHeader(QString fileFrom, DDE_OSC_HEADER &header)
{
    QString path = QFileInfo(fileFrom).absolutePath();
    QString name = QFileInfo(fileFrom).baseName();

    QString headerFile = path + QDir::separator() + name + ".hdr";
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
    if (ret <=0 )
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

long OscDataJSonStorage::loadData(QString fileFrom, OscType::OscDataBuffer *data)
{
    QString path = QFileInfo(fileFrom).absolutePath();
    QString name = QFileInfo(fileFrom).baseName();

    QString datFile = path + QDir::separator() + name + ".dat";
    QFile file( datFile );

    if(!file.open( QIODevice::ReadOnly))
    {
        QTextStream(stdout) << "file open failed: " << datFile << ENDL;
        return _return_FAIL;
    }

    QCborStreamReader reader( &file );
    QByteArray bytes = decodeByteArray(reader);

    file.close();

    QCborValue cborValue = QCborValue::fromCbor(bytes);
    QJsonValue resValue = cborValue.toJsonValue();
    QJsonObject obj = resValue.toObject();

    long ret = jsonToData(obj, *data);

    return ret;
}

long OscDataJSonStorage::jsonToHeader(const QJsonObject& obj, DDE_OSC_HEADER &h)
{
    uint8_t ver = obj["version"].toVariant().toUInt();
    uint8_t sub_ver = obj["sub_version"].toVariant().toUInt();

    if (ver != DATA_VERSION) {
        QTextStream(stdout) << "The json header version " <<  ver << " is not supported" <<  ", the current version is " << DATA_SUBVERSION << ENDL;
        return -1;
    }

    if (sub_ver > DATA_SUBVERSION) {
        QTextStream(stdout) << "The json header version " << sub_ver <<  "is an older version of the current version " << DATA_SUBVERSION << ENDL;
    }

    h.device_id = obj["device_id"].toVariant().toInt();
    h.settings.reason = obj["reason"].toVariant().toInt();
    h.settings.trig_time = obj["trig_time"].toVariant().toInt();
    h.settings.triger_mode = obj["trig_mode"].toVariant().toInt();
    h.settings.time_resolution_us = obj["resolution_us"].toVariant().toInt();

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
        ch.var.color = stringToColor(elem["color"].toString());
        ch.var.type = OSC_VAR_TYPE_FROM_STRING(elem["type"].toString());
    }

    return _return_OK;
}

long OscDataJSonStorage::jsonToData(const QJsonObject& obj,  OscType::OscDataBuffer &data)
{
    uint8_t ver = obj["version"].toVariant().toUInt();
    uint8_t sub_ver = obj["sub_version"].toVariant().toUInt();

    if (ver != DATA_VERSION) {
        QTextStream(stdout) << "The json data version " <<  ver << " is not supported" <<  ", the current supported version is " << DATA_VERSION << ENDL;
        return -1;
    }

    if (sub_ver > DATA_SUBVERSION) {
        QTextStream(stdout) << "The json data version " << sub_ver <<  " is an older version of the current version " << DATA_SUBVERSION << ENDL;
    }

    data.id =  obj["d_id"].toInt();
    data.timestamp = obj["time"].toVariant().toLongLong();
    QJsonArray vars = obj["vars"].toArray();
    QJsonArray values = obj["values"].toArray();

    int maxValueCount = 0;
    for (int i = 0; i < vars.count(); ++i) {
        data.ch[i].varId = vars[i].toInt();
        data.ch[i].values = values[i].toArray().toVariantList();
        int valueCount = data.ch[i].values.count();
        maxValueCount = maxValueCount < valueCount ? valueCount : maxValueCount;
    }

    data.valueCount = maxValueCount;

    return _return_OK;
}
