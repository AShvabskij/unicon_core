#ifndef DDE_DISPATCHER_H
#define DDE_DISPATCHER_H

#include <QMap>

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

#define MAX_DEV_SUPPORT  (32+1)
typedef quint16 DevInd;

struct DevID
{
    SysType type;
    DevInd id; // todo rename to 'ind'

    bool isValid() const {
        return id <= MAX_DEV_SUPPORT && type != SysType::Undefined;
    }
};

bool operator==(const DevID& a, const DevID& b);

class IDDE;

class  IDDE_Dispatcher
{
public:
    virtual ~IDDE_Dispatcher() {};
    virtual void registerDDE(SysType sysInterface, IDDE* dde) = 0;

    virtual void setDefaultDDE(IDDE* dde) = 0;
    virtual IDDE* dde(SysType sysInterface) = 0;
    virtual IDDE* operator() (SysType sysInterface = SysType::Undefined) = 0;
};

class DDE_Dispatcher : public IDDE_Dispatcher
{
public:
    DDE_Dispatcher() = default;
    virtual ~DDE_Dispatcher();
    virtual void registerDDE(SysType sysInterface, IDDE* dde);
    virtual IDDE* dde(SysType sysInterface);
    virtual void setDefaultDDE(IDDE* dde) {
        Q_ASSERT(dde);
        m_defDDE = dde;
    }

    IDDE* operator() (SysType sysType = SysType::Undefined) {
        if (m_ddeList.contains(sysType)) {
            return m_ddeList.value(sysType);
        } else {
//          Q_ASSERT(false);
            return m_defDDE;
        }
    }

private:

    QMap<SysType, IDDE*> m_ddeList;
    IDDE* m_defDDE;
};

#endif // DDE_DISPATCHER_H
