#ifndef OSCDATASAVER_H
#define OSCDATASAVER_H

#include <QJsonObject>
#include "osc_types.h"

class OscFileStorage: public IOscFileStorageService
{
public:
    OscFileStorage() = default;
    virtual ~OscFileStorage() {}

    static OscFileStorage* instance() {
        static OscFileStorage m_instance;

        return &m_instance;
    }

    long save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data) override;
    long checkVersion(QString fileFrom) override;
    long loadHeader(QString fileFrom, DDE_OSC_HEADER &header)  override;
    long loadData(QString fileFrom, OscType::OscDataBuffer &data) override;
    long loadData(const DDE_OSC_HEADER& header, OscType::OscDataBuffer& data) override;
    QList<DDE_OSC_HEADER> headerList(QDate date) override;
    QString getFolderPath(const QDateTime dateTime) override;


protected:
    virtual long checkVersion(int ver, int subVer);
    QJsonObject serializeToJSon(const OscType::OscDataBuffer &dat) const;
    QString createFolder(const QDateTime dateTime);
    long saveObj(const QString fileName, const QJsonObject &obj, bool useBinaryFormat = false);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    virtual long jsonToHeader(const QJsonObject& obj, DDE_OSC_HEADER &h);
    virtual long jsonToData(const QJsonObject& obj, OscType::OscDataBuffer &data);
    virtual long decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data);

private:
    QStringList getSortedFilesByCreationDate(const QString &dirPath, QString mask);
};

#endif // OSCDATASAVER_H
