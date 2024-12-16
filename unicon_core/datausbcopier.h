#ifndef DATAUSBCOPIER_H
#define DATAUSBCOPIER_H

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QTimer>

class DataUsbCopier: public QObject
{
    Q_OBJECT

public:
    DataUsbCopier(const QString& sourceDir, const QString& usbMountPoint, QObject* parent = nullptr);

private slots:
    void onUsbConnected();

private:
    void copyFilesToUsb(const QString &sourceDirPath, const QString& destinationPath);

    QString m_sourceDirPath;
    QString m_usbMountPath;
    QFileSystemWatcher m_watcher;
};


#endif // DATAUSBCOPIER_H
