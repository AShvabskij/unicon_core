#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#define MAX_DEV_SUPPORT  (32+1)

#include <QVariantList>
#include <QVector>
#include <QDateTime>
#include <QJsonObject>

#pragma once
#include "DDE_TYPES.h"

typedef quint16 DevInd;
typedef QList<quint16> DeviceIndList;


constexpr const char* sysTypeToString(SysType type) {
    switch (type) {
    case FILE_IO: return "FILE_IO"; // only for demo mode
    case UAVCAN: return "UAVCAN";
    case CANOPEN: return "CANOPEN";
    case MODBUS: return "MODBUS";
    case DLOG_CPLOT: return "DLOG_CPLOT";
    case DLOG_ISTART: return "DLOG_ISTART";
    case SysType_Unknown: return "Unknown";
    default: return "";
    }

    return "";
}

struct DevID
{
    SysType type;
    DevInd id; // todo rename to 'ind'

    bool isValid() const {
        return id <= MAX_DEV_SUPPORT && type != SysType::SysType_Undefined;
    }
};

bool operator==(const DevID& a, const DevID& b);

struct ParamID
{
    DevID devId;
    int moduleId;
    int id;

    bool isValid() const {
        return id != 0 && moduleId >= 0 && devId.isValid();
    }

    int uid() const {
        return (devId.id << 12) + (moduleId << 6) + id;
    }

    QString logStr() const {
        return QString("device id = %0, moduleId = %1, paramId = %2").arg(devId.id).arg(moduleId).arg(id);
    }
};

bool operator==(const ParamID& a, const ParamID& b);

struct Param
{
    ParamID ID = {DevID(), 0, 0};

    QString name = "";
    QString desc = "";
    QString valueUnit = "";
    QMap<int, QString> valueTexts;
    int valueFormat = 0;
    float valueScale = 0.0;
    bool writable = false;

    bool operator == (const Param& p) const {
        return this->ID == p.ID;
    }

    QJsonObject toJsonObject() const {
        QJsonObject obj;
        obj["device_id"] = this->ID.devId.id;
        obj["module_id"] = this->ID.moduleId;
        obj["param_id"] = this->ID.id;
        obj["u_id"] = this->ID.uid();

        obj["name"] = this->name;
        obj["desc"] = this->desc;
        obj["value_unit"] = this->valueUnit;
        obj["value_format"] = this->valueFormat;
        obj["value_scale"] = double(this->valueScale);
        obj["rw"] = this->writable ? "W" : "R";
        obj["value_texts"] = [](const QMap<int, QString>& txtValues ) {
            QJsonObject json;
            QMapIterator<int, QString> i(txtValues);
            while (i.hasNext()) {
                i.next();
                json.insert(QString::number(i.key()), i.value());
            }
            return json;
        }(this->valueTexts);

        return obj;
    }
};

typedef QVector<Param> ParamList;

struct Module
{
    int id = 0;
    int deviceId = 0;
    QString name;
    QString desc;
    ParamList params;
};

typedef QVector<Module> ModuleList;


struct Device
{
    DevID ID = {SysType::SysType_Undefined, 0};
    QString name; // name of similar devices : DEVICE_NAME + HW_REV + SW_REV, f.e. "DCDC00010002"
    QString instanceName; // unique device instance name: name + ID
    QString desc;
    SysType sysType  = SysType_Undefined;
    ModuleList modules;

    Device() = default;

    Device(DevID ID) {
        this->ID = ID;
        sysType = ID.type;
    }

    bool isValid() {
        return ID.isValid();
    }
    bool isEmpty() {
        return modules.count() == 0;
    }
};
typedef QVector<Device> DeviceList;

#endif // DEVICE_TYPES_H
