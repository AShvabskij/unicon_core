#ifndef OSCFILESTORAGE_V11_H
#define OSCFILESTORAGE_V11_H

#include "oscfilestorage.h"

class OscFileStorage_v1_2 : public OscFileStorage
{
public:
    OscFileStorage_v1_2() = default;

protected:
    virtual long checkVersion(int ver, int subVer) override;

    long decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data) override;

};

#endif // OSCFILESTORAGE_V11_H
