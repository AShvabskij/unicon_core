#ifndef DATAUSBCOPIER_H
#define DATAUSBCOPIER_H

#include <QCoreApplication>
#include <QStorageInfo>
#include <QFileSystemWatcher>

class IOscFileStorageService;

class DataUsbCopier: public QObject
{
    Q_OBJECT

public:
    DataUsbCopier(IOscFileStorageService *storage, QObject* parent = nullptr);
    void monitorUSBDevices();

signals:
    void usbConnected(const QString& usbPath);

private slots:
    void onMediaChanged(const QString&);
    void onUsbConnected(const QString& usbRootPath);

private:
    void copyFilesToUsb(const QString &sourceDirPath, const QString& destinationPath);
    void recursiveCopy(const QString& srcPath, const QString& dstPath);
    bool isUsbDrive(const QStorageInfo& storage);
    QString getUsername();

    QString usbDevicePath();

    QString m_sourceDirPath = "";
    QString m_usbMountPath = "";
    QFileSystemWatcher m_watcher;

    IOscFileStorageService* m_storage;
};

// For a more robust solution, consider integrating libudev to directly interact with the Linux device subsystem
//class UdevMonitor : public QThread {
//    Q_OBJECT

//public:
//    explicit UdevMonitor(QObject *parent = nullptr) : QThread(parent) {}
//    ~UdevMonitor() override { stopMonitoring(); }

//    void stopMonitoring() {
//        if (udevMonitor) {
//            udev_monitor_unref(udevMonitor);
//            udevMonitor = nullptr;
//        }
//        if (udev) {
//            udev_unref(udev);
//            udev = nullptr;
//        }
//    }

//protected:
//    void run() override {
//        udev = udev_new();
//        if (!udev) {
//            qCritical() << "Cannot initialize udev.";
//            return;
//        }

//        // Create monitor to listen for USB device events
//        udevMonitor = udev_monitor_new_from_netlink(udev, "udev");
//        udev_monitor_filter_add_match_subsystem_devtype(udevMonitor, "usb", nullptr);
//        udev_monitor_enable_receiving(udevMonitor);

//        int monitorFd = udev_monitor_get_fd(udevMonitor);

//        while (true) {
//            fd_set fds;
//            FD_ZERO(&fds);
//            FD_SET(monitorFd, &fds);

//            // Wait for udev events
//            int ret = select(monitorFd + 1, &fds, nullptr, nullptr, nullptr);
//            if (ret > 0 && FD_ISSET(monitorFd, &fds)) {
//                struct udev_device *device = udev_monitor_receive_device(udevMonitor);
//                if (device) {
//                    QString action = udev_device_get_action(device);
//                    QString devNode = udev_device_get_devnode(device);
//                    QString devType = udev_device_get_devtype(device);

//                    if (action == "add" && devType == "usb_device") {
//                        qDebug() << "USB Device Added:" << devNode;
//                        emit usbDeviceAdded(devNode);
//                    } else if (action == "remove" && devType == "usb_device") {
//                        qDebug() << "USB Device Removed:" << devNode;
//                        emit usbDeviceRemoved(devNode);
//                    }

//                    udev_device_unref(device);
//                }
//            }
//        }
//    }

//signals:
//    void usbDeviceAdded(const QString &devNode);
//    void usbDeviceRemoved(const QString &devNode);

//private:
//    struct udev *udev = nullptr;
//    struct udev_monitor *udevMonitor = nullptr;
//};

#endif // DATAUSBCOPIER_H
