#include "oscdatasaver.h"

#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCborValue>

long OscDataJSonFileSaver::save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data)
{
    //  const char* home = getenv("HOME");
        QString path =  QString("./data/"); // qApp->applicationDirPath()
        QJsonObject jsonObj = headerToJson(header);

        QDateTime trigTime = QDateTime::currentDateTime(); //QDateTime::fromTime_t(static_cast<uint>(header.settings.trig_time));
        QString hdrfileName = path + QString("%1_%2_%3.hdr").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh:mm:ss:zzz"));

        _dde_func_return_t res = saveObj(hdrfileName, jsonObj);

        if (!res)
            return res;

        QJsonObject datjsonObj = data.toJson();
        QString datFileName = path + QString("%1_%2_%3.dat").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh:mm:ss:zzz"));

        res = saveObj(datFileName, datjsonObj, true);

        qInfo() << "Saved data file:" << datFileName << Qt::endl;

        return res;
}

long OscDataJSonFileSaver::saveObj(const QString fileName, const QJsonObject &obj, bool useBinary)
{
    _dde_func_return_t res = _return_OK;

    QJsonDocument doc(obj);
    QByteArray bytes;

    if (useBinary) {
        QCborValue v = QCborValue::fromJsonValue(obj);
        bytes = v.toCbor(QCborValue::UseFloat);
    } else {
        bytes = doc.toJson(QJsonDocument::Compact);
    }

    QFile file( fileName );

    if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << bytes;
        file.close();
    }
    else
    {
         QTextStream(stdout) << "file open failed: " << fileName << endl;//Qt::endl;
         return _return_FAIL;
    }

    return res;
}

QJsonObject OscDataJSonFileSaver::headerToJson(const DDE_OSC_HEADER &h)
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
        obj["var_id"] = ch.var.id;
        obj["name"] = ch.var.name;
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

QString OscDataJSonFileSaver::colorToString(const int &c)
{
    QString ret = QString("#%1")
            .arg(QString::number(c, 16).rightJustified(6, '0'));

    return ret;

}
