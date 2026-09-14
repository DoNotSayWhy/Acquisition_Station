#ifndef ZFYCONTROL_H
#define ZFYCONTROL_H

#include <QObject>
#include<QFile>
#include<QDebug>
#include<QDir>
#include<QFileInfoList>
#include <QUuid>
#include<QThread>
#include<QDateTime>
#include<QProcess>
#include<config.h>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <qhostaddress.h>
#include <qnetworkinterface.h>


#include "hsglobal.h"
#include <QSet>

#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <sstream>


using namespace std;




struct File_copy_Info{
    QString file_uuid;
    QString file_name;
    QString file_est;
    QString file_path;
    QString user_id;
    QString driver_id;
    QString file_gps;
    QDateTime file_uploadtime;
    QDateTime file_creattime;
    qint64 file_size;
    int file_type;

};
struct FileMapping {
    QString sourcePath;
    QString copyPath;
};
struct zfyConfig {
    QString devNo ;
    QString polNo ;
    QString department;
    QString userName;
    QString dev;
    QString pol;
    QString isFlashLight;
    QString uncorrelated;

    // 构造函数
    zfyConfig(QString dNo = "", QString pNo = "", QString dept = "", QString user = "", QString device = "", QString policy = "",QString isFlash ="",QString uncorre = "")
            : devNo(dNo), polNo(pNo), department(dept), userName(user), dev(device), pol(policy) ,isFlashLight(isFlash),uncorrelated(uncorre) {}

};
Q_DECLARE_METATYPE(File_copy_Info);
class ZFYControl : public QObject
{
    Q_OBJECT
public:
    explicit ZFYControl(bool autodeletecopyfile,QObject *parent = nullptr);
    ZFYControl(int zfynum);
    ~ZFYControl();

    int getZfynum(){return this->zfynum;}
    void setZfynum(int value);
    bool copyFile(const QString &sourceFilePath, const QString &destinationFilePath,const qint64 &totalfilesize,qint64 &copiedfilesize,QStringList &copiedsuccessfullypath,
                  int windowNum,QString fileType);

    QString mountUsbPath;
    QSet<QString> readyfiles ;

    int failNums;
    bool  is_send_end = false; ;

    int encodevideo = -1 ;
    int zfydevicetype = 0 ;
    int breakgather = 0;
    int  _task_windowNum ;
    QString _task_userNo ;
    QString _task_deviceNo;
    QString _task_userName;
    QString _task_corpName;


signals:
    void copyonesuccess(QVariant dataVar,qreal progress,int zfynum);
    void mount_finish(int num);
    void start_mount(int num);
    void mount_fail();
    void sendzfyProgress(qreal progress,int zfynum);
    void myRemoveUSB(QString path);
//    void newcopyonesuccess(QString srcPath,QString uuidstr);
    void insertT_Files_sqlite(QList<QVariantMap> map,bool sourceFilePathIsErr,const QString &sourceDirFather,int winNum);
    void checkAndDeleteFile(QString sourceFilePath,int winNum);
    void no_Files_Copy_ZFY(const QString &sourceDirFather,int windowNum);

    void currentDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);

    void insertT_UserInfo(const QVariantMap &copyTUserInfo);
    void currentDeviceFileCount(int windowNum,qint64 fileTotalCount,qint64 copySucFileCount);
    void currentCopyProgressInfo(int zfynum,qreal progress,qint64 copyCompleteSize,qint64 totalFileSize);
    void unplugZFYTOCT(const QString &sourceDirFather,int windowNum);

public slots:
//    void copyFile2(QVariantMap map);
    void myCopyFile(const QString &path,int zfywindow);
    void readBashStandardOutputInfo();
    void readBashStandardErrorInfo();
    void checkt_filedata(QString sourceFilePath);
//    void myinsertfiles_sqlite(QVariantMap copyfilemap);
    void recTaskDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);

private:
    //QString path ="/mnt/usb/zfy1";
    QString path;
    int user_Id = 123456;

    QFileInfoList getAllFile();

    qint64 copySize = 0;
    qint64 sizeall = 0;
    qint64 mysizeall = 0;

    int zfynum = -1;
    QString dirpathCP ;
    bool sourceFilePathIsErr = false;

    volatile bool isStop;
    QProcess *m_proces_bash;
    bool autodeletecopyfile;

    void copyFilesRecursively(const QString &sourceDir, const QString &destinationDir,
                              const QString &sourceDirFather,const qint64 &totalfilesize,
                              qint64 &copiedfilesize,const QString &polNo,
                               QString devNo,QStringList &copiedsuccessfullypath,int winNum,
                              int totalFileCount,int totalCompleteFileCount,bool stopRecursion);
//    bool copyFile(const QString &sourceFilePath, const QString &destinationFilePath,const qint64 &totalfilesize,qint64 &copiedfilesize,QStringList &copiedsuccessfullypath);
    void getFilesSizeInFolder(const QString &folderPath, qint64 &totalFileSize);
    void getFilesSizeFileCountInFolder(const QString &folderPath, qint64 &totalFileSize,qint64 &totalFileCount);
    void insert_t_file_data(const QString &sourceDirFather,const QStringList &copiedsuccessfullypath,const QString &devNo,const QString &polNo,int winNum);
    QHostAddress getHostIPV4Address();
    int getFileCategory(const QString& fileType);
    bool isFolderEmpty(const QString &folderPath);
    QWaitCondition pauseCondition;
    QMutex pauseMutex;
    bool isCopying = true;
    QVector<FileMapping> fileVector;
    bool isCopyingPaused = false;
    bool readConfigFile(const QString &path, zfyConfig &config);
    int getVideoDuration(const QString &filePath);
    bool isDeleteZFYData= false;
    QString getFileType(const QString& fileType);
    QString PathFile;
    bool value1 = false;
    QString fileTranslate = "0";
    QString ffmpegPath = nullptr;
public:
    void pauseCopying();
    void resumeCopying();
    void pauseCopyingNEW();
    void resumeCopyingNEW();
    QString getParentPathAsDevNo(QString filename);

private:
    void compressVideo(const QString &sourceFile, const QString &targetFile);
    void convertToMP4(const QString &inputFile, const QString &outputFile);
    bool zfybeginwork = false;

};

#endif // ZFYCONTROL_H
