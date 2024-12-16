#include "datausbcopier.h"


DataUsbCopier::DataUsbCopier(const QString &sourceDir, const QString &usbMountPoint, QObject *parent) : QObject(parent)
{
    m_sourceDirPath = sourceDir;
    m_usbMountPath = usbMountPoint;

    m_watcher.addPath(m_usbMountPath);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &DataUsbCopier::onUsbConnected);
}

void DataUsbCopier::onUsbConnected()
{
    QDir usbDir(m_usbMountPath);
    QStringList subDirs = usbDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (!subDirs.isEmpty()) {
        QString usbDevicePath = m_usbMountPath + "/" + subDirs.first();
        qDebug() << "USB connected at:" << usbDevicePath;
        copyFilesToUsb(m_sourceDirPath, usbDevicePath);
    }

}

void DataUsbCopier::copyFilesToUsb(const QString &sourceDirPath, const QString &destinationPath)
{
    QDir sourceDir(sourceDirPath);
    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist:" << sourceDirPath;
        return;
    }

    QDir destDir(destinationPath);
    if (!destDir.exists()) {
        if (!destDir.mkpath(destinationPath)) {
            qDebug() << "Failed to create destination directory:" << destinationPath;
            return;
        }
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for (const QString& fileName : files) {
        QString srcFilePath = sourceDir.absoluteFilePath(fileName);
        QString destFilePath = destDir.absoluteFilePath(fileName);
        if (QFile::copy(srcFilePath, destFilePath)) {
            qDebug() << "Copied:" << srcFilePath << "to" << destFilePath;
        } else {
            qDebug() << "Failed to copy:" << srcFilePath;
        }
    }
}
