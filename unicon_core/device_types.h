#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#define MAX_DEV_SUPPORT  (32+1)

//#include <QtTypes>
#include <QVariantList>
#include <QDateTime>

typedef quint16 DevInd;
typedef QList<quint16> DeviceIndList;

enum SysType
{
    Undefined = 0,
    DEFAULT = 0,
    FILE_IO = 1,
    UAVCAN = 2,
    CANOPEN = 3,
    MODBUS = 4,
    CONNEX_MVCP = 5,
    Unknown
};

struct DevID
{
    SysType type;
    DevInd id; // todo rename to 'ind'

    bool isValid() const {
        return id <= MAX_DEV_SUPPORT && type != SysType::Undefined;
    }
};

bool operator==(const DevID& a, const DevID& b);

#endif // DEVICE_TYPES_H
