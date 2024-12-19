#include "datausbcopier.h"

#include "osc_types.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QTimer>

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
    connect(this, SIGNAL(errorDiskFull(const QString&)), this, SLOT(onDiskFullError(const QString&)), Qt::QueuedConnection);
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

    QDate date = QDateTime::currentDateTime().date().addDays(-90);

    copyFilesToUsb(date, usbRootPath);
}

void DataUsbCopier::copyFilesToUsb(const QDate startDate, const QString& usbRootPath)
{
    qDebug() << "Copy data logger files from date:" << startDate.toString();

    QDate date = startDate;
    while (date != QDateTime::currentDateTime().date().addDays(1)) {
        QString sourceDirPath = m_storage->getFolderPath(date.startOfDay());
        copyFilesWithStructure(sourceDirPath, usbRootPath);
        date = date.addDays(1);
    }
}

void DataUsbCopier::onDataSaved(quint16 device_id)
{
    qDebug() << "On data saved, dev id =" << device_id;

    QDate date = QDateTime::currentDateTime().date();

    QTimer::singleShot(5000, [this, date]() {
        if (m_usbMountPath == "") {
            m_usbMountPath = usbDevicePath();
        }

        if (m_usbMountPath != "") {
            copyFilesToUsb(date, m_usbMountPath);
        }
    });

    QString dataPath = m_storage->getDataLoggerRootPath();

    checkDiskSpace(dataPath);

    return;
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

    QString rootPath = storage.rootPath();
    QString device = storage.device();

#ifdef __WIN32__
    // On Windows, check the device path for removable drives (e.g., E:, F:, etc.)
    if (storage.device().startsWith("\\\\.\\") || rootPath.startsWith("E:\\") || rootPath.startsWith("F:\\")) {
        return true; // Possible USB device on Windows
    }
    return false;
#else
    // Check the root path for hints (Linux-specific)

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

void DataUsbCopier::copyFilesWithStructure(const QString &sourceDirPath, const QString &destRootPath)
{
    QDir sourceDir(sourceDirPath);
    if (!sourceDir.exists()) {
//      qDebug() << "Source directory does not exist:" << sourceDirPath;
        return;
    }

    QDir destRootDir(destRootPath);
    if (!destRootDir.exists()) {
        if (!destRootDir.mkpath(destRootPath)) {
            qDebug() << "Failed to create destination directory:" << destRootPath;
            return;
        }
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for (const QString& fileName : files) {

        QString relativePath = destRootDir.relativeFilePath(sourceDirPath);
        QString destFileDir = destRootPath + "/" + relativePath;

        // Recreate the target dir structure based on the source dir structure
        QDir destSubDir(destFileDir);
        if (!destSubDir.exists()) {
            if (!destSubDir.mkpath(destFileDir)) {
                qWarning() << "Failed to create directory:" << destFileDir;
            }
        }

        QString srcFile = sourceDir.absoluteFilePath(fileName);
        QString destFile = destSubDir.absoluteFilePath(fileName);

        if (QFile::exists(destFile)) {
            continue;
        }

        if (QFile::copy(srcFile, destFile)) {
            qDebug() << "Copied:" << srcFile << "to" << destFile;
        } else {
//          qDebug() << "Failed to copy:" << srcFile;
            copyFileWithErrorCheck(srcFile, destFile);
        }
    }
}

bool DataUsbCopier::copyFileWithErrorCheck(const QString &sourcePath, const QString &destinationPath)
{
    QFile sourceFile(sourcePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open source file:" << sourceFile.errorString();
        return false;
    }

    QFile destinationFile(destinationPath);
    if (!destinationFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open destination file:" << destinationFile.errorString();
        return false;
    }

    const qint64 bufferSize = 4096; // Buffer size for copying
    char buffer[bufferSize];
    qint64 bytesRead;

    while ((bytesRead = sourceFile.read(buffer, bufferSize)) > 0) {
        if (destinationFile.write(buffer, bytesRead) == -1) {
            qWarning() << "Write error:" << destinationFile.errorString();
            if (destinationFile.error() == QFileDevice::ResourceError) {
                qWarning() << "Disk full error detected!";
                QString dataPath = m_usbMountPath + "/DataLogger";
                emit errorDiskFull(dataPath);
            }
            return false;
        }
    }

    if (bytesRead == -1) {
        qWarning() << "Read error:" << sourceFile.errorString();
        return false;
    }

    return true;
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

bool DataUsbCopier::checkDiskSpace(const QString rootfolder)
{
    QStorageInfo storageInfo(rootfolder);

    if (storageInfo.isValid() && storageInfo.isReady()) {
        qint64 freeBytes = storageInfo.bytesAvailable();  // Free space available for the current user
        qint64 totalBytes = storageInfo.bytesTotal();     // Total space on the storage device
        qint64 usedBytes = totalBytes - freeBytes;        // Used space

        const double freeSpaceProc = (double)usedBytes/(double)totalBytes;
        if (freeSpaceProc < 0.2) {
            qInfo() << "The disk free space is not enough, try to clean the oldest folder with log data";

            qDebug() << "Path:" << rootfolder;
            qDebug() << "Total Space:" << totalBytes / (1024 * 1024) << "MB";
            qDebug() << "Used Space:" << usedBytes / (1024 * 1024) << "MB";
            qDebug() << "Free Space:" << freeBytes / (1024 * 1024) << "MB";

            emit errorDiskFull(rootfolder);
            return false; // need to clear data logs
        }
    } else {
        qDebug() << "Storage information is not valid or not ready for path:" << rootfolder;
        return false; // do nothing
    }

    return true;
}

void DataUsbCopier::onDiskFullError(const QString& dataPath)
{
    m_storage->cleanOldestData(dataPath);
}

