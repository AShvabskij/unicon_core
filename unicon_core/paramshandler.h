#ifndef PARAMSHANDLER_H
#define PARAMSHANDLER_H

#include "basereqhandler.h"
#include "device_types.h"

struct ParamValue
{
    ParamID paramID;

    QVariant value;
    qlonglong timestamp = 0;
    int error = 0;

    int format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_UNDEFINED;
    float scale = 0.0;
    static const int MAX_PARAM_VALUE_ACTUALITY_MS = 500;

    QJsonObject toJsonValue() const {
        QJsonObject el;
        el["value"] = value.toJsonValue();
        el["time"] = timestamp;

        return el;
    }

    ParamValue() = default;
    ParamValue(const Param& p) {
        paramID = p.ID;

        format = p.valueFormat;
        scale = p.valueScale;
    }

    bool isValid() const {
        return paramID.devId.isValid() && paramID.id >= 0 && format != GLIO_ELEMENT_FORMAT_ENUM::FORMAT_UNDEFINED;
    }

    bool isActual() const {
        auto actuality = QDateTime::currentMSecsSinceEpoch();
        actuality -= timestamp > 0 ? timestamp : actuality;

        if (actuality > MAX_PARAM_VALUE_ACTUALITY_MS) {
            return false;
        }

        return true;
    }

};
typedef QVector<ParamValue> ParamValueList;

class ParamsHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    ParamsHandler(IDDE_Dispatcher*);
    ~ParamsHandler();

    virtual int handle(const QJsonObject& request);
    virtual void handleClose();

signals:
    void requestStreamValue();

private slots:
    void onStreamTimerAlarm();

private:
    void handleGetHeader(const QJsonObject &request);
    void handleGetValue(const QJsonObject &request);
    void handleSetValue(const QJsonObject &request);
    void handleOpenStream(const QJsonObject &request);
    void handleCloseStream(const QJsonObject &request);
    void handleCloseAllStreams(const QJsonObject &request);

    long getParamValue(const Param &p, ParamValue* out);
    long getParamValue(const ParamID &paramId, ParamValue* out);
    ParamValueList getModuleValues(const DevID &devId, int moduleId, long &isOk);

    long getParamHeader(const ParamID& paramId, Param *out);
    long setParamValue(const ParamValue &value);
    long getModuleParams(const DevID& devId, int moduleId, DDE_GET_PARAMS_HEADER& ret);

    long convertValue(const ParamID &paramId, const GLIO_ELEMENT_VALUE &el, ParamValue *out);
    QJsonObject createHeaderObj(int requestId, const ParamList &params);
    QJsonObject createValueObj(int requestId, const ParamValue& value, int error = 0);
    QJsonObject createStreamValueObj(const ParamValue& value, int error = 0);

    void startPooling(int intervalMsc = 0);
    void stopPooling();
    void streamParamsValue();
    void stopStreamsParamValue();
    void stopStreamParamValue(const Param &param);
    Q_SLOT void sendActualParamValue(const Param &param, int requestId, int error = 0);

    ParamList m_capturedParams; // all params captured by opened streams
    QMap<int, DDE_GET_PARAMS_HEADER> m_capturedModules; // Modules (groups) of captured parameters

    int m_streamValCount = 0;
    QTimer* m_streamTimer;
    mutable DDE_GET_PARAMS_HEADER* m_header = nullptr;
    mutable DDE_GET_PARAMS_DATA *m_data = nullptr;

};

#endif // PARAMSHANDLER_H
