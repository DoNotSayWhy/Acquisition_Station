#include "connectads.h"
#include <QDebug>
#include <QThread>
int verbose = 1;
#define RETRY_MAX                     5
#define BOMS_GET_MAX_LUN              0xFE
static char* MANUFACTURER= "NOVATEK\0";
static uint8_t ENDPOINT_OUT = 0x02;
static uint8_t ENDPOINT_IN = 0x81;

// Section 5.2: Command Status Wrapper (CSW)
struct command_status_wrapper {
    uint8_t dCSWSignature[4];
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t bCSWStatus;
};

static const uint8_t cdb_length[256] = {
    //	 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  0
    06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  1
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  2
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  3
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  4
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  5
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  6
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  7
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  8
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  9
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  A
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  B
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  C
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  D
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  E
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  F
};
#define ERR_EXIT(errcode) do { qDebug()<<"error: "<<errcode<<"   "<<libusb_strerror((enum libusb_error)errcode); return -1; } while (0)

#define CALL_CHECK(fcall) do { int _r=fcall; if (_r < 0) ERR_EXIT(_r); } while (0)



ConnectADS::ConnectADS(QObject *parent) : QObject(parent)
{


    isConnect = false;


    //m_usb_dev = new QUsbDevice();
    //this->setupDevice();
    //this->openDevice();


//    int r = libusb_init(NULL);
//    if (r<0) {
//        qDebug()<<"init error"<<r;
//    }
//    libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);



    connect(this,SIGNAL(writeoneSuccess(QString)),this,SLOT(changetoUdisk(QString)));
    connect(this,SIGNAL(changetoUdisksuccess()),this,SLOT(changeisConnect()));
}

ConnectADS::~ConnectADS()
{
    //libusb_free_device_list(devs,1);
    libusb_exit(NULL);

}

void ConnectADS::setupDevice()
{
    /* There are 2 ways of identifying devices depending on the platform.
   * You can use both methods, only one will be taken into account.
   */

    qDebug("setupDevice");

    //m_usb_dev->setLogLevel(QUsbDevice::logDebug);

    //
    m_filter.pid = 0x8611;
    m_filter.vid = 0x0603;

    //
//    m_config.alternate = 0;
//    m_config.config = 1;
//    m_config.interface = 0;

    m_usb_dev->setId(m_filter);
    //m_usb_dev->setConfig(m_config);
}



void ConnectADS::connectdevice(){
    //libusb_open()

    //devHandle = libusb_open_device_with_vid_pid(NULL,0x0603,0x8611);
//    devHandle = m_usb_dev->getdevHandle();
//    if (!devHandle) {
//        qDebug()<<"open handler faile \n";
//        return;
//    }

//    communicate(devHandle,passwordtext);



    //libusb_release_interface(devHandle,0);
    //libusb_close(devHandle);
}

bool ConnectADS::openDevice()
{
    qDebug("Opening");

    if (m_usb_dev->open() == QUsbDevice::statusOK) {
        // Device is open

        return true;
    }
    return false;
}



static int get_mass_storage_status(libusb_device_handle *handle, uint8_t endpoint, uint32_t expected_tag)
{
    int i, r, size;
    struct command_status_wrapper csw;

    // The device is allowed to STALL this transfer. If it does, you have to
    // clear the stall and try again.
    i = 0;
    do {
        r = libusb_bulk_transfer(handle, endpoint, (unsigned char*)&csw, 13, &size, 1000);
        if (r == LIBUSB_ERROR_PIPE) {
            libusb_clear_halt(handle, endpoint);
        }
        i++;
    } while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));
    if (r != LIBUSB_SUCCESS) {
        qDebug()<<"   get_mass_storage_status:" << libusb_strerror((enum libusb_error)r);
        return -1;
    }
    if (size != 13) {
        qDebug()<<"   get_mass_storage_status: received "<<size<<" bytes (expected 13)";
        return -1;
    }
    if (csw.dCSWTag != expected_tag) {
        qDebug()<<"   get_mass_storage_status: mismatched tags (expected"<<expected_tag<<", received " << csw.dCSWTag<<")";
        return -1;
    }
    // For this test, we ignore the dCSWSignature check for validity...
    qDebug()<<"   Mass Storage Status: " << csw.bCSWStatus <<" "<< (csw.bCSWStatus?"FAILED":"Success\n");
    if (csw.dCSWTag != expected_tag)
        return -1;
    if (csw.bCSWStatus) {
        // REQUEST SENSE is appropriate only if bCSWStatus is 1, meaning that the
        // command failed somehow.  Larger values (2 in particular) mean that
        // the command couldn't be understood.
        if (csw.bCSWStatus == 1)
            return -2;	// request Get Sense
        else
            return -1;
    }

    // In theory we also should check dCSWDataResidue.  But lots of devices
    // set it wrongly.
    return 0;
}

int ConnectADS::transferBulkData(libusb_device_handle * usbHandle,command_block_wrapper * cbw,int data_length,unsigned char * buffer ) {
    //    QThread::msleep(500);
    int transferredLength = 0;

    int i = 0,r = 0;


    do {
        // The transfer length must always be exactly 31 bytes.
        r = libusb_bulk_transfer(usbHandle,ENDPOINT_OUT,(unsigned char *)cbw,31,&transferredLength,1000);
        if (r == LIBUSB_ERROR_PIPE) {
            libusb_clear_halt(usbHandle, ENDPOINT_OUT);
            qDebug()<<"LIBUSB_ERROR_PIPE ERROR, clear";
        }
        i++;
    } while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));

    if (r==0 && transferredLength == 31) {
        qDebug()<<"\ntransferBulkData:Write USBC successful!";
    }
    else {
        qDebug()<<("\ntransferBulkData:Write USBC error, code")<<r;
        return r;
    }


    if (buffer) {
        memset(buffer, 0, 64);

        CALL_CHECK(libusb_bulk_transfer(usbHandle, ENDPOINT_IN, (unsigned char *)buffer, data_length, &transferredLength, 1000));



        if (r==0) {
            qDebug()<<"transferBulkData:Received:"<<transferredLength<<" chars";
            qDebug()<<QLatin1String((const char *)buffer,transferredLength);
            //            ui->lineResponse->setText(QLatin1String((char *)strResponse,transferredLength));
        }
        else {
            qDebug()<<"transferBulkData:Read error"<<r;
            return r;
        }
    }


    return get_mass_storage_status(usbHandle,ENDPOINT_IN,cbw->dCBWTag);



}

volatile bool ConnectADS::getIsConnect() const
{
    return isConnect;
}

void ConnectADS::toConnectZFYwithDevice(libusb_device_handle *devHandle)
{
    communicate(devHandle,passwordtext);
    libusb_release_interface(devHandle,0);
}


int ConnectADS::communicate(libusb_device_handle * devHandle,QString text) {



    int r;

    //    if (ENDPOINT_IN & LIBUSB_ENDPOINT_IN) {
    //        qDebug()<<"cannot send command on IN endpoint\n";
    ////        return;
    //    }

    //    if (ENDPOINT_OUT & LIBUSB_ENDPOINT_OUT) {
    //        qDebug()<<"cannot send command on OUT endpoint\n";
    ////        return;
    //    }



    r = libusb_set_auto_detach_kernel_driver(devHandle,1);
    if (r<0) {
        qDebug()<<"Detach kernel driver:"<<r;
        return r;
    }
    //    r = libusb_set_configuration(devHandle,1);
    //    if (r<0) {
    //        qDebug()<<"Set Configuration error:"<<r;
    //        return;
    //    }

    r = libusb_claim_interface(devHandle, 0);
    if (r<0) {
        qDebug()<<"Claim interface error:"<<r;
        return r;
    }
    uint8_t lun;
    printf("Reading Max LUN:\n");
    r = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
                                BOMS_GET_MAX_LUN, 0, 0, &lun, 1, 1000);
    // Some devices send a STALL instead of the actual value.
    // In such cases we should set lun to 0.
    if (r == 0) {
        lun = 0;
    } else if (r < 0) {
        printf("   Failed: %s", libusb_strerror((enum libusb_error)r));
    }
    qDebug()<<"   Max LUN ="<<lun;

    //   uint8_t cdb[16] = {0xC5,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0xFF,0x02,0x00,0x00,0x00,0x00};	// SCSI Command Descriptor Block

    uint8_t cdbInquery[16] = {0x12,0x00,0x00,0x00,0x24,0x00};	// SCSI Command Descriptor Block

    struct command_block_wrapper cbw;

    memset(&cbw, 0, sizeof(cbw));
    cbw.dCBWSignature[0] = 'U';
    cbw.dCBWSignature[1] = 'S';
    cbw.dCBWSignature[2] = 'B';
    cbw.dCBWSignature[3] = 'C';
    cbw.dCBWTag = 1;
    cbw.dCBWDataTransferLength = 0x24;
    cbw.bmCBWFlags = LIBUSB_ENDPOINT_IN;
    cbw.bCBWLUN = lun;
    // Subclass is 1 or 6 => cdb_len
    cbw.bCBWCBLength = cdb_length[cdbInquery[0]];;
    memcpy(cbw.CBWCB, cdbInquery, 0x06);

    uint8_t buffer[64];
    this->transferBulkData(devHandle,&cbw,0x24,(unsigned char *)&buffer);

    char vid[9], pid[9], rev[5];

    int i;
    // The following strings are not zero terminated
    for (i=0; i<8; i++) {
        vid[i] = buffer[8+i];
        pid[i] = buffer[16+i];
        rev[i/2] = buffer[32+i/2];	// instead of another loop
    }
    vid[8] = 0;
    pid[8] = 0;
    rev[4] = 0;
    qDebug()<<"    VID:PID:REV "<<vid<<":"<<pid<<":"<<rev;




    int tag = 100;
    unsigned char cdb[16];	// SCSI Command Descriptor Block


    tag++;


    memset(cdb,0,sizeof(cdb));
    cdb[0] = 0xC5;
    cdb[1] = 0x07;
    //cdb[9] = ui->lineRequest->text().length();
    cdb[9] = text.length();
    cdb[10] = 0x02;
    cdb[11] = 0x03;
    cdb[12] = 0x01;
    memset(&cbw, 0, sizeof(cbw));
    cbw.dCBWSignature[0] = 'U';
    cbw.dCBWSignature[1] = 'S';
    cbw.dCBWSignature[2] = 'B';
    cbw.dCBWSignature[3] = 'C';
    cbw.dCBWTag = tag;
    //cbw.dCBWDataTransferLength = ui->lineRequest->text().length();
    cbw.dCBWDataTransferLength = text.length();
    cbw.bmCBWFlags = LIBUSB_ENDPOINT_OUT;
    cbw.bCBWLUN = lun;
    // Subclass is 1 or 6 => cdb_len
    cbw.bCBWCBLength = 0x10;
    memcpy(cbw.CBWCB, cdb, 0x10);


    int transferredLength = 0;


    i = 0;

    do {
        // The transfer length must always be exactly 31 bytes.
        r = libusb_bulk_transfer(devHandle,ENDPOINT_OUT,(unsigned char *)&cbw,31,&transferredLength,1000);
        //qDebug()<<"(unsigned char *)&cbw:"<<&cbw;
        //qDebug()<<"transferredLength:"<<transferredLength;
        if (r == LIBUSB_ERROR_PIPE) {
            libusb_clear_halt(devHandle, ENDPOINT_OUT);
        }
        i++;
    } while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));

    if (r==0 && transferredLength == 31) {
        qDebug()<<"Write USBC successful!";
    }
    else {
        qDebug()<<("Write USBC error, code")<<r;
        return r;
    }


    //QByteArray tempData = ui->lineRequest->text().toLocal8Bit();
    QByteArray tempData = text.toLocal8Bit();
    transferredLength = 0;
    int stringLength = tempData.length();

    i = 0;
    do {

        // The transfer length must always be exactly 31 bytes.
        uint8_t requestData[64];
        memset(requestData,0,sizeof(requestData));
        memcpy(&requestData,tempData.data(),stringLength);
        r = libusb_bulk_transfer(devHandle,ENDPOINT_OUT,(unsigned char *)&requestData,stringLength,&transferredLength,1000);
        qDebug()<<"stringLength:"<<stringLength;
        if (r == LIBUSB_ERROR_PIPE) {
            libusb_clear_halt(devHandle, ENDPOINT_OUT);
        }
        i++;
    } while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));



    if (r==0 && transferredLength == stringLength) {
        qDebug()<<"Write command successful! " <<tempData<<" "<<transferredLength<<"bytes";

        if(tempData == "@Ner#800000000"){
            emit changetoUdisksuccess();
        }
    }
    else {
        qDebug()<<("Write error, code")<<r;
        return r;
    }


    char strResponse[32];

    memset(strResponse,32,'\0');
    i = 0;
    do {
        r = libusb_bulk_transfer(devHandle,ENDPOINT_IN,(unsigned char *)strResponse,13,&transferredLength,1000);
        if (r == LIBUSB_ERROR_PIPE) {
            libusb_clear_halt(devHandle, ENDPOINT_IN);
        }
        i++;
    } while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));



    if (r==0) {
        qDebug()<<"Received22:"<<transferredLength<<" chars";
        qDebug()<<QLatin1String((const char *)strResponse,transferredLength);
        //            ui->lineResponse->setText(QLatin1String((char *)strResponse,transferredLength));
    }
    else {
        qDebug()<<"Read error"<<r;
        return r;
    }



    for(int jj=0;jj<1;++jj){
        QThread::msleep(100);
        tag+=jj;


        //        unsigned char cdb123[16] = {0xC5,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x02,0x02,0x01,0x00,0x00,0x00};	// SCSI Command Descriptor Block
        //        unsigned char cdb[16] = {0xC5,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x02,0x03,0x01,0x00,0x00,0x00};	// SCSI Command Descriptor Block

        tag++;
        //        unsigned char cdb[16];	// SCSI Command Descriptor Block

        memset(cdb,0,sizeof(cdb));
        cdb[0] = 0xC5;
        cdb[1] = 0x07;
        cdb[9] = 0x10;
        cdb[10] = 0xFF;
        cdb[11] = 0x04;

        memset(&cbw, 0, sizeof(cbw));
        cbw.dCBWSignature[0] = 'U';
        cbw.dCBWSignature[1] = 'S';
        cbw.dCBWSignature[2] = 'B';
        cbw.dCBWSignature[3] = 'C';
        cbw.dCBWTag = tag;
        cbw.dCBWDataTransferLength = 0x10;
        cbw.bmCBWFlags = LIBUSB_ENDPOINT_IN;
        cbw.bCBWLUN = lun;
        // Subclass is 1 or 6 => cdb_len
        cbw.bCBWCBLength = 0x10;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //04 first

        tag++;
        cbw.dCBWTag = tag;
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //04 second

        tag++;
        cbw.dCBWTag = tag;
        cdb[11] = 0x06;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //06


        tag++;
        cbw.dCBWTag = tag;
        cdb[11] = 0x04;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //04


        tag++;
        cbw.dCBWTag = tag;
        cdb[11] = 0x03;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //03 first

        tag++;
        cbw.dCBWTag = tag;
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //03 second


        tag++;
        cbw.dCBWTag = tag;
        cdb[11] = 0x05;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //05

        tag++;


        cbw.dCBWTag = tag;
        memset(cdb,0,sizeof(cdb));
        cdb[0] = 0xC5;
        cdb[1] = 0x07;
        cdb[9] = 0x1E;
        cdb[10] = 0x02;
        cdb[11] = 0x01;
        cdb[12] = 0x01;
        cbw.dCBWDataTransferLength = 0x1E;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x1E,(unsigned char *)&buffer); //1E

        tag++;
        cbw.dCBWTag = tag;
        memset(cdb,0,sizeof(cdb));
        cdb[0] = 0xC5;
        cdb[1] = 0x07;
        cdb[9] = 0x10;
        cdb[10] = 0xFF;
        cdb[11] = 0x04;
        cbw.dCBWDataTransferLength = 0x10;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //04


        tag++;
        cbw.dCBWTag = tag;
        cdb[11] = 0x04;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x10,(unsigned char *)&buffer); //04

        tag++;
        cbw.dCBWTag = tag;
        memset(cdb,0,sizeof(cdb));
        cdb[0] = 0xC5;
        cdb[1] = 0x07;
        cdb[9] = 0x1E;
        cdb[10] = 0x02;
        cdb[11] = 0x02;
        cdb[12] = 0x01;
        cbw.dCBWDataTransferLength = 0x1E;
        memcpy(cbw.CBWCB, cdb, 0x10);
        transferBulkData(devHandle,&cbw,0x1E,(unsigned char *)&buffer); //1E


        //ui->lineResponse->setText(QByteArray((const char *)&buffer,12));
        QThread::msleep(100);
        emit writeoneSuccess(QString((const char *)&buffer));






    }

    return 0;



}
void ConnectADS::changetoUdisk(QString getRespone){
    qDebug()<<"changetoUdisk start";

    if (!devHandle) {
        qDebug()<<"open handler faile \n";
        return;
    }

    if(getRespone.contains("@Ver:OKONCE")){
        qDebug()<<"123"<<getRespone;


        communicate(devHandle,toGetUserID);

    }
    if(getRespone.contains("@UserID#")){
        qDebug()<<"UserID"<<getRespone;

        communicate(devHandle,toGetDriveID);
    }

    if(getRespone.contains("@DriveID#")){
        qDebug()<<"DriveID"<<getRespone;

        communicate(devHandle,toUdisktext);
    }

}

void ConnectADS::changeisConnect()
{
    qDebug()<<"toUdisk success";
    isConnect = true;
}


