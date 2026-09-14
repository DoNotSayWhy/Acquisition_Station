#include "wspairdevicethread.h"

QString currentPlugInPort = "";
QString currentPlugOutPort = "";
bool currentIsInsert = false;

WsPairDeviceThread::WsPairDeviceThread()
{

}

void WsPairDeviceThread::requestStop()
{
    stopRequested.store(true);
}

static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    struct libusb_device_descriptor desc;
    int rc;

    (void)ctx;
    (void)dev;
    (void)event;
    (void)user_data;

    try{
        rc = libusb_get_device_descriptor(dev, &desc);
        if (LIBUSB_SUCCESS != rc) {
            //fprintf (stderr, "Error getting device descriptor\n");
            qDebug()<<stderr<<"Error getting device descriptor\n";
        }

        uint8_t busNum = libusb_get_bus_number(dev);
        uint8_t portNum = libusb_get_port_number(dev);
        uint8_t devAdrs = libusb_get_device_address(dev);
        Q_UNUSED(portNum);
        Q_UNUSED(devAdrs);


        uint8_t path[8];
        int ret = libusb_get_port_numbers(dev, path, sizeof(path));
        qDebug()<<"ret"<<ret;
        QString port_num;
        if (ret > 0) {
            port_num.append(QString("%1.%2").arg(busNum,0,10).arg(path[0], 0, 10));
            for (int j = 1; j < ret; ++j){
                port_num.append(".").append(QString("%1").arg(path[j], 0, 10));
            }
        }

         currentPlugInPort = port_num;
        currentIsInsert = true;
        QThread::msleep(1500);

        //send port_num signal.

        if (LIBUSB_SUCCESS != rc) {
            qDebug()<< "Error opening device:"<<rc;

        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================  wspairdevicethread::hotplug_callback_detach 捕获到异常: ============================ " << e.what() << std::endl;
    }

    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    (void)ctx;
    (void)dev;
    (void)event;
    (void)user_data;

    try{
        uint8_t path[8];

        uint8_t busNum = libusb_get_bus_number(dev);

        int ret = libusb_get_port_numbers(dev, path, sizeof(path));
        QString port_num;
        if (ret > 0) {
            //fprintf(stdout, "path: %d", path[0]);
            port_num.append(QString("%1.%2").arg(busNum,0,10).arg(path[0], 0, 10));
            for (int j = 1; j < ret; ++j){

                //fprintf(stdout, ".%d", path[j]);
                port_num.append(".").append(QString("%1").arg(path[j], 0, 10));

            }
        }

        currentPlugOutPort = port_num;
        qDebug()<<"Device detached"<<"port"<<port_num;

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================  wspairdevicethread::hotplug_callback_detach 捕获到异常: ============================ " << e.what() << std::endl;
    }

    return 0;
}

// 重新加载配置
void WsPairDeviceThread::reloadConfig(){

}

void WsPairDeviceThread::clearForm(int num){

}

void WsPairDeviceThread::run()
{
    libusb_hotplug_callback_handle hp[2];
    int product_id, vendor_id, class_id;

//    //    vendor_id  = (argc > 1) ? (int)strtol (argv[1], NULL, 0) : 0x045a;
//    //    product_id = (argc > 2) ? (int)strtol (argv[2], NULL, 0) : 0x5005;
//    //    class_id   = (argc > 3) ? (int)strtol (argv[3], NULL, 0) : LIBUSB_HOTPLUG_MATCH_ANY;

////    vendor_id  =  0x0603;
////    product_id =  0x8611;
////    vendor_id  =  0x4255;
////    product_id =  0x1000;
    vendor_id  =  LIBUSB_HOTPLUG_MATCH_ANY;
    product_id =  LIBUSB_HOTPLUG_MATCH_ANY;
    class_id   =  LIBUSB_HOTPLUG_MATCH_ANY;

    rc = libusb_init (NULL);
    if (rc < 0)
    {
        //printf("failed to initialise libusb: %s\n", libusb_error_name(rc));
        qDebug()<<"failed to initialise libusb:"<<libusb_error_name(rc);
        return;
    }

    if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
        //printf ("Hotplug capabilites are not supported on this platform\n");
        qDebug()<<"Hotplug capabilites are not supported on this platform";
        libusb_exit (NULL);
        return;
    }

    rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, LIBUSB_HOTPLUG_NO_FLAGS, vendor_id,
                                           product_id, class_id, hotplug_callback, NULL, &hp[0]);
    if (LIBUSB_SUCCESS != rc) {
        //fprintf (stderr, "Error registering callback 0\n");
        qDebug()<<stderr<<"Error registering callback 0";
        libusb_exit (NULL);
        return;
    }

    rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_NO_FLAGS, vendor_id,
                                           product_id,class_id, hotplug_callback_detach, NULL, &hp[1]);
   if (LIBUSB_SUCCESS != rc) {
        //fprintf (stderr, "Error registering callback 1\n");
        qDebug()<<stderr<<"Error registering callback 1";
        libusb_hotplug_deregister_callback(NULL, hp[0]);
        libusb_exit(NULL);
        return;
    }
    while (!stopRequested.load() && !is_mainwindow_exited) {

        try{

            // A short timeout makes shutdown deterministic.  The former
            // blocking call could hold the restart path indefinitely.
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 200000;
            rc = libusb_handle_events_timeout(NULL, &timeout);
            if (rc < 0) {

                qDebug()<<"libusb_handle_events() failed:"<<libusb_error_name(rc);
            } else {

                //qDebug()<<"libusb_handle_events1111:"<<isInsert;
                //判断插入拔出，处理事件
                if (currentIsInsert) {
                    if(currentPlugInPort.length() > 0){
                        emit insertRecorder(currentPlugInPort);

                        QThread::msleep(100);
                        currentPlugInPort = "";
                    }
                    currentIsInsert = false;
                } else {
                    if(currentPlugOutPort.length() > 0){
                        emit deleteRecorder(currentPlugOutPort);

                        QThread::msleep(100);
                        currentPlugOutPort = "";
                    }
                }
            }


        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================  wspairdevicethread::libusb_handle_events 捕获到异常: ============================ " << e.what() << std::endl;
        }

    }

    libusb_hotplug_deregister_callback(NULL, hp[0]);
    libusb_hotplug_deregister_callback(NULL, hp[1]);
    libusb_exit(NULL);
}

void WsPairDeviceThread::toreconnect(int zfynum)
{
    zfyreconnectnum = zfynum;
    qDebug()<<"to reconnect zfy num12344:"<<zfyreconnectnum;
}


void WsPairDeviceThread::communicate(){
    //emit deleteZFY(deleteNum);
}
void WsPairDeviceThread::getUsbPeriodically(){

}
