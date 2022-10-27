#ifndef PARAMSHANDLER_H
#define PARAMSHANDLER_H

#include "basereqhandler.h"

struct ParamID
{
    DevID devId;
    int moduleId;
    int id;

    bool isValid() const {
        return id != 0 && moduleId != 0 && devId.isValid();
    }

    int uid() const {
        return (moduleId << 6) + id;
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
};

typedef QVector<Param> ParamList;

struct ParamValue
{
    ParamID paramID;

    QVariant value;
    qlonglong timestamp = 0;

    int format = GLIO_ELEMENT_FORMAT_ENUM::FORMAT_UNDEFINED;
    float scale = 0.0;

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

    bool isValid() {
        paramID.devId.isValid() && paramID.id >= 0 && format != GLIO_ELEMENT_FORMAT_ENUM::FORMAT_UNDEFINED;
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
    ParamValueList getModuleValues(const ParamID& groupId, int &isOk);

    long getParamHeader(const ParamID& paramId, Param *out);
    long getParamHeaders(const DevID& deviceId, int moduleId, ParamList *out);
    long setParamValue(const ParamValue &value);

    ParamValue valueFrom(const ParamID &paramId, const GLIO_ELEMENT_VALUE &el);
    QJsonObject createHeaderObj(int requestId, const ParamList &params);
    QJsonObject createValueObj(int requestId, const ParamValue& value, int error = 0);
    QJsonObject createStreamValueObj(const ParamValue& value, int error = 0);

    void startPooling(int intervalMsc);
    void stopPooling();
    void streamParamsValue();
    void stopStreamsParamValue();
    void stopStreamParamValue(const Param &param);
    Q_SLOT void sendActualParamValue(const Param &param, int requestId, int error = 0);

    ParamList m_capturedParams;

    int m_streamValCount = 0;
    QTimer* m_streamTimer;
    mutable DDE_GET_PARAMS_HEADER* m_header = nullptr;
    mutable DDE_GET_PARAMS_DATA *m_data = nullptr;

};

#endif // PARAMSHANDLER_H
