#ifndef CONNECTADS_H
#define CONNECTADS_H
#include "libusb-1.0/libusb.h"
#include<QString>
#include <QObject>
#include <qusbdevice.h>
struct command_block_wrapper {
    uint8_t dCBWSignature[4];
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t bmCBWFlags;
    uint8_t bCBWLUN;
    uint8_t bCBWCBLength;
    uint8_t CBWCB[16];
};

class ConnectADS : public QObject
{
    Q_OBJECT
public:
    explicit ConnectADS(QObject *parent = nullptr);
    ~ConnectADS();
    void connectdevice();
    //void changetoUdisk();

    volatile bool getIsConnect() const;

    void toConnectZFYwithDevice(libusb_device_handle *devHandle);

signals:
    void writeoneSuccess(QString respone);
    void changetoUdisksuccess();
private slots:
    void changetoUdisk(QString getRespone);
    void changeisConnect();
private:
    libusb_device * dev = NULL;
    libusb_device **devs = NULL;


    libusb_device_handle * devHandle;
    QUsbDevice *m_usb_dev;

    int communicate(libusb_device_handle * devHandle,QString text);
    int transferBulkData(libusb_device_handle * usbHandle,command_block_wrapper * cbw,int data_length,unsigned char * buffer );

    QString passwordtext = "@Ver#800000000";
    QString toGetUserID =  "@GetUserID";
    QString toGetDriveID =  "@GetDriveID";
    QString toUdisktext =  "@Ner#800000000";
    volatile bool isConnect;


    void setupDevice();

    QUsbDevice::Id m_filter;
    QUsbDevice::Config m_config;


    bool openDevice();
};

#endif // CONNECTADS_H
