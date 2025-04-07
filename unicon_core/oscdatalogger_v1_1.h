#ifndef OSCDATALOGGER_V11_H
#define OSCDATALOGGER_V11_H

#include <QJsonObject>
#include "oscdatalogger.h"

class OscDataLogger_v1_1: public OscDataLogger
{
public:
    OscDataLogger_v1_1() = default;
    virtual ~OscDataLogger_v1_1() {}

    static OscDataLogger_v1_1* instance() {
        static OscDataLogger_v1_1 m_instance;

        return &m_instance;
    }

    virtual long checkVersion(int ver, int subVer) override;

    long decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data) override;

private:
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit) const;

};

#endif
