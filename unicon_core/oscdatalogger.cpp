#include "oscdatalogger.h"
#include "DDE_TYPES.h"

#include <QDateTime>
#include <QVariant>
#include <QtDebug>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QCborValue>
#include <QCborMap>
#include <QCborArray>
#include <QCborStreamReader>
#include <QCborStreamWriter>
#include <QDataStream>
#include <QDir>
#include <QElapsedTimer>

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define ENDL Qt::endl
#else
#define ENDL "\n"
#endif

const int DATA_VERSION = 1;
const int DATA_SUBVERSION = 1;

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
}

using namespace OscType;

long OscDataLogger::save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data)
{
    //  const char* home = getenv("HOME");
        QDateTime now = QDateTime::currentDateTime();
        QString path =  createFolder(now);
        QJsonObject jsonObj = headerToJson(header);

        QDateTime trigTime = QDateTime::fromSecsSinceEpoch(header.settings.trig_time, Qt::LocalTime);
        QString baseFileName = QString("%1-%2-%3").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh_mm_ss"));

        QString headerFile = path + "/" + baseFileName + ".hdr";

        _dde_func_return_t res = saveObj(headerFile, jsonObj);


        if (res != _return_OK) {
            return res;
        }

        QJsonObject datjsonObj = serializeToJSon(data);

        QString datFile = path + "/" + baseFileName + ".dat";
        QString lastDatFile = path + "/" + "last_data" + ".json";

        res = saveObj(datFile, datjsonObj, true);
        if (res == _return_OK) {
            // save data to json format for debugging purpose only
            res = saveObj(lastDatFile, datjsonObj, false);
        }

        if (res == _return_OK) {
            qInfo() << "Saved osc data, device id = " << header.device_id << " to file: " << headerFile << ENDL;
        }

        return res;
}

QString OscDataLogger::createFolder(const QDateTime dateTime)
{
    QString path = getFolderPath(dateTime);

    QDir dir;
    bool res = dir.mkpath(path);

    if (!res) {
        qWarning() << "Couldn't create folder:" + path;
    }

    return path;
}

QString OscDataLogger::getFolderPath(const QDateTime dateTime)
{
    QString year = "Y" + QString::number(dateTime.date().year());
    QString abbreviatedMonth = dateTime.toString("MMM");
    QString dayOfMonth = abbreviatedMonth + "_" + QString::number(dateTime.date().day());

    QString dataLoggerPath = getDataLoggerRootPath();
    QString path = dataLoggerPath + year + "/" + abbreviatedMonth + "/" + dayOfMonth;

    return path;
}

QString OscDataLogger::getDataLoggerRootPath()
{
    QString dataLoggerPath =  QString("./DataLogger/"); // qApp->applicationDirPath()
    return dataLoggerPath;
}

void OscDataLogger::cleanOldestData(const QString rootPath)
{
    QDir rootDir(rootPath);

    if (!rootDir.exists()) {
        qWarning() << "Root path does not exist:" << rootPath;
        return;
    }

    // Получаем список папок годов
    QStringList yearFolders = rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    if (yearFolders.isEmpty()) {
        qWarning() << "No year folders found in root path:" << rootPath;
        return;
    }

    // Находим самую старую папку года
    QString oldestYearFolder = yearFolders.first();
    QDir yearDir(rootDir.filePath(oldestYearFolder));

    // Получаем список папок месяцев
    QStringList monthFolders = yearDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    if (monthFolders.isEmpty()) {
//      qWarning() << "No month folders found in year folder:" << yearDir.path();
        return;
    }

    // Находим самый старый месяц
    QString oldestMonthFolder = monthFolders.first();
    QDir monthDir(yearDir.filePath(oldestMonthFolder));

    // Получаем список папок дней
    QStringList dayFolders = monthDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    if (dayFolders.isEmpty()) {
//      qWarning() << "No day folders found in month folder:" << monthDir.path();
        return;
    }

    // Находим самый старый день
    QString oldestDayFolder = dayFolders.first();
    QDir dayDir(monthDir.filePath(oldestDayFolder));

    // Удаляем все файлы в самой старой папке
    QFileInfoList files = dayDir.entryInfoList(QDir::Files);
    for (const QFileInfo &file : files) {
        if (!QFile::remove(file.filePath())) {
            qWarning() << "Failed to remove file:" << file.filePath();
        }
    }

    // Удаляем саму папку
    if (!dayDir.rmdir(dayDir.path())) {
        qWarning() << "Failed to remove folder:" << dayDir.path();
    }

    qInfo() << "Cleaned oldest folder:" << dayDir.path();
}

long OscDataLogger::saveObj(const QString fileName, const QJsonObject &obj, bool useBinaryFormat)
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

#ifdef __linux__
        iStream.setCodec( "utf-8" );
#else
        iStream.setEncoding(QStringConverter::Utf8);
#endif
        iStream << bytes;
    }

    file.close();
    return res;
}

QJsonObject OscDataLogger::headerToJson(const DDE_OSC_HEADER &h)
{
    QJsonObject res;

    res["version"] = DATA_VERSION;
    res["sub_version"] = DATA_SUBVERSION;
    res["device_id"] = h.device_id;
    res["id"] = h.device_id;
    res["trig_time"] = QString::number(h.settings.trig_time);
    res["reason"] = QString::number(h.settings.reason);
    res["resolution_us"] = QString::number(h.settings.time_resolution_us);

    QJsonArray channelsObj;
    for (int chInd = 0; chInd < h.settings.channels_count; chInd++) {
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

long OscDataLogger::checkVersion(QString fileFrom)
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
    file.close();

    QJsonDocument d = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject obj = d.object();

    int ver = obj["version"].toVariant().toInt();
    int sub_ver = obj["sub_version"].toVariant().toInt();

    long res = checkVersion(ver, sub_ver);

    return res;
}

long OscDataLogger::loadHeader(QString fileFrom, DDE_OSC_HEADER &header)
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
    file.close();

    QJsonDocument d = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject obj = d.object();

    long ret = jsonToHeader(obj, header);

    return ret;
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

long OscDataLogger::loadData(QString fileFrom, OscType::OscDataBuffer& data)
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
/*
    QJsonValue resValue = cborValue.toJsonValue();
    QJsonObject obj = resValue.toObject();
*/
    long ret = decodeData(cborValue, data);

    return ret;
}

long OscDataLogger::loadData(const DDE_OSC_HEADER& header, OscType::OscDataBuffer& data)
{
    QDateTime trigTime = QDateTime::fromSecsSinceEpoch(header.settings.trig_time, Qt::LocalTime);
    QString path =  getFolderPath(trigTime);
    QString baseFileName = QString("%1-%2-%3").arg(header.device_id).arg(header.settings.reason).arg(trigTime.toString("hh_mm_ss"));

    QString datFile = path + QDir::separator() + baseFileName + ".dat";

    long ret = loadData(datFile, data);

    if (ret == _return_FAIL && header.settings.reason == 0) {
        // Заплатка для случая, когда reason в заголовке (header.settings.reason) отсутствует

        QDir dir(path);
        QStringList fileList = dir.entryList(QStringList() << "*.dat", QDir::Files);
        for (QString fileName: fileList) {
            QString firstPart = QString("%1-").arg(header.device_id);
            int pos = fileName.indexOf(firstPart);
            if (fileName.contains(trigTime.toString("hh_mm_ss")) && pos == 0) {
                datFile = path + QDir::separator() + fileName;
                break;
            }
        }

        qDebug() << "datFile = " << datFile;
        if (!datFile.isEmpty()) {
            ret = loadData(datFile, data);
        }
    }

    return ret;
}

QList<DDE_OSC_HEADER> OscDataLogger::headerList(QDate date)
{
    QList<DDE_OSC_HEADER> headers;
    QString path =  getFolderPath(QDateTime(date, QTime()));

    QStringList fileList = getSortedFilesByCreationDate(path, "*.hdr");

    for (QString file: fileList) {
        DDE_OSC_HEADER hdr;
        file = path + QDir::separator() + file;
        long res = loadHeader(file, hdr);
        if (res != _return_OK) {
            continue;
        }

        headers.append(hdr);
    }

    return headers;
}

QStringList OscDataLogger::getSortedFilesByCreationDate(const QString &dirPath, QString mask)
{
    QDir dir(dirPath);

    // Check if directory exists
    if (!dir.exists()) {
        return QStringList();
    }

    // Get the list of files with a specific extension (e.g., *.hdr)
    QStringList fileList = dir.entryList(QStringList() << mask, QDir::Files);

    // Create a list of QFileInfo objects to hold file information
    QList<QFileInfo> fileInfoList;
    for (const QString &fileName : fileList) {
        QFileInfo fileInfo(dir.absoluteFilePath(fileName));
        fileInfoList.append(fileInfo);
    }

    // Sort the list by creation date and time
    std::sort(fileInfoList.begin(), fileInfoList.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.birthTime() > b.birthTime();
    });

    // Convert the sorted QFileInfo list back to a QStringList
    QStringList sortedFileList;
    for (const QFileInfo &fileInfo : fileInfoList) {
        sortedFileList.append(fileInfo.fileName());
    }

    return sortedFileList;
}

QJsonObject OscDataLogger::serializeToJSon(const OscDataBuffer& dat) const
{
    QJsonObject res;
    QJsonArray allValues;
    QList<int> varIdList;
    QJsonArray varIdListObj;

    res["version"] = DATA_VERSION;
    res["sub_version"] = DATA_SUBVERSION;

    res["d_id"] = dat.id;
    res["time"] = dat.timestamp;

    for (const OscChannelValues& chVal : dat.chArray) {
        if (chVal.varId == 0) continue;

        varIdList << chVal.varId;
        varIdListObj << chVal.varId;
    }
    res["vars"] = varIdListObj;

    for (const OscChannelValues& chVal : dat.chArray) {
        QJsonArray valuesObj;

        if (!varIdList.contains(chVal.varId))
                continue;

        switch (chVal.type) {
            case OSC_VAR_INT:
                for (int i=0; i< chVal.intValues.count(); i++) {
                    valuesObj << chVal.intValues[i];
                } break;
            case OSC_VAR_FLOAT:
                for (int i=0; i< chVal.fltValues.count(); i++) {
                    valuesObj << chVal.fltValues[i];
                } break;

            case OSC_VAR_DISCRETE:
                for (int i=0; i< chVal.discrValues.count(); i++) {
                    valuesObj << chVal.discrValues[i];
                }
            case UNDEFINED: {}
        };

        allValues.append(valuesObj);
    }

    res["values"] = allValues;

    return res;
}

long OscDataLogger::checkVersion(int ver, int subVer)
{
    if (ver != DATA_VERSION) {
        return -1;
    }

    if (subVer != DATA_SUBVERSION) {
        return -1;
    }

    return _return_OK;
}

long OscDataLogger::jsonToHeader(const QJsonObject& obj, DDE_OSC_HEADER &h)
{
    h.device_id = obj["device_id"].toVariant().toInt();
    h.settings.reason = obj["reason"].toVariant().toInt();
    h.settings.trig_time = obj["trig_time"].toVariant().toInt();
    h.settings.triger_mode = obj["trig_mode"].toVariant().toInt();
    h.settings.time_resolution_us = obj["resolution_us"].toVariant().toInt();

    QJsonArray arr = obj["channels"].toArray();
    h.settings.channels_count = arr.count();
    for (int ind = 0; ind < h.settings.channels_count; ind++) {
        QJsonObject elem = arr[ind].toObject();
        auto& ch = h.channels[ind];

        ch.chNum = elem["ch_num"].toInt();
        ch.firstBit = elem["first_bit"].toInt();
        ch.lastBit = elem["last_bit"].toInt();
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

long OscDataLogger::jsonToData(const QJsonObject& obj,  OscType::OscDataBuffer &data)
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
        data.chArray[i].varId = vars[i].toInt();
        int valueCount = 0;
        data.chArray[i].append(values[i].toArray().toVariantList());
        valueCount = data.chArray[i].count();

        maxValueCount = maxValueCount < valueCount ? valueCount : maxValueCount;
    };


    data.valueCount = maxValueCount;

    return _return_OK;
}

long OscDataLogger::decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data)
{
    QCborMap obj = sourceDat.toMap();
    uint8_t ver = obj.value("version").toVariant().toUInt();
    uint8_t sub_ver = obj.value("version").toVariant().toUInt();

    if (ver != DATA_VERSION) {
        QTextStream(stdout) << "The json data version " <<  ver << " is not supported" <<  ", the current supported version is " << DATA_VERSION << ENDL;
        return -1;
    }

    if (sub_ver > DATA_SUBVERSION) {
        QTextStream(stdout) << "The json data version " << sub_ver <<  " is an older version of the current version " << DATA_SUBVERSION << ENDL;
    }

    data.id =  obj.value("d_id").toInteger();
    data.timestamp = obj.value("time").toVariant().toLongLong();
    QCborArray vars = obj.value("vars").toArray();
    QCborArray values = obj.value("values").toArray();

    QElapsedTimer timer;
    timer.start();

    int maxValueCount = 0;
    for (int i = 0; i < vars.size(); ++i) {
        data.chArray[i].varId = vars[i].toInteger();
        int valueCount = 0;
        data.chArray[i].append(values[i].toArray());
        valueCount = data.chArray[i].count();
/*
        if (data.chArray[i].type == OSC_VAR_DISCRETE)
            break;
*/
        maxValueCount = maxValueCount < valueCount ? valueCount : maxValueCount;
    };

    data.valueCount = maxValueCount;

    qDebug() << "Decode data from cbor file:"
             << "device ind =" << data.id
             << "timestamp =" << data.trig_time
             << "reason =" << data.reason
             << "vars =" << vars.size()
             << "values =" << data.valueCount
             << "took" << timer.elapsed() << "ms";


    return _return_OK;
}

