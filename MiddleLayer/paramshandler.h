#ifndef PARAMSHANDLER_H
#define PARAMSHANDLER_H

#include "basereqhandler.h"

struct Param
{
    int id = 0;
    int deviceId = 0;
    int moduleId = 0;
    QString name = "";
    QString desc = "";
    QString valueUnit = 0;
    QMap<int, QString> valueTexts;
    qint8 valueFormat = 0;
    float valueScale = 0.0;
    bool writable = false;

    bool operator == (const Param& p) const {
        return this->id == p.id && this->deviceId == p.deviceId && this->moduleId == p.moduleId;
    }
};
typedef QVector<Param> ParamList;

struct ParamValue
{
    int id = 0;
    int deviceId = 0;
    int moduleId = 0;

    QVariant value;
    qlonglong timestamp = 0;

    qint8 format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_UNDEFINED;
    float scale = 0.0;

    QJsonObject toJsonValue() const {
        QJsonObject el;
        el["value"] = value.toJsonValue();
        el["time"] = timestamp;

        return el;
    }
};
typedef QVector<ParamValue> ParamValueList;

class ParamsHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    ParamsHandler(IDDE* dde);
    ~ParamsHandler();

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

    long getParamValue(const Param &p, ParamValue* out);
    long getParamValue(int deviceId, int moduleId, int paramId, ParamValue* out);
    long getParamHeader(int deviceId, int moduleId, int paramId, Param *out);
    long getParamHeaders(int deviceId, int moduleId, ParamList *out);
    long setParamValue(const ParamValue &value);

    ParamValue valueFrom(int deviceId, int moduleId, const GLIO_ELEMENT_VALUE &el);
    QJsonObject createHeaderObj(int requestId, const ParamList &params);
    QJsonObject createValueObj(int requestId, const ParamValue& value, int error = 0);
    QJsonObject createStreamValueObj(const Param& param, const ParamValue& value, int error = 0);

    void startPooling(int intervalMsc);
    void stopPooling();
    long streamParamsValue();
    void stopStreamsParamValue();
    void stopStreamParamValue(const Param &param);
    Q_SLOT void sendEmptyResponse (const Param &param, int requestId, int error = 0);

    ParamList m_capturedParams;

    int m_streamValCount = 0;
    QTimer* m_streamTimer;
    mutable DDE_GET_PARAMS_HEADER* m_header = nullptr;
    mutable DDE_GET_PARAMS_DATA *m_data = nullptr;

};

#endif // PARAMSHANDLER_H
