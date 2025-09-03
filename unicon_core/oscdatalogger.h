#ifndef OSCDATALOGGER_H
#define OSCDATALOGGER_H

#include <QJsonObject>
#include "osc_types.h"

class OscDataLogger: public IOscDataLogger
{
public:
    OscDataLogger() = default;
    virtual ~OscDataLogger() {}

    static OscDataLogger* instance() {
        static OscDataLogger m_instance;

        return &m_instance;
    }

    long save(const DDE_OSC_HEADER& header, SysType type, const OscType::OscDataBuffer& data) override;
    long load(const DDE_OSC_HEADER& header, SysType type, /*out*/OscType::OscDataBuffer& data) override;

    long checkVersion(QString fileFrom) override;
    long loadHeader(QString fileFrom, DDE_OSC_HEADER &header)  override;
    long loadData(QString fileFrom, OscType::OscDataBuffer &data) override;
    QList<DDE_OSC_HEADER> headerList( const DevID& devID, QDate date) override;
    QString getFolderPath(const QDateTime dateTime) override;
    QString getDataLoggerRootPath() override;

    void cleanOldestData(const QString rootPath) override;

protected:
    virtual long checkVersion(int ver, int subVer);
    QJsonObject serializeToJSon(const OscType::OscDataBuffer &dat) const;
    QString createFolder(const QDateTime dateTime);
    long saveObj(const QString filePath, const QJsonObject &obj, bool useBinaryFormat = false);
    QJsonObject loadObj(QString filePath);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h, SysType type);
    virtual long jsonToHeader(const QJsonObject& obj, DDE_OSC_HEADER &h);
    virtual long jsonToData(const QJsonObject& obj, OscType::OscDataBuffer &data);
    virtual long decodeData(const QCborValue& sourceDat,  OscType::OscDataBuffer &data);

private:
    QStringList getSortedFilesByCreationDate(const QString &dirPath, QString mask);
    qint32 discreteValue(qint32 rawValue, qint8 firstBit, qint8 lastBit) const;
};

#endif // OSCDATALOGGER_H
