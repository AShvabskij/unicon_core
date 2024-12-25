#ifndef OSCDATALOGGER_V12_H
#define OSCDATALOGGER_V12_H

#include "oscdatalogger.h"

class OscDataLogger_v1_2 : public OscDataLogger
{
public:
    OscDataLogger_v1_2() = default;

protected:
    virtual long checkVersion(int ver, int subVer) override;

    long decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data) override;

};

#endif // OSCDATALOGGER_V12_H
