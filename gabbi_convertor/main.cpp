#include <QCoreApplication>
#include <QString>
#include <QDateTime>
#include <QVariant>
#include <QtDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QColor>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>

#include <iostream>
#include <memory>

#include <oscdatastorage.h>
#include <oscdatastorage_v1_2.h>

#include <oscdataservice.h>
#include "DDE_TYPES.h"

const int SET_SIZE = 16;
const char SEP = ',';
const int MAX_BIT_NUM = 15; // from zero to 15
const int MAX_VALUE = 0xFFFE; // from zero to 15

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define ENDL Qt::endl
#else
#define ENDL "\n"
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

        OscType::OscChannelValues& chValues = buff->chArray[chInd];
        chValues.channelNum = channel.chNum;
        chValues.varId = channel.var.id;
        chValues.scale = channel.gain;
        chValues.offset = channel.offset;
    }

    return buff;
}

int denormalizeValue(float value)
{
    if (value > MAX_VALUE) {
        value = MAX_VALUE;
    }

    uint16_t zeroLevel = 0x7FFF;
    int res = value + zeroLevel;
    return res;
}

long generateContent(const DDE_OSC_HEADER& hdr, const OscType::OscDataBuffer& data, QTextStream& stream)
{
     QTextStream& res = stream;

     QDateTime trigTime = QDateTime::fromTime_t(static_cast<uint>(hdr.settings.trig_time));
     QString plotName = QString("%1_%2_%3").arg(hdr.device_id).arg(hdr.settings.reason).arg(trigTime.toString("hh:mm:ss:zzz"));
     double resolution_sec = (double)hdr.settings.time_resolution_us / (1000 * 1000);

     res << ".PlotName," << plotName << "," << ENDL;
     res << ".Date," << QDateTime(QDateTime::fromTime_t(static_cast<uint>(hdr.settings.trig_time))).date().toString() << "," << ENDL;
     res << ".Time," << QDateTime(QDateTime::fromTime_t(static_cast<uint>(hdr.settings.trig_time))).time().toString() << "," << ENDL;
     res << ".Ts," << resolution_sec << ","  << ENDL;

     res << ENDL;

     int numOfSet = 1;
     QStringList varIndexes;
     QStringList varNames;
     QStringList varNumOfSets;

     for (int i= 0; i < hdr.settings.channel_count; ++i) {
         auto& ch = hdr.channels[i];

         if (ch.var.id <= 0)
             continue;

         int chNumOfSet = (ch.chNum + 1) - (numOfSet - 1) * SET_SIZE;

         QStringList line;

         QRgb rgb = ch.var.color;

         if (ch.var.type == OSC_VAR_FLOAT || ch.var.type == OSC_VAR_INT) {
             int lastBit = (ch.lastBit > 0 && ch.lastBit < MAX_BIT_NUM) ? ch.lastBit : MAX_BIT_NUM;

             line << QString("@") + QString(ch.var.name)
                 << QString("L") + QString::number(numOfSet)
                 << QString::number(chNumOfSet).rightJustified(2, '0')
                 << QString("D") + QString::number(ch.firstBit).rightJustified(2, '0')
                 << QString("D") + QString::number(lastBit).rightJustified(2, '0')
                 << QString::number(ch.gain) << QString::number(ch.offset)
                 << QString::number(qRed(rgb)) + " " + QString::number(qGreen(rgb)) + " " + QString::number(qBlue(rgb))
                 << "TRUE";
         }
         else if (ch.var.type == OSC_VAR_DISCRETE) {
             line << QString("&") + QString(ch.var.name)
                 << QString("L") + QString::number(numOfSet)
                 << QString::number(chNumOfSet).rightJustified(2, '0')
                 << QString("D") + QString::number(ch.firstBit).rightJustified(2, '0')
                 << QString("BIT")
                 << QString::number(qRed(rgb)) + " " + QString::number(qGreen(rgb)) + " " + QString::number(qBlue(rgb))
                 << "TRUE";
         }

         res << line.join(SEP) << ENDL;

         varIndexes << QString::number(i);
         varNames << ch.var.name;
         varNumOfSets <<  QString("L") + QString::number(numOfSet) + "_" + QString::number(chNumOfSet).rightJustified(2, '0');
     }

     res << ENDL << ENDL;

     res << "* " << SEP << varIndexes.join(SEP) << ENDL;
     res << "* " << SEP << varNames.join(SEP)  << ENDL;
     res << "* " << SEP << varNumOfSets.join(SEP)  << ENDL;

     for (int i = 0; i < data.valueCount; i++) {
         QStringList rec;
         rec << QString::number(i + 1);

         for (int chNum = 0; chNum <= OSC_MAX_VARS; chNum++) {
             const OscType::OscChannelValues& chVal = data.chArray[chNum];

             if (chVal.varId == 0)
                 continue;

             if (i >= chVal.count()) {
                 rec << 0;
                 break;
             }

             QVariant val = chVal.value(i);
             int value = denormalizeValue(val.toFloat());
             rec << QString::number(value);
         }

         res << rec.join(",") << ENDL;
     }

    return 1;
}

long doConvert(QString fileFrom, QString fileTo)
{
    QList<IOscDataStorageService*> datServiceCollection;
    datServiceCollection.append(new OscDataStorage());
    datServiceCollection.append(new OscDataStorage_v1_2());

    long res = _return_OK;
    bool isHandled = false;
    for (IOscDataStorageService* datService : datServiceCollection) {

        res = datService->checkVersion(fileFrom);
        if (res <= 0)
            continue;

        isHandled = true;

        DDE_OSC_HEADER hdr;
        res = datService->loadHeader(fileFrom, hdr);

        if (res != _return_OK)
            break;

        OscType::OscDataBuffer* datBuff = createDataBuffer(hdr);
        res = datService->loadData(fileFrom, *datBuff);

        if (res != _return_OK)
            break;

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

        break;
    }

    if (res != _return_OK && !isHandled ) {
        QTextStream(stdout) << "There is not any suitable converter for the file!" << ENDL;
    }

    return res;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

//  std::cout << "Hello, world!" << std::endl;

    QCoreApplication::setApplicationName("GabbiConverter");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Read file names from command line arguments");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption file1Option(QStringList() << "from", "Path to source file (Unicon format)", "id-reason-time[.hdr|dat]");
    QCommandLineOption file2Option(QStringList() << "to", "Write generated data into <file> (Gabbi csv format)", "id-reason-time.csv");

    parser.addOption(file1Option);
    parser.addOption(file2Option);

    parser.process(app);

    QString fileFrom = parser.value(file1Option);
    QString fileTo = parser.value(file2Option);


    if (fileFrom.isEmpty() || fileTo.isEmpty()) {
        QTextStream(stdout) << "Error: Missing command line argument(s)." << ENDL;
        parser.showHelp(-2);
        return -2;
    }

    QString path = QFileInfo(fileTo).absolutePath();
    QString name = QFileInfo(fileTo).baseName();
    QString sfx = QFileInfo(fileTo).suffix();

    if (sfx.isEmpty()) {
        sfx = "csv";
    }

    fileTo = QDir::cleanPath(path + QDir::separator() + name + "." + sfx);

    QTextStream(stdout) << "Convert from " << fileFrom << " to " << fileTo << ENDL;

    long res = doConvert(fileFrom, fileTo);

    if (res > 0) {
        QTextStream(stdout) << "Convertion Completed successfully!" << ENDL;
    }

    return res;
}
