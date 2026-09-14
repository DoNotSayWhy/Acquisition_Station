#ifndef ZIANGFINGERUTILITY_H
#define ZIANGFINGERUTILITY_H

#include <QObject>
#include <QDebug>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdlib>
#include "SYProtocol.h"

#define fp_602 1
#define fp_606 2
#define fp_608 3
#define fp_xbt 4
#define fp_fl  5
#define fp_kvm 6

#define path_pc   1
#define path_arm  0

//删除模板失败
#define ZAZ_DEL_TEMP_ERR 0x10
//清空指纹库失败
#define ZAZ_CLEAR_TEMP_ERR 0x11
//不能进入休眠
#define ZAZ_SLEEP_ERR 0x12
//口令不正确
#define ZAZ_INVALID_PASSWORD 0x13
//系统复位失败
#define ZAZ_RESET_ERR 0x14
//无效指纹图象
#define ZAZ_INVALID_IMAGE 0x15
//其他错误代码，请调用 ZAZErr2Str(int nErrCode) 进行查看

class ZiangFingerUtility : public QObject {
    Q_OBJECT

public:
    explicit ZiangFingerUtility(QObject *parent = nullptr);
    ~ZiangFingerUtility();

    bool enrollFinger(int nhanle);
    bool searchFinger(int nhanle);
    void performFunctionTests(int nhanle);
    void printarr(unsigned char *inarr, int len);
    void printarrbit(unsigned char *inarr, int len);
    bool tryOpenDeviceEx(int fpType, int path, int &deviceHandle);
    void initializeDevicebyenroll();
    void initializeDevicebysearch();
    void runFingerTests(int nhanle);
    void deletefingerDelChar(HANDLE hHandle,int nAddr,int iStartPageID,int nDelPageNum);
    void cleanfinger(HANDLE hHandle,int nAddr);

private:
    static const int MaxCountssearch = 500;
    static constexpr int MaxCountsenroll = 10000000;
    int nRet,iType=0;
    int DEV_ADDR = 0xffffffff;
    HANDLE nhanle;
    int IMAGE_X = 256;
    int IMAGE_Y = 360;

signals:
    void updateStatus(const QString& message);
    void searchFingerStatus(const QString& message);
    void uploadFingerChar(const QByteArray featureData);
    void uploadRecognizeChar(const QByteArray featureData);

public slots:
    void startEnroll();
    void startRecognize();

};



#endif // ZIANGFINGERUTILITY_H
