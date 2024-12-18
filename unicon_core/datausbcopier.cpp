#include "datausbcopier.h"

#include "osc_types.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>

DataUsbCopier::DataUsbCopier(IOscFileStorageService *storage, QObject *parent) : QObject(parent), m_storage(storage)
{
    // Watch for usb folder in Debian /media/username/USB_NAME

    QString userName =  getUsername();
    QString watchedPath = "/media/"+ userName;

    qDebug() << "watched path:" << watchedPath;

    m_watcher.addPath(watchedPath);
//  m_watcher.addPath("/run/media");

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &DataUsbCopier::onMediaChanged);

//    m_timer = new QTimer(this);
//    QObject::connect(m_timer, &QTimer::timeout, this, [this]() {
//        qDebug() << "monitor usb devices";
//        this->monitorUSBDevices();
//    });

    connect(this, SIGNAL(usbConnected(const QString&)), this, SLOT(onUsbConnected(const QString&)), Qt::QueuedConnection);
}

QString DataUsbCopier::getUsername()
{
    QProcess process;
    process.start("whoami", QStringList());
    process.waitForFinished();
    QString username = process.readAllStandardOutput().trimmed();
    return username;
}

void DataUsbCopier::monitorUSBDevices()
{
    QString usbPath = usbDevicePath();
    if (usbPath != "") {
        if (m_usbMountPath == usbPath) {
            return; // already done before
        }

        m_usbMountPath = usbPath;
        qDebug() << "usb is inserted, path:" << usbPath;

        emit usbConnected(usbPath);
    } else {
        m_usbMountPath = "";
        qDebug() << "usb is removed";
    }
}

void DataUsbCopier::onMediaChanged(const QString& )
{
    QTimer::singleShot(5000, [this]() {
        monitorUSBDevices();
    });

    return;
}

void DataUsbCopier::onUsbConnected(const QString& usbRootPath)
{
    //  QString usbPath = usbDevicePath();

    if (usbRootPath == "") {
        return;
    }

    QString destPath = usbRootPath + "/DataLogger";
    QDate date = QDateTime::currentDateTime().date().addDays(-90);

    qDebug() << "Copy data logger files from date:" << date.toString();

    while (date != QDateTime::currentDateTime().date()) {
        QString sourceDirPath = m_storage->getFolderPath(date.startOfDay());
        copyFilesToUsb(sourceDirPath, destPath);
        date = date.addDays(1);
    }
}

QString DataUsbCopier::usbDevicePath()
{
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (isUsbDrive(storage)) {
            return storage.rootPath();
        }
    }

    return "";

    // another way to get the path
    //    QDir usbDir(m_usbMountPath);
    //    QStringList subDirs = usbDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    //    if (!subDirs.isEmpty()) {
    //        QString usbDevicePath = m_usbMountPath + "/" + subDirs.first();
    //        qDebug() << "USB connected at:" << usbDevicePath;
    //    }
}

bool DataUsbCopier::isUsbDrive(const QStorageInfo &storage)
{
    // Check if the storage is valid, not read-only, and mounted
    if (!storage.isValid() || !storage.isReady() || storage.isReadOnly()) {
        return false;
    }

//  qDebug() << "storage name = " << storage.name() << "path = " << storage.rootPath() << "fsType = " << storage.fileSystemType() << "device = " << storage.device();

#ifdef __WIN32__
    // On Windows, check the device path for removable drives (e.g., E:, F:, etc.)
    if (storage.device().startsWith("\\\\.\\") || rootPath.startsWith("E:\\") || rootPath.startsWith("F:\\")) {
        return true; // Possible USB device on Windows
    } else {
        return false;
    }
#else
    // Check the root path for hints (Linux-specific)
    QString rootPath = storage.rootPath();
    QString device = storage.device();

    if (!device.startsWith("/dev/sdb") && !device.startsWith("/dev/sda")) {
        return false;
    }

    if (!rootPath.startsWith("/media") && !rootPath.startsWith("/run")) {
        return false;
    }

    // Additional checks could be done on file system type (e.g., FAT32, exFAT)
//    QString fsType = storage.fileSystemType();
//    // check file systems often found on USB drives
//    if (fsType != "vfat" && fsType != "exfat" && fsType != "ntfs") {
//        return false;
//    }

    return true;

#endif
}

void DataUsbCopier::copyFilesToUsb(const QString &sourceDirPath, const QString &destinationPath)
{
    QDir sourceDir(sourceDirPath);
    if (!sourceDir.exists()) {
//      qDebug() << "Source directory does not exist:" << sourceDirPath;
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
        QString srcFile = sourceDir.absoluteFilePath(fileName);
        QString destFile = destDir.absoluteFilePath(fileName);
        if (QFile::exists(destFile)) {
            continue;
        }

        if (QFile::copy(srcFile, destFile)) {
            qDebug() << "Copied:" << srcFile << "to" << destFile;
        } else {
            qDebug() << "Failed to copy:" << srcFile;
        }
    }
}

void DataUsbCopier::recursiveCopy(const QString& srcPath, const QString& dstPath)
{
    qDebug() << "Recursive copy from" << srcPath << "to" << dstPath;
    QDir().mkpath(dstPath); // be sure path exists

    const QDir srcDir(srcPath);
    foreach (const auto& dirName, srcDir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        recursiveCopy(srcPath + "/" + dirName, dstPath + "/" + dirName);
    }

    foreach (const auto& fileName, srcDir.entryList(QStringList(), QDir::Files, QDir::Name)) {
        QFile::copy(srcPath + "/" + fileName, dstPath + "/" + fileName);
        break; // temporarly
    }
}
