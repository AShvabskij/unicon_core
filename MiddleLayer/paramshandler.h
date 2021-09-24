#ifndef PARAMSHANDLER_H
#define PARAMSHANDLER_H

#include "basereqhandler.h"
#include <QTimer>

struct ParamValue
{
    QVariant value;
    qint8 valueFormat = 0;

    qlonglong timestamp = 0;
    float scale = 0.0;
    bool isValid() {
        return value.toInt() != -1;
    }

    QJsonObject toJson() const {
        QJsonObject el;
        el["value"] = value.toJsonValue();
        el["format"] = valueFormat;
        el["scale"] = scale;
        el["time"] = timestamp;

        return el;
    }
};
typedef QVector<ParamValue> ParamValueList;

struct Param
{
    int id = 0;
    int deviceId = 0;
    int moduleId = 0;
    QString name = "";
    QString desc = "";
    qint8 valueUnit = 0;

    bool operator == (const Param& p) const {
        return this->id == p.id && this->deviceId == p.deviceId && this->moduleId == p.moduleId;
    }

    static QString valueUnitToString(qint8 unit)
    {
        switch (unit) {
        case 1: return "A";
        case 2: return "V";
        case 3: return "W";
        case 4: return "С";
        case 5: return "S";
        };

        return "";
    }
};
typedef QVector<Param> ParamList;

class ParamsHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    ParamsHandler(IDDE* dde);
    virtual int handle(const QJsonObject& request);

signals:
    void requestStreamValue();

private slots:
    void onStreamTimerAlarm();

private:
    int handleGetHeader(const QJsonObject &request);
    int handleGetValue(const QJsonObject &request);
    int handleSetValue(const QJsonObject &request);
    int handleOpenStream(const QJsonObject &request);
    int handleCloseStream(const QJsonObject &request);

    long getParamValue(int deviceId, int paramId, ParamValue* out);
    long getParamHeader(int deviceId, int paramId, Param *out);
    long getParamHeaders(int deviceId, int moduleId, ParamList *out);

    ParamValue valueFrom(const GLIO_ELEMENT_VALUE &el);
    QJsonObject createHeaderObj(int requestId, const ParamList &params);
    QJsonObject createValueObj(int requestId, const Param& param, const ParamValue& value);
    QJsonObject createStreamValueObj(const Param& param, const ParamValue& value, int error = 0);

    void startPooling(int intervalMsc);
    void stopPooling();
    long streamParamsValue();
    void stopStreamsParamValue();
    void stopStreamParamValue(const Param &param);

    ParamList m_capturedParams;

    int m_streamValCount = 0;
    QTimer* m_streamTimer;
};

#endif // PARAMSHANDLER_H
