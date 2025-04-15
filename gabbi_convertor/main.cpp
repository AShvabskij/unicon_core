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

#include <oscdatalogger.h>
#include <oscdatalogger_v1_1.h>
#include <oscdatalogger_v1_2.h>

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

using namespace OscType;

OscType::OscDataBuffer* createDataBuffer(const DDE_OSC_HEADER &hdr)
{
    OscType::OscDataBuffer* buff = new OscType::OscDataBuffer();

    buff->id = hdr.device_id;
    buff->timestamp = hdr.settings.trig_time;
    buff->trig_time = hdr.settings.trig_time;

    for (int ind = 0; ind < OSC_MAX_VARS; ind++) {
        const OSC_VAR& var = hdr.vars[ind];

        if (!var.isValid()) continue;

        OscType::OscChannelVar& chVar = buff->vars[ind];
        chVar.channelNum = var.chNum;
        chVar.varId = var.var.id;
        chVar.type = var.var.type;
        chVar.scale = var.gain;
        chVar.offset = var.offset;
        chVar.firstBit = var.firstBit;
        chVar.lastBit = var.lastBit;

        OscChannelData& chValues = buff->data[var.chNum];
        chValues.type = var.var.type;

        chValues.reserve(MAX_DATA_COUNT);
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

bool hasOnlyOneBit(int n)
{
    return n != 0 && (n & (n - 1)) == 0;
}

long generateContent(const DDE_OSC_HEADER& hdr, const OscType::OscDataBuffer& datBuff, QTextStream& stream)
{
    QTextStream& res = stream;

    QDateTime trigTime = QDateTime::fromSecsSinceEpoch(hdr.settings.trig_time);
    QString plotName = QString("%1_%2_%3").arg(hdr.device_id).arg(hdr.settings.reason).arg(trigTime.toString("hh:mm:ss:zzz"));
    double resolution_sec = (double)hdr.settings.time_resolution_us / (1000 * 1000);

    res << ".PlotName," << plotName << "," << ENDL;
    QDateTime date = QDateTime::fromSecsSinceEpoch(hdr.settings.trig_time);
    res << ".Date," << date.date().toString("yyyy-MM-dd") << "," << ENDL;
    res << ".Time," << date.time().toString("hh:mm:ss:zzz") << "," << ENDL;
    res << ".Ts," << resolution_sec << ","  << ENDL;

    res << ENDL;

    int numOfSet = 1;
    QStringList varIndexes;
    QStringList varNames;
    QStringList varNumOfSets;

    for (int i= 0; i < OSC_MAX_VARS; ++i) {
        auto& var = hdr.vars[i];

        if (!var.isValid())
            continue;

        int chNumOfSet = (var.chNum + 1) - (numOfSet - 1) * SET_SIZE;

        QStringList line;

        QRgb rgb = var.var.color;

        if (var.var.type == OSC_VAR_FLOAT || var.var.type == OSC_VAR_INT) {
            int lastBit = (var.lastBit > 0 && var.lastBit < MAX_BIT_NUM) ? var.lastBit : MAX_BIT_NUM;
            float gain = (var.gain != 0.0 && var.gain != 1.0) ? var.gain : var.var.__rm__scale;

            line << QString("@") + QString(var.var.name)
                 << QString("L") + QString::number(numOfSet)
                 << QString::number(chNumOfSet).rightJustified(2, '0')
                 << QString("D") + QString::number(var.firstBit).rightJustified(2, '0')
                 << QString("D") + QString::number(lastBit).rightJustified(2, '0')
                 << QString::number(gain) << QString::number(var.offset)
                 << QString::number(qRed(rgb)) + " " + QString::number(qGreen(rgb)) + " " + QString::number(qBlue(rgb))
                 << "TRUE";
        } else if (var.var.type == OSC_VAR_DISCRETE) {

            line << QString("&") + QString(var.var.name)
                 << QString("L") + QString::number(numOfSet)
                 << QString::number(chNumOfSet).rightJustified(2, '0')
                 << QString("D") + QString::number(var.firstBit).rightJustified(2, '0')
                 << QString("BIT")
                 << QString::number(qRed(rgb)) + " " + QString::number(qGreen(rgb)) + " " + QString::number(qBlue(rgb))
                 << "TRUE";
        }

        res << line.join(SEP) << ENDL;

        varIndexes << QString::number(i);
        varNames << var.var.name;
        varNumOfSets <<  QString("L") + QString::number(numOfSet) + "_" + QString::number(chNumOfSet).rightJustified(2, '0');
    }

    res << ENDL << ENDL;

    res << "* " << SEP << varIndexes.join(SEP) << ENDL;
    res << "* " << SEP << varNames.join(SEP)  << ENDL;
    res << "* " << SEP << varNumOfSets.join(SEP)  << ENDL;

    QMap<int/*channel*/, QVariant/*value*/> chValues;

    for (int i = 0; i < datBuff.valueCount; i++) {
        QStringList rec;
        rec << QString::number(i + 1);

        for (int ind = 0; ind < OSC_MAX_VARS; ind++) {
            const OscType::OscChannelVar& var = datBuff.vars[ind];
            const OscType::OscChannelData& chDat = datBuff.data[var.channelNum];

            if (!var.isValid()) continue;

            switch (var.type) {
            case OSC_VAR_INT:
            case OSC_VAR_FLOAT: {
                QVariant val = chDat.value(i);
                int n = denormalizeValue(val.toFloat());
                chValues[var.channelNum] = n;
            } break;

            case OSC_VAR_DISCRETE: {
                int discrValue = chDat.value(i).toInt();
                chValues[var.channelNum] = discrValue; // 04.2025 The decision for a newer version of the data buff

            } break;
            case UNDEFINED: {}
            };
        }

        for (int key : chValues.keys() ) {
            QVariant value = chValues[key];
            rec << QString::number(value.toInt());
        }

        res << rec.join(",") << ENDL;
    }

    return 1;
}

long doConvert(QString fileFrom, QString fileTo)
{
    QList<IOscDataLogger*> datServiceCollection;
    datServiceCollection.append(new OscDataLogger());
    datServiceCollection.append(new OscDataLogger_v1_1());
    datServiceCollection.append(new OscDataLogger_v1_2());

    long res = _return_OK;
    bool isHandled = false;
    for (IOscDataLogger* datService : datServiceCollection) {

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
        iStream.setEncoding(QStringConverter::Utf8);

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
