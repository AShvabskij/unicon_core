#include "oscdatalogger_v1_1.h"
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


using namespace OscType;

long OscDataLogger_v1_1::checkVersion(int ver, int subVer)
{
    if (ver != DATA_VERSION) {
        return -1;
    }

    if (subVer != DATA_SUBVERSION) {
        return -1;
    }

    return _return_OK;
}

long OscDataLogger_v1_1::decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data)
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

    QMap<int/*var_id*/, int/*bitNum*/> discrVarsBits;

    int maxValueCount = 0;
    for (int i = 0; i < vars.size(); ++i) {
        int valueCount = 0;
        const OscChannelVar& var = data.var(vars[i].toInteger());
        if (!var.isValid()) continue;

        const QCborArray& varValues = values[i].toArray();

        if (var.type == OSC_VAR_DISCRETE) {
            QVariantList values = data.data[var.channelNum].values();
            if (values.empty()) {
                values = QVariantList(varValues.size(), 0);
            }

            for (int ii = 0; ii < varValues.size(); ii++) {
                int bitValue = varValues[ii].toInteger();
                if (bitValue != 0) {
                    int bitNum = var.firstBit;
                    int bitMask = 1 << bitNum;
                    int oldValue = values[ii].toInt();

                    QVariant newValue = (oldValue | bitMask);
                    values[ii] = newValue;
                }
            }

            data.data[var.channelNum].clear();
            data.data[var.channelNum].append(values);

        } else {
            data.data[var.channelNum].clear();
            data.data[var.channelNum].append(varValues);
        }

        valueCount = data.data[var.channelNum].count();
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
