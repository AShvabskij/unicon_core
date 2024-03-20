#ifndef OSCDATASAVER_H
#define OSCDATASAVER_H

#include <QJsonObject>
#include "osc_types.h"

class OscDataStorage: public IOscDataStorageService
{
public:
    OscDataStorage() = default;
    ~OscDataStorage() override {}

    static OscDataStorage* instance() {
        static OscDataStorage m_instance;

        return &m_instance;
    }

    long save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data) override;
    long checkVersion(QString fileFrom) override;
    long loadHeader(QString fileFrom, DDE_OSC_HEADER &header)  override;
    long loadData(QString fileFrom, OscType::OscDataBuffer &data) override;

protected:
    virtual long checkVersion(int ver, int subVer);
    QJsonObject serializeToJSon(const OscType::OscDataBuffer &dat) const;
    QString createPath(const DDE_OSC_HEADER &header);
    long saveObj(const QString fileName, const QJsonObject &obj, bool useBinaryFormat = false);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    virtual long jsonToHeader(const QJsonObject& obj, DDE_OSC_HEADER &h);
    virtual long jsonToData(const QJsonObject& obj, OscType::OscDataBuffer &data);
    virtual long decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data);
};

#endif // OSCDATASAVER_H
