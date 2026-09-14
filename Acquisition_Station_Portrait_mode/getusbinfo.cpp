#include "getusbinfo.h"

GetUsbInfo::GetUsbInfo(QObject *parent) :
    QThread(parent),isStop(false)
{
}

GetUsbInfo::~GetUsbInfo()
{
    qDebug()<<"GetUsbInfo::~GetUsbInfo "<<endl;
}

int GetUsbInfo::initUsbDevices()
{
    libusb_device **devs;
    int r;
    ssize_t cnt;

    r = libusb_init(NULL);
    if (r < 0)
        return r;

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
        return (int) cnt;

    getAllDevices(devs);
    libusb_free_device_list(devs, 1);

    libusb_exit(NULL);
    return 0;
}

QString GetUsbInfo::getAllDevices(libusb_device **devs)
{
    libusb_device *dev;
    int i = 0;
    lstUsb.clear();
    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);

        if (r < 0) {
            qDebug()<<"failed to get device descriptor"<<stderr;
            return "";
        }

        STUUSBDevices stu;
        uint8_t path[8];
        int ret = libusb_get_port_numbers(dev, path, sizeof(path));
        if (ret > 0) {
            //fprintf(stdout, "path: %d", path[0]);
            stu.path.append(QString("%1").arg(path[0], 0, 10));
            for (int j = 1; j < ret; ++j){

                //fprintf(stdout, ".%d", path[j]);
                stu.path.append(".").append(QString("%1").arg(path[j], 0, 10));

            }
        }
        //fprintf(stdout, "\n");
        stu.idProduct = QString("%1").arg(desc.idProduct, 4, 16, QLatin1Char('0'));//QString::number(desc.idProduct);
        stu.idVendor = QString("%1").arg(desc.idVendor, 4, 16, QLatin1Char('0'));//QString::number(desc.idVendor);

        lstUsb.append(stu);
    }

    ADSlist.clear();
    for(int i=0;i<lstUsb.count();i++)
    {
        if(lstUsb.at(i).idVendor == "0603" && lstUsb.at(i).idProduct == "8611"){
            ADSlist.append(lstUsb.at(i));
        }
    }


    //showAllUsbDevices();
    return QString(lstUsb[0].idProduct);
}

void GetUsbInfo::showAllUsbDevices()
{
    for(int i=0;i<lstUsb.count();i++)
    {
        qDebug()<<"vid: "<<lstUsb.at(i).idVendor
               <<"pid: "<<lstUsb.at(i).idProduct
                    <<"path: "<<lstUsb.at(i).path<<"\n";
    }
}

void GetUsbInfo::setRunStatus()
{
    isStop = true;
}

void GetUsbInfo::run()
{
    qDebug()<<"GetUsbInfo::run() "<<endl;
    while (!isStop)
    {
       initUsbDevices();
       sleep(10);
    }
}

QList<STUUSBDevices> GetUsbInfo::getADSDevices()
{

    //需要先去更新设备列表
    initUsbDevices();
    return ADSlist;
}

