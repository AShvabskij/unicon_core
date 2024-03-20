#ifndef OSCDATASTORAGE_V11_H
#define OSCDATASTORAGE_V11_H

#include "oscdatastorage.h"

class OscDataStorage_v1_2 : public OscDataStorage
{
public:
    OscDataStorage_v1_2() = default;

protected:
    virtual long checkVersion(int ver, int subVer) override;

    long decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data) override;

};

#endif // OSCDATASTORAGE_V11_H
