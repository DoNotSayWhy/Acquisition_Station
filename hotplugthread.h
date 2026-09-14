#ifndef HOTPLUGTHREAD_H
#define HOTPLUGTHREAD_H

#include <QObject>
#include<QThread>
#include "libusb-1.0/libusb.h"
#include <QDebug>
#include<config.h>
#include<dasbuddy.h>


#include "hsglobal.h"


class HotPlugThread : public QThread
{
    Q_OBJECT
public:
    HotPlugThread(DASBuddy **buddy);
signals:
    void deleteZFY(int num);
    void insertZFY();
protected:
    void run();
private slots:

    void hswork();
private:
    int rc;
    void communicate();
    Config *getconfig;
    //int hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);

    int zfyreconnectnum = -1;
};

#endif // HOTPLUGTHREAD_H
