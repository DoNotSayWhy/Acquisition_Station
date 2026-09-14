#ifndef MAINFORM_H
#define MAINFORM_H

#include <QWidget>
#include<dasbuddy.h>
#include<QFile>
#include<zfycontrol.h>
#include<mysqllite.h>
#include<QFileInfoList> 
#include<QDateTime>
#include<hotplugthread.h>
#include<QMessageBox>
#include<loginform.h>
#include<QDesktopServices>
#include<QUrl>
#include<QNetworkRequest>
#include <QtNetwork>
#include<QListWidgetItem>
#include<QVector>
#include <unistd.h>
#include <set>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <QDir>
#include <fstream>
#include <copytask.h>
//QStandardItemModel
#include<qstandarditemmodel.h>
#include "networkutility.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <unordered_map>
#include <string>
#include "wspairdevicethread.h"
#include "lockerutils.h"
#include "autoquerythread.h"
#include "loginformfactory.h"
#include "faceformutility.h"
#include "devicepolicydialog.h"
#include "lampthread.h"


#include "hsglobal.h"


#include <regex>

#include <QtConcurrent/qtconcurrentrun.h>

using namespace std;

class CheckHsDiskThread;


namespace Ui {
class MainForm;
}

struct WindowProgress {
    mutable int windowId;
    mutable bool isOccupied;
};


struct UsbPathAndInfo{
    //mount path.
    QString path;
    //dev path,e: /dev/sde
    QString devPath;
    QString devPathEnd;
    QString insetTime;
    //this port is usb port, with the root bus and hub port
    QString usbPort;
    //portNum will use for the special windows.
    int portNum;

};

class MainForm : public QWidget
{
    Q_OBJECT

public:
    explicit MainForm(const QVector<DASBuddy *> &buddy, MySqlLite *sqltie,
                      NetworkUtility *net, QWidget *parent = nullptr);
    ~MainForm();
    void setlistWidget();

    void setIPV4Label(QString ipv4Address);

    QMap<int, QVariant> hsMapDisks;
    void setDbusDevicePairStatus(int _idx,bool _sign);
    void setDbusDevicePairInfos(int idx, int offval,QString diskPath);


    QMap<int, QString> hsDynamicMapDisks;
    void deleteCurrentlyCopying(QString _path);

    bool isInteger(const std::string& str) ;

    bool isstandard;

    void releaseSysCaches();

public slots:

    void recSigTaskWork(int type,bool stat,int winidx);


signals:
    void sigSetPriorityCollect(int zfynum,bool isPriority);
    //type = 0 means can see all fiels type = 1 means can only see own files
    void gosearch(QString type,QString username,bool videojur);
    void MyStartCopy(QString dirpath,int mywindow);
    void insertfiles_sqlite(QVariantMap map);
    void takedevice(int num);
    void connectfinish();
    void getUsers(QStringList userlist);
    void toshineBackground();

    void sigsqltocheckfile(QFileInfoList filelist,QString driverid,int num);

    void sigtobuddycheckcopy(int num);
    void toopenbroswer();
    void insertt_logToMSQ(QString username,int type);
    void MainFselectWeekUserInfoTOMSQ(QString myipv4Address);
    void MainFselectWeekFileTOMSQ(QString myipv4Address);

    void setSigTaskWork(bool stat,int winidx);
    void setSigTaskDeviceType(int type,int winidx);

    void currentTaskDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);


private slots:
    void on_pushButton_searchfiles_clicked();
    void updateNewProgress(qreal progress,int zfynum);
    void finishedThreadSlot();

    void on_pushButton_setup_clicked();

    void checkjurstr(int type,const QString &username,const QString &roleId);

    void toinsertZFYDialog();

    void requestFinished(QNetworkReply *reply);
    void updatedisksizelabel(double disksize,bool pingresult,double totalSize,double useSize);

    void myRemoveCurrentDrives(QString sourceDirPath);
    void onCopyFinished(const QString &path,int windowNum);
    void gotoPersonCenter();
    void setPriorityCollect(int windowNum,bool isPriority);

    void currentCopyDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);

    void currentCopyDeviceFileTotalCount(int windowNum, qint64 fileTotalCount,qint64 cpySuccessFileCount);
    void currentCopyDeviceProgressInfo(int windowNum, qreal progress,qint64 copiedCompleteSize,qint64 fileTotalSize);
    void isUnplugUsbDeviceMainForm(const QString &usbpath,int windowNum);

    void on_pushButton_PersonCenter_clicked();

    void on_btn_unlock_clicked();
    void recDasBusDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);





private:
    Ui::MainForm *ui;
    // Mirrors MainWindow's dynamically sized port-widget collection.
    QVector<DASBuddy *> dasbuddy;
    QString getBattery(QString path);
    QString path = "/mnt/usb/zfy1/LOG/BATTERY.TXT";
    ZFYControl *zfy1 ;
    ZFYControl *zfy2 ;
    QStringList uuid;
    MySqlLite *mysql;
    NetworkUtility *mynet;
    QString dirpath;
    int windownum = 0;
    float testNum = 0.1;
    //libusb_device_handler * deviceHandler;

    int user_Id = 123456;
    //QThread *controlthread;
    QThread *controlthread2;
    void closeThread();

    HotPlugThread *thread1;

    CheckHsDiskThread *threadCKDisk = nullptr;

    LoginForm *loginForm = nullptr;

    QNetworkRequest request;
    QNetworkAccessManager* naManager;

    int maxindex = -1;



    int uniqueWindowId = 0;
    QTimer *updatetime;
    QTimer *updatedate;
    QString myipv4Address;
    int portNum;
    int runMode;
private:
    void queryStorageInfo();
    QStringList getUsbPaths();
    int32_t myexec(const char *cmd, vector<string> &resvec);
    QStringList getUsbPathsTest();
    int32_t myexecTest(const char *cmd, std::vector<std::string> &resvec);
    void setUserUploadMain(int zfynum, qreal progress) ;
    QList<WindowProgress> windowProgressList;
    int insertOrUpdateWindowProgress();
    void updatelabeltime();
    void updatedatelabel();
    bool timerRunningIsUnplug = false;
    QTimer *timerIsUnplug;
    void checkLogForDevice(const std::string devPath ,std::string& usbPort,std::string& insetTime);
    QStringList usbhublist;
    Config *getconfig;
    void loadPortConfig();
    QList<UsbPathAndInfo> getUsbPathsAndInfo();
    void loadPairDeviceConfig();
    void initPairDeviceThread();
    bool isPairDevice;
    WsPairDeviceThread* mPairDeviceThread = nullptr;
    //初始化
    void initAllWindowProgress(int windowsCount);
    //根据窗口id获取进度状态
    bool isWindowProgressOccupied(int windowsId);
    //获取一个可用的窗口
    int getUsableWindowProgress();
    //设置窗口进度状态
    void setWindowProgressOccupied(int windowsId,bool isOccupied);
    void showPersonCenter(int type,const QString &username,const QString &roleId);
    QStringList getUsbMountPoints();
    bool isUsbDrive(const QString &devicePath);

    QString getMountPath(const QString &device);
    bool readConfigFile(const QString &path);
signals:
    void sendProgress(QVariant dataVar,qreal progress,int zfynum);
    void mainToNetSelect(QString version,QString workstationIp);
    void selectPolNoRelevance(const QString &path,const QString &polNo);
private slots:
        void receiveInfo();

        void on_btn_update_clicked();
        void setbtnUpdateText(QString);

        void on_btn_exit_clicked();

        void on_btn_help_clicked();

        void on_pushButton_Activate_clicked();
        void toInsertRecorder(QString portNum);
        void toDeleteRecorder(QString portNum);

private:
    QHash<QString, int> currentUsbPortAndWinId;
    QHash<QString, QString> currentUsbPortAndPath;

    QStringList currentDrives;
    //set<QString> currentlyCopying;  // 正在处理的U盘路径
    QSet<QString> currentlyCopying;  // 正在处理的U盘路径

    QHash<QString, QString> currentlyCopyHash;

    void checkUsbDrives();
    void checkUsbDrivesInfos();


    // 任务线程列表
    QList<CopyTask *> copyThreadList;


//    lockerutils *utils;
//    //串口对象指针
//    QSerialPort *m_serialPort;
    autoquerythread *pQueryThread;
    void showLoginForm(int type);
    LampThread *lampThread = nullptr;

private:
    void openPort();
    void queryLockStatus();

    void pushButtonOpenOneKey();
    void checkActivation();
    int getRemainingDays(const QDateTime &firstRun, int totalDays);
    bool loadActivationInfo(QString &activationCode, QString &hardwareID, QDateTime &firstRun, int &trialDays);
    void saveActivationInfo(const QString &activationCode, int days);
    QString generateActivationCode(const QString &hardwareID, const QString &version);
    QString getAllMACAddresses();
    void initLOGO();
    void pushButtonOpenBlueLamp();

    void closePort();
    void quitApplication();



    void pushButtonOffBlueLamp();

    void pushButtonOffRedLamp();

    void pushButtonOffGreenLamp();

    void handleError();
    void turnOnRedLamp();

private slots:
    void turnOffRedLampAndRestoreBlue();
    void turnOffLamp(unsigned short keyId);
    void turnOnLamp(unsigned short keyId);

public:
    std::unordered_map<unsigned short,std::string> lockStatusData;
    QString byteArrayToHexStr(const QByteArray &ba);

    QString findCode(const QString &fileName);
    void convertCode(const QString &path);

    QStringList readTextGBKFiles(const QString &filePath);
    void writeTextUTF8File(const QString &filePath, QStringList contents);

private:
    void sendCommand(const std::string &command);
    enum LampChannel {
        RedLampChannel = 6,
        GreenLampChannel = 7,
        BlueLampChannel = 8
    };


};



///////////

class CheckHsDiskThread : public QThread {

    Q_OBJECT

public:
    CheckHsDiskThread(MainForm* mf,QObject *parent = nullptr) : QThread(parent) {


        //this->_mf = static_cast<MainForm *>(parent) ;
        this->_mf =  dynamic_cast<MainForm *>(mf) ;
    }

    MainForm *_mf;

signals:

    void  sigOfflineDisk(int idx,int offval,QString diskPath);
    void  sigOffWindowId(int idx,bool sign);
    void  sigOffDasbuddy(int idx,bool sign);
    void  sigDeleteCopyPtah(QString _path);


public:
    void run() override {

        // _mf->hsMapDisks;
        QList<int> _keysToRemove;

        while (true) {

            QCoreApplication::processEvents();


            if(is_mainwindow_exited){
                break;
            }

            if(_mf->hsMapDisks.isEmpty())
            {
                //qDebug() << "================== _mf->hsMapDisks is empty ================== ";
            }else {

                QMap<int, QVariant>::iterator _devpair;
                for (_devpair= _mf->hsMapDisks.begin(); _devpair != _mf->hsMapDisks.end(); ++_devpair) {
                    //qDebug() << _devpair.key() << ": " << _devpair.value();

                    int _key = _devpair.key();
                    QString _diskPath = _devpair.value().toString();

                    if(!_diskPath.isEmpty()){

                        QFile _dirPath(_diskPath);
                        if(!_dirPath.exists()){
                             _keysToRemove.append(_key);

                            emit this->sigOfflineDisk(_key,-1,"");

                            emit sigOffDasbuddy(_key,false);
                            emit sigOffWindowId(_key,false);

                            emit sigDeleteCopyPtah(_diskPath);

                        }

                    }


                }

            }


            for (int rmskey : _keysToRemove) {
                _mf->hsMapDisks.remove(rmskey);
            }


            _keysToRemove.clear();
            QThread::msleep(1000);

        }


    }

};



#endif // MAINFORM_H
