#ifndef WSPAIRDEVICETHREAD_H
#define WSPAIRDEVICETHREAD_H
#include <QObject>
#include<QThread>
#include "libusb-1.0/libusb.h"
#include <QDebug>
#include <atomic>
#include<config.h>
#include<dasbuddy.h>

class WsPairDeviceThread : public QThread
{
    Q_OBJECT
public:
    WsPairDeviceThread();
    void requestStop();
    void reloadConfig();
    void clearForm(int num);
signals:
    void insertRecorder(QString portNum);
    void deleteRecorder(QString portNum);
protected:
    void run();
private slots:
    void toreconnect(int zfynum);
    void getUsbPeriodically();
private:
    int rc;
    void communicate();
    Config *getconfig;
    //int hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);

    int zfyreconnectnum = -1;



    QStringList usbhublist;
    int indexNum = -1;
    int deleteNum = -1;

    bool isInsert = false;
    std::atomic_bool stopRequested {false};

};

#endif // WSPAIRDEVICETHREAD_H
