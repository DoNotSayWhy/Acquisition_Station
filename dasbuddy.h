#ifndef DASBUDDY_H
#define DASBUDDY_H

#include <QObject>
#include <QWidget>
#include<QLabel>
#include<QDebug>
#include<QPushButton>
#include<QPainter>
#include<QEvent>
#include<QString>
#include<QProgressBar>
#include<QWidget>
#include<QFrame>
#include<QColor>
#include "libusb-1.0/libusb.h"
#include<QThread>
#include<zfycontrol.h>
#include <QtNetwork>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include<QUrl>
#include<QTimer>
#include<QBitmap>
#include<QPainter>
#include "ui_dasbuddy.h"
#include <QMessageBox>


#include<QHeaderView>

#include<mysqllite.h>

namespace Ui {
class DASBuddy;
}

struct command_block_wrapper {
    uint8_t dCBWSignature[4];
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t bmCBWFlags;
    uint8_t bCBWLUN;
    uint8_t bCBWCBLength;
    uint8_t CBWCB[16];
};

class DASBuddy : public QWidget
{
    Q_OBJECT

public:
    explicit DASBuddy(bool deletefile,QWidget *parent = nullptr);
    ~DASBuddy();
    void setBackgroundColor(QColor color);
    void initLableTextStyle();
    void initProgressBarStyle();

    void setphoto(QString path);

    int num=-1;
    ZFYControl *zfy;
    QThread *controlthread = nullptr;

    void closeThread();
    void setIsConnect(volatile bool value);

    void setNetworkPic(const QString &szUrl);

    int getMybattery() const;
    void setMybattery(const int &value);

    QString getWritingnow() const;

    void setWritingnow(const QString &value);
    void startcopynoexits(QFileInfoList copylist);
    void setPhotoText(QString text);
    void setUploadProgressBar(int progress);
    void setCurrentUserAndDevice(QString userNo,QString deviceNo);
    void setCurrentUserNameAndDepartment(QString userName, QString department);
    void setCurrentFileTotalCount(qint64 fileTotalCount,qint64 cpySuccessFileCount);

    void setCurrentCopyProgressInfo(qreal progress,qint64 copiedCompleteSize,qint64 fileTotalSize);

    void statusDoNothing();
    //now copy files.
    void statusCopyFils();
    //copy complete.
    void statusCopyComplete();
    /*
    *  0:do nothing.
    *  1:now copy files
    *  2:copy complete
    * */
    void setCurrentStatus(int status);
    void initConnectSignalSlot();
    void setDasbuddyNumber(int number){this->nNumber = number;}
    int getDasbuddyNumber(){return this->nNumber;}


    int mainIdx;
    QString mainDiskPath;

    qint64 beginCalcTime;
    void updateToalTime();

    void testThmodthid();

    void selectUsers();


private:

    int nNumber;
    qint64 lastCopySize;
    qint64 lastCalcTime;

    QString progressBarStyleCollecting;
    QString progressBarStyleComplete;
    bool isPriority = false;
signals:
    void setPriority(int zfynum,bool isPriority);

signals:
    void sendProgress(QVariant dataVar,qreal progress,int zfynum);
    void startCopy(QVariantMap varmap);
    void sendConnectPortNumber(int num,int battery);
    void getAllSettingStr(QVariantMap map);
    void sig_insertUserLog(int num);
    void insertZFY(int num);
    void sigtowirtesql(QVariantMap copyfilemap);
    void checkfileiscopy(QFileInfoList filelist,QString driverid,int num);
    void mountsuccess(int num);

    void zfycopyfinished();


    void  sigOffWindowId(int idx,bool sign);

    void sendDeviceMessage(int type,bool stat,int winidx);

    void  sigDelCurrentlyCopying(QString _path);
    void senduserlist();

    void currentDasBusDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);


public slots:
    void onPriorityPushButtonClicked();

     void setDevicePairStatus(bool stat);
     void setDeviceShowAll(bool stat);
     void setDevicePairInfos(int idx,QString diskPath);

     void writeHsParamIni();


private slots:
    void finishedThreadSlot();
    void requestFinished(QNetworkReply *reply);


    void on_devicePairButton_clicked();

    void on_pushButton_zfy_clicked();

    void on_pushButton_sdt_clicked();


private:
    int Number;

    QFrame *frame_background,*frame_text;

    QString converRGB2HexStr(QColor _rgb);
    libusb_device_handle *handle = NULL;

    QString userid,driverid;
    int mybattery;

    QString passwordtext = "@Ver#800000000";
    QString toGetBattery = "@Battery";
    QString toGetUserID =  "@GetUserID";
    QString toGetDriveID =  "@GetDriveID";
    QString toUdisktext =  "@Ner#800000000";


    QString moviesize,photosize,moviecycTime,moviecyc,dateWater;
    QString gsensor,moviecodec,prerec,delayrec,gPS,motiondet,ircut,recmode;

    QString toGetMoviesize =  "@GetMoviesize";
    QString toGetPhotosize =  "@GetPhotosize";
    QString toGetMoviecycTime =  "@GetMoviecycTime";
    QString toGetMoviecyc =  "@GetMoviecyc";
    QString toGetDateWater =  "@GetDateWater";
    QString toGetGsensor =  "@GetGsensor";
    QString toGetMoviecodec =  "@GetMoviecodec";
    QString toGetPrerec =  "@GetPrerec";
    QString toGetDelayrec =  "@GetDelayrec";
    QString toGetGPS =  "@GetGPS";
    QString toGetMotiondet =  "@GetMotiondet";
    QString toGetIrcut =  "@GetIrcut";
    QString toGetRecmode =  "@GetRecmode";


    volatile bool isConnect;

    int transferBulkData(libusb_device_handle * usbHandle,command_block_wrapper * cbw,int data_length,unsigned char * buffer );


    QString writingnow;

    QNetworkRequest request;
    QNetworkAccessManager* naManager;
    QTimer *timer_battery = nullptr;//battery power +1 everyminmute;
    QTimer *timer_shine = nullptr;

    bool deletefile;

    qint64 copySize = 0;

    QFileInfoList file_gps;

    QVariantMap gpsMap;

    QDateTime uploadDate;
    QString dirpath;
//    QLabel *idleLabel ;
private:
    Ui::DASBuddy *ui;
};

#endif // DASBUDDY_H
