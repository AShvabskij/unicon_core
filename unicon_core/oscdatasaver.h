#ifndef OSCDATASAVER_H
#define OSCDATASAVER_H

#include <QJsonObject>
#include "osc_types.h"

class OscDataJSonFileSaver: public IOscDataSaver
{
public:
    OscDataJSonFileSaver() = default;
    ~OscDataJSonFileSaver() override {}

    static OscDataJSonFileSaver* instance() {
        static OscDataJSonFileSaver m_instance;

        return &m_instance;
    }

    long save(const DDE_OSC_HEADER &header, const OscType::OscDataBuffer &data) override;

private:
    long saveObj(const QString fileName, const QJsonObject &obj, bool useBinary = false);
    QJsonObject headerToJson(const DDE_OSC_HEADER &h);
    QString colorToString(const int &c);
};

#endif // OSCDATASAVER_H
