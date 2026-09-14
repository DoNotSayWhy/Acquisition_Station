#ifndef GETUSBINFO_H
#define GETUSBINFO_H

#include <QObject>
#include <QString>
#include <QObject>
#include <QList>
#include <QThread>
#include "libusb-1.0/libusb.h"
#include<QDebug>
struct STUUSBDevices{
    QString idProduct;
    QString idVendor;
    QString path;
};

class GetUsbInfo:  QThread
{
public:
    GetUsbInfo(QObject *parent);
    ~GetUsbInfo();
    int initUsbDevices();
    QString getAllDevices(libusb_device **devs);
    void showAllUsbDevices();
    void setRunStatus();
    void run();

    bool isStop;
    QList<STUUSBDevices> lstUsb;
    QList<STUUSBDevices> ADSlist;

    QList<STUUSBDevices> getADSDevices();
};

#endif // GETUSBINFO_H
