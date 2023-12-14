#ifndef OSCDATASAVER_H
#define OSCDATASAVER_H

#include <QJsonObject>
#include "osc_types.h"

class OscDataJSonStorage: public IOscDataStorageService
{
public:
    OscDataJSonStorage() = default;
    ~OscDataJSonStorage() override {}

    static OscDataJSonStorage* instance() {
        static OscDataJSonStorage m_instance;

        return &m_instance;
    }

    long save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data) override;
    long loadHeader(QString fileFrom, DDE_OSC_HEADER &header)  override;
    long loadData(QString fileFrom, OscType::OscDataBuffer* data) override;

private:
    QString createPath(const DDE_OSC_HEADER &header);
    long saveObj(const QString fileName, const QJsonObject &obj, bool useBinaryFormat = false);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    long jsonToHeader(const QJsonObject& obj, DDE_OSC_HEADER &h);
    long jsonToData(const QJsonObject& obj, OscType::OscDataBuffer &data);
    QString colorToString(const int &c);
};

#endif // OSCDATASAVER_H
