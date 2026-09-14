#include "copytask.h"
#include "zfycontrol.h"

CopyTask::CopyTask(const QString &sourcePath, int windowNumber, QObject *parent)
    : QObject(parent), sourcePath(sourcePath), windowNumber(windowNumber), zfyControl( new ZFYControl(windowNumber)) {
    setAutoDelete(false);
}

CopyTask::CopyTask(QObject *parent)
{ 

}

void CopyTask::run() {


    // 确保 zfyControl 在当前线程中操作
//    zfyControl->moveToThread(QThread::currentThread());

    //DirectConnection
    connect(zfyControl, &ZFYControl::sendzfyProgress, this, &CopyTask::copyTaskProgress, Qt::DirectConnection);
    connect(zfyControl, &ZFYControl::insertT_Files_sqlite, this, &CopyTask::CopyTaskinsertfiletest, Qt::QueuedConnection);
    connect(zfyControl, &ZFYControl::checkAndDeleteFile, this, &CopyTask::select_t_file, Qt::QueuedConnection);
    connect(zfyControl, &ZFYControl::no_Files_Copy_ZFY, this, &CopyTask::isUnplugUsbDeviceCT, Qt::QueuedConnection);
    connect(zfyControl, &ZFYControl::currentDeviceInfo,this,&CopyTask::currentCopyDeviceInfo, Qt::QueuedConnection);
    connect(zfyControl, &ZFYControl::insertT_UserInfo,this,&CopyTask::insertT_UserInfoCT,Qt::QueuedConnection);
    connect(zfyControl, &ZFYControl::currentDeviceFileCount,this,&CopyTask::currentCopyDeviceFileCount, Qt::QueuedConnection);
    connect(zfyControl, &ZFYControl::currentCopyProgressInfo,this,&CopyTask::currentCopyProgressInfo, Qt::QueuedConnection);
    connect(zfyControl, &ZFYControl::unplugZFYTOCT,this,&CopyTask::no_Files_Copy_CT);

    connect(this, &CopyTask::currentZfyDeviceInfo,zfyControl,&ZFYControl::recTaskDeviceInfo,Qt::DirectConnection);

    //qDebug() << "Calling zfyControl->checkt_filedata for path:" << sourcePath;


    zfyControl->checkt_filedata(sourcePath);

}

void CopyTask::copyTaskProgress(qreal progress, int zfynum) {


    emit progressUpdated(progress, zfynum);
}


void CopyTask::CopyTaskinsertfiletest(QList<QVariantMap> variantMapList, bool sourceFilePathIsErr, const QString &sourceDirFather,int winNum) {

    if (sourceFilePathIsErr) {

        emit onCopyFinishedCT(sourceDirFather, winNum);
    }


    emit CopyTaskinsertfiletestemit(variantMapList, sourceFilePathIsErr,sourceDirFather, winNum);

//    qDebug() << "CopyTask::startcopyFile finished, emitting finished signal";
}

void CopyTask::select_t_file(QString sourceFilePath,int winNum) {


    emit deletFileData(sourceFilePath,winNum);
}


void CopyTask::startcopyFile(const QString &sourceFilePath,int winNum) {

    QCoreApplication::processEvents();


    if(getWindowNumber() == winNum){

        zfyControl->zfydevicetype = taskdevicetype;
        zfyControl->myCopyFile(sourceFilePath, winNum);
    }
}



void CopyTask::currentCopyDeviceInfo(int windowNum,QString userNo, QString deviceNo,QString userName,QString corpName){

    emit sigCurrentCopyDeviceInfo(windowNum,userNo,deviceNo,userName,corpName);
}


void CopyTask::insertT_UserInfoCT(const QVariantMap &copyTUserInfo)
{

    emit insertT_UserInfoCTtoMain(copyTUserInfo);
}

void CopyTask::currentCopyDeviceFileCount(int windowNum,qint64 fileTotalCount,qint64 cpySuccessFileCount){


    emit sigCurrentCopyDeviceFileTotalCount(windowNum,fileTotalCount,cpySuccessFileCount);
}

void CopyTask::currentCopyProgressInfo(int zfynum,qreal progress,qint64 copiedCompleteSize,qint64 fileTotalSize) {

    emit sigCurrentCopyProgressInfo(zfynum,progress,copiedCompleteSize,fileTotalSize);
}

void CopyTask::setPriorityCollect(int zfyNum,bool isPriority){

    printf("CopyTask::setPriorityCollect zfyNum=%d,priority:%d\n",zfyNum,isPriority);
    if(isPriority){
        if(zfyNum == zfyControl->getZfynum()){
            printf("==============CopyTask::setPriorityCollect zfyNum=%d,priority:%d ---[%d]resumeCopying\n",zfyNum,isPriority,zfyControl->getZfynum());
            zfyControl->resumeCopying();
        } else {
            printf("==============CopyTask::setPriorityCollect zfyNum=%d,priority:%d ---[%d]pauseCopying\n",zfyNum,isPriority,zfyControl->getZfynum());
            zfyControl->pauseCopying();
        }
    } else {
        printf("==============CopyTask::setPriorityCollect zfyNum=%d,priority:%d ---[%d]resumeCopying\n",zfyNum,isPriority,zfyControl->getZfynum());
        zfyControl->resumeCopying();
    }
}




void CopyTask::setBeginWork(bool stat,int winidx)
{
    if(windowNumber == winidx){

         taskbeginwork = stat;
    }

}


void CopyTask::setBeginWorkType(int type,int winidx)
{


    if(windowNumber == winidx){

         taskdevicetype = type;
    }

}



void CopyTask::recMainFormDeviceInfo(int windowNum, QString userNo, QString deviceNo, QString userName, QString corpName)
{


        emit currentZfyDeviceInfo(  windowNum,  userNo,  deviceNo,  userName,  corpName);
}


void CopyTask::testtask() {
    qDebug() << "emit is ok";
}


void CopyTask::no_Files_Copy_CT(const QString &sourceDirFather,int windowNum) {

    emit onCopyFinishedCT(sourceDirFather, windowNum);
    emit finished();

    delete this;
}



void CopyTask::isUnplugUsbDeviceCT(const QString &usbpath,int windowNum){

//    QStringList usbPaths = getUsbPathsTest();
    QFile sourceFile(usbpath);
    if (!sourceFile.exists()) {
        qDebug() << "Source file does not exist:" << usbpath;
        // 条件满足，停止定时任务
        if(timerRunningIsUnplug) {
            timerIsUnplug->stop();
            timerRunningIsUnplug = false;
        }
        emit onCopyFinishedCT(usbpath, windowNum);

        emit finished();
        delete this;
//    }

//    if (!usbPaths.contains(usbpath)) {
//        // 条件满足，停止定时任务
//        if(timerRunningIsUnplug) {
//            timerIsUnplug->stop();
//            timerRunningIsUnplug = false;
//        }
//        emit onCopyFinishedCT(usbpath, windowNum);

//        emit finished();
//        delete this;
    } else {
        // 条件不满足，启动定时任务
        if(!timerRunningIsUnplug) {
            timerIsUnplug = new QTimer(this);
            connect(timerIsUnplug, &QTimer::timeout, this, [=]() {
                isUnplugUsbDeviceCT(usbpath, windowNum);

            });
            timerIsUnplug->start(1000); // 1秒钟触发一次
            timerRunningIsUnplug = true;
        }



    }
}

void CopyTask::deleteThreadCT(int winNum) {

   // qDebug() << "CopyTask::deleteThreadCT() windowNumber" << windowNumber << " param winNum=" << winNum;
    /*
    if (zfyControl) {
        if(zfyControl->getZfynum() == winNum){
            delete zfyControl; // 释放动态分配的内存
            zfyControl = nullptr; // 防止重复删除
        }
    }
    */
}

CopyTask::~CopyTask() {

    if (zfyControl) {
        delete zfyControl; // 释放动态分配的内存
        zfyControl = nullptr; // 防止重复删除
    }
    emit finished();
}
int32_t CopyTask::myexecTest(const char *cmd, std::vector<std::string> &resvec) {


    resvec.clear();
    FILE *pp = popen(cmd, "r"); //建立管道
    if (!pp) {
        return -1;
    }
    char tmp[1024]; //设置一个合适的长度，以存储每一行输出
    while (fgets(tmp, sizeof(tmp), pp) != NULL) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0'; //去除换行符
        }
        resvec.push_back(tmp);
    }
    pclose(pp); //关闭管道
    return resvec.size();
}

QStringList CopyTask::getUsbPathsTest() {


    QDir dir("/dev");
    QStringList path_list;

    const char *sd_chars = "abcdefghijklmnopqrstuvwxyz";
    for (const char *c = sd_chars; *c; ++c) {
        QString usb_device = QString("/dev/sd%1").arg(*c);
        QFile usb_device_file(usb_device);
        if (usb_device_file.exists()) {
            std::vector<std::string> resvec;
            char df_path[64];
            sprintf(df_path, "df -h | grep %s", qPrintable(usb_device));
            myexecTest(df_path, resvec);
            if (!resvec.empty()) {
                std::string df_output = resvec.back();
                size_t last_space = df_output.find_last_of(" \t");
                if (last_space != std::string::npos) {
                    std::string mount_path = df_output.substr(last_space + 1);
                    path_list.append(QString::fromStdString(mount_path));
                }
            }
        }
    }

    return path_list;
}
int CopyTask::getWindowNumber() const{
    return windowNumber;
}
void CopyTask::setZFYCPStaus() const{

    zfyControl->pauseCopyingNEW();
}
void CopyTask::setZFYCPStausStart() const{

    zfyControl->resumeCopyingNEW();
}

