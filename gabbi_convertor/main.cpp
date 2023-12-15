#include <QCoreApplication>
#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QtDebug>
#include <QFile>
#include <QColor>

#include <iostream>
#include <memory>

#include <oscdatajsonstorage.h>
#include <oscdataservice.h>
#include "DDE_TYPES.h"

const int SET_SIZE = 16;
const char SEP = ',';

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define ENDL Qt::endl
#else
#define ENDL endl
#endif

OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER &hdr)
{
    OscType::OscDataBuffer* buff = new OscType::OscDataBuffer();

    buff->id = hdr.device_id;
    buff->timestamp = hdr.settings.trig_time;

    for (int chInd = 0; chInd < hdr.settings.channel_count; chInd++) {
        const OSC_CHANNEL& channel = hdr.channels[chInd];
        if (channel.var.id <= 0) {
            continue;
        }

        OscType::OscChannelValues& chValues = buff->ch[chInd];
        chValues.channelNum = channel.chNum;
        chValues.varId = channel.var.id;
        chValues.scale = channel.var.scale;
    }

    return buff;
}

long generateContent(const DDE_OSC_HEADER& hdr, const OscType::OscDataBuffer& data, QTextStream& stream)
{
     QTextStream& res = stream;

     QDateTime trigTime = QDateTime::fromTime_t(static_cast<uint>(hdr.settings.trig_time));
     QString plotName = QString("%1_%2_%3").arg(hdr.device_id).arg(hdr.settings.reason).arg(trigTime.toString("hh:mm:ss:zzz"));

     res << ".PlotName, " << plotName << ENDL;
     res << ".Date, " << QDateTime(QDateTime::fromTime_t(static_cast<uint>(hdr.settings.trig_time))).date().toString();
     res << ".Time, " << QDateTime(QDateTime::fromTime_t(static_cast<uint>(hdr.settings.trig_time))).time().toString();
     res << ".Ts" << hdr.settings.time_resolution_us;

     res << ENDL;

     int numOfSet = 1;
     QStringList varIndexes;
     QStringList varNames;
     QStringList varNumOfSets;

     for (int i= 0; i < hdr.settings.channel_count; ++i) {
         auto& ch = hdr.channels[i];

         if (ch.var.id <= 0)
             continue;

         int chNumOfSet = ch.chNum - (numOfSet - 1) * SET_SIZE;

         QStringList line;

         QRgb rgb = ch.var.color;

         if (ch.var.type == OSC_VAR_TYPE::ANALOG || ch.var.type == OSC_VAR_TYPE::DIGITAL) {
             line << QString("@") + QString(ch.var.name)
                 << QString("L") + QString::number(numOfSet)
                 << QString::number(chNumOfSet).leftJustified(2, '0')
                 << QString("D") + QString::number(ch.firstBit).leftJustified(2, '0')
                 << QString("D") + QString::number(ch.lastBit).leftJustified(2, '0')
                 << QString::number(ch.gain) << QString::number(ch.offset)
                 << QString::number(qRed(rgb)) << QString::number(qGreen(rgb)) << QString::number(qBlue(rgb))
                 << "TRUE";
         }
         else if (ch.var.type == OSC_VAR_TYPE::DISCRETE) {
             line << QString("&") + QString(ch.var.name)
                 << QString("L") + QString::number(numOfSet)
                 << QString::number(chNumOfSet).leftJustified(2, '0')
                 << QString("D") + QString::number(ch.firstBit).leftJustified(2, '0')
                 << QString("BIT")
                 << QString::number(ch.var.color) << "TRUE";
         }

         res << line.join(SEP) << ENDL;

         varIndexes << QString::number(i);
         varNames << ch.var.name;
         varNumOfSets <<  QString("L") + QString::number(numOfSet) + "_" + QString::number(chNumOfSet).leftJustified(2, '0');
     }

     res << ENDL << ENDL;

     res << "* " << SEP << varIndexes.join(SEP) << ENDL;
     res << "* " << SEP << varNames.join(SEP)  << ENDL;
     res << "* " << SEP << varNumOfSets.join(SEP)  << ENDL;

     for (int i = 0; i < data.valueCount; i++) {
         QStringList rec;
         for (int chNum = 0; chNum <= OSC_MAX_VARS; chNum++) {
             const OscType::OscChannelValues& chVal = data.ch[chNum];

             if (chVal.varId == 0)
                 continue;

             if (i >= chVal.values.count()) {
                 rec << 0;
                 break;
             }

             auto& ch = hdr.channels[chNum];
             rec << chVal.values[i].toString();
         }

         res << rec.join(",") << ENDL;
     }

    return 1;
}

long doConvert(QString fileFrom, QString fileTo)
{
    IOscDataStorageService* datFile = new OscDataJSonStorage();

    DDE_OSC_HEADER hdr;
    long res = datFile->loadHeader(fileFrom, hdr);

    if (!res)
        return res;

    OscType::OscDataBuffer* datBuff = createDataBuffer(hdr);
    res = datFile->loadData(fileFrom, datBuff);

    if (!res)
        return res;

    QFile file( fileTo );

    if(!file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
         QTextStream(stdout) << "file open failed: " << fileTo << ENDL;
         return _return_FAIL;
    }

    QTextStream iStream( &file );
    iStream.setCodec( "utf-8" );

    res = generateContent(hdr, *datBuff, iStream);
    file.close();

    return res;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc >= 3) {
        QString firstArg = argv[1];
        QString secondArg = argv[2];

        qInfo() << "convert from  = " << firstArg << "to = " << secondArg;

        long res = doConvert(firstArg, secondArg);

        if (res) {
            qInfo() << "Convertion сompleted successfully!";
        }
    }

//  return a.exec();
}
