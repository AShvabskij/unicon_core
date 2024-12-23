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

#include "oscdatalogger_v1_2.h"
#include "DDE_TYPES.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#define ENDL Qt::endl
#else
#define ENDL "\n"
#endif

const int DATA_VERSION = 1;
const int DATA_SUBVERSION = 2; // temp version for Israel 2024

long OscDataLogger_v1_2::checkVersion(int ver, int subVer)
{
    if (ver != DATA_VERSION) {
        return -1;
    }

    if (subVer != DATA_SUBVERSION) {
        return -1;
    }

    return _return_OK;
}

long OscDataLogger_v1_2::decodeData(const QCborValue &sourceDat, OscType::OscDataBuffer &data)
{
    QCborMap obj = sourceDat.toMap();
    uint8_t ver = obj.value("version").toVariant().toUInt();

    if (ver != DATA_VERSION) {
        QTextStream(stdout) << "The json data version " <<  ver << " is not supported" <<  ", the current supported version is " << DATA_VERSION << ENDL;
        return -1;
    }

    data.id =  obj.value("d_id").toInteger();
    data.timestamp = obj.value("time").toVariant().toLongLong();
    QCborArray vars = obj.value("vars").toArray();
    QCborArray values = obj.value("values").toArray();

    int maxValueCount = values.size() / vars.size();
    int valueCount = values.size() / vars.size();

    for (int i = 0; i < vars.size(); ++i) {
        for (int j = 0; j < valueCount; j++) {
            int ind = i*valueCount + j;
            QVariant val =  values[ind].toVariant();
            data.chArray[i].append(val);
        }

        data.chArray[i].varId = vars[i].toInteger();
        maxValueCount = maxValueCount < valueCount ? valueCount : maxValueCount;
    };

    data.valueCount = maxValueCount;

    return _return_OK;
}
