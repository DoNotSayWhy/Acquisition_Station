#ifndef COPYTASK_H
#define COPYTASK_H
#include<zfycontrol.h>
#include <QObject>
#include <QRunnable>
#include <QString>

class CopyTask : public QObject, public QRunnable {
    Q_OBJECT

public:
    CopyTask(const QString &sourcePath, int windowNumber, QObject *parent = 0);
    CopyTask(QObject *parent = 0);
    ~CopyTask();
    void run();

public slots:
    void setPriorityCollect(int zfyNum,bool isPriority);


    void setBeginWork(bool stat,int winidx);
    void setBeginWorkType(int type,int winidx);


    void recMainFormDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);

signals:
    void progressUpdated(qreal progress, int windowNumber);
    void CopyTaskinsertfiletestemit(QList<QVariantMap> variantMapList,bool sourceFilePathIsErr,const QString &sourceDirFather,int windowNum);
    void deletFileData(QString sourceFilePath,int winNum);
    void finished();
    void onCopyFinishedCT(const QString &sourceDirFather,int windowNum);
    void emptyWindowCT(int windowNum);

    void sigCurrentCopyDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);

    void insertT_UserInfoCTtoMain(const QVariantMap &copyTUserInfo);
    void sigCurrentCopyDeviceFileTotalCount(int windowNum,qint64 fileTotalCount,qint64 cpySuccessFileCount);
    void sigCurrentCopyProgressInfo(int zfynum,qreal progress,qint64 copiedCompleteSize,qint64 fileTotalSize);


    void currentZfyDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);


private:
    QString sourcePath;
    int windowNumber;
    QStringList getUsbPathsTest();
    int32_t myexecTest(const char *cmd, std::vector<std::string> &resvec);
    bool timerRunningIsUnplug = false;
    QTimer *timerIsUnplug;


    bool taskbeginwork = false;
    int  taskdevicetype = 0;

public:
    ZFYControl *zfyControl;
    int getWindowNumber() const;
    void setZFYCPStaus() const;
    void setZFYCPStausStart() const;
private slots:
    void copyTaskProgress(qreal progress,int zfynum);
    void CopyTaskinsertfiletest(QList<QVariantMap> variantMapList,bool sourceFilePathIsErr,const QString &sourceDirFather,int winNum);
    void select_t_file(QString sourceFilePath,int winNum);
    void startcopyFile(const QString &sourceFilePath,int winNum);
    void testtask();
    void no_Files_Copy_CT(const QString &sourceDirFather,int windowNum);
    void deleteThreadCT(int winNum);

    void currentCopyDeviceInfo(int windowNum,QString userNo,QString deviceNo,QString userName,QString corpName);


    void insertT_UserInfoCT(const QVariantMap &copyTUserInfo);
    void currentCopyDeviceFileCount(int windowNum,qint64 fileTotalCount,qint64 cpySuccessFileCount);
    void currentCopyProgressInfo(int zfynum,qreal progress,qint64 copiedCompleteSize,qint64 fileTotalSize);
    void isUnplugUsbDeviceCT(const QString &usbpath,int windowNum);
};

#endif // COPYTASK_H
