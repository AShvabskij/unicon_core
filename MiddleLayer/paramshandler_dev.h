#ifndef PARAMSHANDLER_H
#define PARAMSHANDLER_H

#include "basereqhandler.h"
#include <QTimer>

struct ParamValue
{
    QVariant value;
    uint8_t valueFormat = 0;
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

    ParamValue value;
};
typedef QVector<Param> ParamList;

class ParamsHandler : public BaseReqHandler
{
    Q_OBJECT
public:
    ParamsHandler();
    virtual int handle(const QJsonObject& request);

signals:
    void requestStreamValue();

private slots:
    void slotTimerAlarm();

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
    QJsonObject createStreamValueObj(const Param& param, const ParamValue& value);

    void startPooling();
    void stopPooling();
    long streamParamValue();
    void stopStreamParamValue(const Param &param);

    Param m_cupturedParam;
    ParamList m_cupturedParams;
    int m_streamValCount = 0;
    int m_requestId;
    QTimer* m_timer;
};

#endif // PARAMSHANDLER_H
