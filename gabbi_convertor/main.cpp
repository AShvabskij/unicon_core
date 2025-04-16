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

    int numOfSet = 1;

    for (int ind = 0; ind < OSC_MAX_VARS; ind++) {
        const OSC_VAR& var = hdr.vars[ind];

        if (!var.isValid()) continue;

        OscType::OscChannelVar& chVar = buff->vars[ind];
        chVar.varName = var.var.name;
        chVar.channelNum = var.chNum;
        chVar.varId = var.var.id;
        chVar.type = var.var.type;
        chVar.scale = var.gain;
        chVar.offset = var.offset;
        chVar.firstBit = var.firstBit;
        chVar.lastBit = var.lastBit;

        if (var.setLn == 0 && var.setCh == 0) {
            int chNumOfSet = (var.chNum + 1) - (numOfSet - 1) * SET_SIZE;
            chVar.setLn = numOfSet;
            chVar.setCh = chNumOfSet;
        } else {
            chVar.setLn = var.setLn;
            chVar.setCh = var.setCh;
        }

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
    res << ".Ts," << QString::number(resolution_sec, 'f') << ","  << ENDL;

    res << ENDL;

    QStringList varIndexes;
    QStringList varNames;
    QMap<QString/*set*/, int/*chNum*/> varSets;
    QList<int> analogChannels;
    QList<int> discreteChannels;

    for (int i= 0; i < OSC_MAX_VARS; ++i) {
        auto& var = datBuff.vars[i];

        if (!var.isValid()) continue;

        if (var.type == OSC_VAR_DISCRETE) continue;

        QStringList line;

        QRgb rgb = var.color;
        float gain = var.scale;

        line << QString("@") + QString(var.varName)
             << QString("L") + QString::number(var.setLn)
             << QString::number(var.setCh).rightJustified(2, '0')
             << QString("D") + QString::number(var.firstBit).rightJustified(2, '0')
             << QString("D") + QString::number(MAX_BIT_NUM).rightJustified(2, '0')
             << QString::number(gain)
             << QString::number(var.offset)
             << QString::number(qRed(rgb)) + " " + QString::number(qGreen(rgb)) + " " + QString::number(qBlue(rgb))
             << "TRUE";

        res << line.join(SEP) << ENDL;

        varIndexes << QString::number(i);
        varNames << var.varName;
        QString set = QString("L") + QString::number(var.setLn) + "_" + QString::number(var.setCh).rightJustified(2, '0');
        varSets.insert(set, var.channelNum);
        analogChannels << var.channelNum;
    }

    for (int i= 0; i < OSC_MAX_VARS; ++i) {
        auto& var = datBuff.vars[i];

        if (!var.isValid())
            continue;

        if (var.type != OSC_VAR_DISCRETE) continue;

        QStringList line;

        QRgb rgb = var.color;

        line << QString("&") + QString(var.varName)
             << QString("L") + QString::number(var.setLn)
             << QString::number(var.setCh).rightJustified(2, '0')
             << QString("D") + QString::number(var.firstBit).rightJustified(2, '0')
             << QString("BIT")
             << QString::number(qRed(rgb)) + " " + QString::number(qGreen(rgb)) + " " + QString::number(qBlue(rgb))
             << "TRUE";

        res << line.join(SEP) << ENDL;

        varIndexes << QString::number(i);
        varNames << var.varName;

        QString set = QString("L") + QString::number(var.setLn) + "_" + QString::number(var.setCh).rightJustified(2, '0');
        varSets.insert(set, var.channelNum);

        if (!discreteChannels.contains(var.channelNum)) {
            discreteChannels << var.channelNum;
        }
    }

    res << ENDL << ENDL;

    QStringList stringChannelList;
    for (int i = 0; i < analogChannels.size(); ++i) {
        stringChannelList.append(QString::number(analogChannels.at(i)));
    }

    for (int i = 0; i < discreteChannels.size(); ++i) {
        stringChannelList.append(QString::number(discreteChannels.at(i)));
    }

    res << "* " << SEP << stringChannelList.join(SEP) << ENDL;
    res << "* " << SEP << varNames.join(SEP)  << ENDL;
    res << "* " << SEP << varSets.keys().join(SEP)  << ENDL;

    for (int i = 0; i < datBuff.valueCount; i++) {
        QStringList rec;
        rec << QString::number(i + 1);

        for (auto key: varSets.keys()) {
            int chNum = varSets[key];
            const OscType::OscChannelData& chDat = datBuff.data[chNum];

            if (analogChannels.contains(chNum)) {
                auto rawVal = chDat.value(i);
                int val = denormalizeValue(rawVal.toFloat());
                rec << QString::number(val);
            } else if (discreteChannels.contains(chNum)) {
                int discrValue = chDat.value(i).toInt();
                rec << QString::number(discrValue); // 04.2025 The decision for a newer version of the data buff
            }
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
