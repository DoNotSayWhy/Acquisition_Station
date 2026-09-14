#include "ziangfingerutility.h"

ZiangFingerUtility::ZiangFingerUtility(QObject *parent)
    : QObject(parent)
{
    nhanle = NULL;
    //1.设置指纹协议
    ZAZ_MODE(0);
    ZAZSetlog(1);//开日志 日志在执行文件目录中  zazlog.txt
}

ZiangFingerUtility::~ZiangFingerUtility() {
    // Close devices or clean up resources if necessary
    ZAZCloseDeviceEx(nhanle);
}

void ZiangFingerUtility::printarr(unsigned char *inarr, int len)
{
    for (int i = 0; i < len; i++) {
        printf("%02X", inarr[i]);
    }
    printf("\n");
}

void ZiangFingerUtility::printarrbit(unsigned char *inarr, int len)
{
    printf("   [");
    for (int i = 0; i < len; i++) {
        for (int j = 0; j < 8; j++) {
            if (inarr[i] & (1 << j)) {
                printf("%d,", i * 8 + j);
            }
        }
    }
    printf("]\n");
}

bool ZiangFingerUtility::tryOpenDeviceEx(int fpType, int path,HANDLE  &deviceHandle)
{
    qDebug()<<"tryOpenDeviceEx \n";
    ZAZ_SETPATH(fpType, path);
    int nRet = ZAZOpenDeviceEx(&deviceHandle, DEVICE_UDisk, 0, 2, 0);
    if (nRet == ZAZ_OK) {
        unsigned char pPassword[4] = {0, 0, 0, 0};
        nRet = ZAZVfyPwd(deviceHandle, DEV_ADDR, pPassword);
        if (nRet == 0) {
            qDebug()<<"tryOpenDeviceEx success. \n";
            return true;
        }
    }
    return false;
}

void ZiangFingerUtility::initializeDevicebyenroll()
{
    bool deviceOpened = false;

    if(nhanle != NULL){
        ZAZCloseDeviceEx(nhanle);
        nhanle=NULL;
    }
    emit updateStatus("连接指纹仪.");
    deviceOpened = tryOpenDeviceEx(fp_606, path_pc, nhanle);
    if (!deviceOpened) {
        deviceOpened = tryOpenDeviceEx(fp_602, path_pc, nhanle);
    }

    if (!deviceOpened) {
        qDebug()<<"Open UDisk failed\n";
        ZAZCloseDeviceEx(nhanle);
        emit updateStatus("指纹仪连接失败！");
        return;
    }

//    runFingerTests(nhanle);
    if (!enrollFinger(nhanle)) {
        ZAZCloseDeviceEx(nhanle);
        emit updateStatus("采集失败，请重新采集");
        printf("Finger enrollment or search test failed\n");
    }
}
void ZiangFingerUtility::initializeDevicebysearch()
{
    bool deviceOpened = false;

    if(nhanle != NULL){
        ZAZCloseDeviceEx(nhanle);
        nhanle=NULL;
    }

    emit searchFingerStatus("正在打开指纹仪...");
    deviceOpened = tryOpenDeviceEx(fp_606, path_pc, nhanle);
    if (!deviceOpened) {
        deviceOpened = tryOpenDeviceEx(fp_602, path_pc, nhanle);
    }

    if (!deviceOpened) {
        printf("Open UDisk failed\n");
        emit searchFingerStatus("指纹仪连接失败！");
        return;
    }

//    runFingerTests(nhanle);
    if (!searchFinger(nhanle)) {
        emit searchFingerStatus("匹配失败");
        ZAZCloseDeviceEx(nhanle);
        printf("Finger enrollment or search test failed\n");
    }
}

void ZiangFingerUtility::runFingerTests(HANDLE nhanle)
{
    if (!enrollFinger(nhanle) || !searchFinger(nhanle)) {
        printf("Finger enrollment or search test failed\n");
    }

    performFunctionTests(nhanle);
    printf("*****************END*****************\n");
}

bool ZiangFingerUtility::enrollFinger(HANDLE nhanle) {
    int counts = 0;
    int buffer = 1;
    emit updateStatus("请按指纹...");
    while (true) {
        qDebug() << "*****************Enroll Finger Test*****************";
        int nRet = ZAZ_NO_FINGER;
        qDebug() << "1. Enroll: Please press finger" << buffer << "......";

        // Wait for the finger
        while (nRet == ZAZ_NO_FINGER) {
            nRet = ZAZGetImage(nhanle, DEV_ADDR);
            counts++;
            if (counts > MaxCountsenroll) {
                qDebug() << "Waiting time over......";
                return false;
            } else {
//                qDebug() << "Waiting time:" << counts << "......";
            }
        }

        if (nRet != ZAZ_OK) {
            qDebug() << "ZAZGetImage is Fail, error code =" << nRet;
            return false;
        }
        qDebug() << "2. ZAZGetImage ok......";

        nRet = ZAZGenChar(nhanle, DEV_ADDR, buffer);
        if (nRet != ZAZ_OK) {
            qDebug() << "ZAZGenChar is Fail, error code =" << nRet;
            continue;  // Retry from the top of the loop
        }
        qDebug() << "3. ZAZGenChar ok Buffer =" << buffer;
        emit updateStatus("采集中......");
        buffer++;

        if (buffer < 3) {
            continue;  // Go back to enroll another finger
        }

        nRet = ZAZRegModule(nhanle, DEV_ADDR);
        if (nRet != ZAZ_OK) {
            qDebug() << "ZAZRegModule is Fail, error code =" << nRet;
            buffer = 1;
            continue;  // Restart the enrollment process
        }
        qDebug() << "4. ZAZRegModule ok";
        int iMbNum = 0;
        nRet = ZAZTemplateNum(nhanle,DEV_ADDR, &iMbNum);
        if(nRet != ZAZ_OK){
            qDebug() << "ZAZTemplateNum is Fail, error code =" << nRet;
            return false;
        }

        nRet = ZAZStoreChar(nhanle, DEV_ADDR, CHAR_BUFFER_A, ++iMbNum);
        if (nRet != ZAZ_OK) {
            qDebug() << "ZAZStoreChar is Fail, error code =" << nRet;
            buffer = 1;
            continue;  // Restart the enrollment process
        }
        qDebug() << "5. ZAZStoreChar ok id = %d"<<iMbNum;
        qDebug() << "*****************END*****************";

        //read the finger feature data.
        unsigned char fingerFeature[512] = {0};
        int featureLen = 0;
        nRet = ZAZUpChar(nhanle,DEV_ADDR,CHAR_BUFFER_A,fingerFeature,&featureLen);
        if(nRet == 0){
            //get feature data.
            QByteArray featureData((char*)fingerFeature,featureLen);
            emit uploadFingerChar(featureData);
        }

        emit updateStatus("采集完成");
        break;
    }

    return true;
}
bool ZiangFingerUtility::searchFinger(HANDLE nhanle) {
    int counts;
    int buffer;
    emit searchFingerStatus("识别开始，请按下手指……");
    while (true) {
        buffer = 1;
        counts = 0;
        qDebug() << "*****************Search Finger Test*****************";

        int nRet = ZAZ_NO_FINGER;
        qDebug() << "1. Search: Please press finger" << buffer << "......";

        // Wait for the finger
        while (nRet == ZAZ_NO_FINGER) {
            nRet = ZAZGetImage(nhanle, DEV_ADDR);
            counts++;
            if (counts > MaxCountssearch) {
                qDebug() << "Waiting time over......";
                return false;
            } else {
//                qDebug() << "Waiting time:" << counts << "......";
            }
        }

        if (nRet != ZAZ_OK) {
            qDebug() << "ZAZGetImage is Fail, error code =" << nRet;
            return false;
        }
        qDebug() << "2. ZAZGetImage ok......";

        nRet = ZAZGenChar(nhanle, DEV_ADDR, buffer);
        if (nRet != ZAZ_OK) {
            qDebug() << "ZAZGenChar is Fail, error code =" << nRet;
            continue; // Retry the entire process
        }
        qDebug() << "3. ZAZGenChar ok Buffer =" << buffer;

        //read the finger feature data.
        unsigned char fingerFeature[512] = {0};
        int featureLen = 0;
        nRet = ZAZUpChar(nhanle,DEV_ADDR,buffer,fingerFeature,&featureLen);
        if(nRet == 0){
            //get feature data.
            QByteArray featureData((char*)fingerFeature,featureLen);
            emit uploadRecognizeChar(featureData);
        } else {
            emit searchFingerStatus("未生成指纹数据，请重按指纹！");
            continue;
        }

        /*
        int nFinger;
        int nScore;
        nRet = ZAZSearch(nhanle, DEV_ADDR, CHAR_BUFFER_A, 0, 1000, &nFinger, &nScore);
        if (nRet != ZAZ_OK) {
            qDebug() << "ZAZSearch is Fail, error code =" << nRet;
            continue; // Retry the entire process
        }
        qDebug() << "4. ZAZSearch ok ID =" << nFinger << "Score =" << nScore;
        */
        qDebug() << "*****************END*****************";

        break;
    }

    return true;
}
void ZiangFingerUtility::performFunctionTests(HANDLE nhanle) {
    qDebug() << "*****************Function Test*****************";

    unsigned char pPwd[4] = {0};
    unsigned char UserContent[32];
    int nRet = ZAZReadIndexTable(nhanle, DEV_ADDR,0, UserContent);
    if (nRet != ZAZ_OK) {
        qDebug() << "1. ZAZReadIndexTable is Fail, error code =" << nRet;
    } else {
        qDebug() << "1. ZAZReadIndexTable ok";
        printarrbit(UserContent, 32);
    }

    nRet = ZAZVfyPwd(nhanle, DEV_ADDR, pPwd);
    if (nRet != ZAZ_OK) {
        qDebug() << "1. ZAZVfyPwd is Fail, error code =" << nRet;
    } else {
        qDebug() << "1. ZAZVfyPwd ok";
    }

    nRet = ZAZDelChar(nhanle, DEV_ADDR, 1, 1);
    if (nRet != ZAZ_OK) {
        qDebug() << "2. ZAZDelChar is Fail, error code =" << nRet;
    } else {
        qDebug() << "2. ZAZDelChar ok";
    }

    nRet = ZAZEmpty(nhanle, DEV_ADDR);
    if (nRet != ZAZ_OK) {
        qDebug() << "3. ZAZEmpty is Fail, error code =" << nRet;
    } else {
        qDebug() << "3. ZAZEmpty ok";
    }

    unsigned char pParTable[512];
    nRet = ZAZReadParTable(nhanle, DEV_ADDR, pParTable);
    if (nRet != ZAZ_OK) {
        qDebug() << "4. ZAZReadParTable is Fail ,error code =" << nRet;
    } else {
        qDebug() << "4. ZAZReadParTable ok";
        printarr(pParTable, 32);
    }

    int iMbNum;
    nRet = ZAZTemplateNum(nhanle, DEV_ADDR, &iMbNum);
    if (nRet != ZAZ_OK) {
        qDebug() << "5. ZAZTemplateNum is Fail ,error code =" << nRet;
    } else {
        qDebug() << "5. ZAZTemplateNum ok  iMbNum =" << iMbNum;
    }

    unsigned int pRandom;
    nRet = ZAZGetRandomData(nhanle, DEV_ADDR, reinterpret_cast<unsigned char*>(&pRandom));
    if (nRet != ZAZ_OK) {
        qDebug() << "6. ZAZGetRandomData is Fail ,error code =" << nRet;
    } else {
        qDebug() << "6. ZAZGetRandomData ok  pRandom =" << QString::number(pRandom, 16);
    }

    int pnLen;
    nRet = ZAZGetCharLen(&pnLen);
    if (nRet != ZAZ_OK) {
        qDebug() << "7. ZAZGetCharLen is Fail ,error code =" << nRet;
    } else {
        qDebug() << "7. ZAZGetCharLen ok";
    }

    unsigned char pTemplet[2048];
    int iTempletLength = 0;
    ZAZSetCharLen(512);
    nRet = ZAZUpChar(nhanle, DEV_ADDR, 1, pTemplet, &iTempletLength);
    if (nRet != ZAZ_OK) {
        qDebug() << "8. ZAZUpChar is Fail ,error code =" << nRet;
    } else {
        qDebug() << "8. ZAZUpChar ok" << iTempletLength;
        printarr(pTemplet, 512);
    }

    // Optionally implement the commented-out sections
    /*
    nRet = ZAZDownChar(nhanle, DEV_ADDR, 1, pTemplet, iTempletLength);
    if (nRet != ZAZ_OK)
        qDebug() << "9. ZAZDownChar is Fail ,error code =" << nRet;
    else {
        qDebug() << "9. ZAZDownChar ok";
    }

    for (int i = 0; i < 32; i++) UserContent[i] = i;
    nRet = ZAZWriteInfo(nhanle, DEV_ADDR, 1, UserContent);
    if (nRet != ZAZ_OK)
        qDebug() << "10. ZAZWriteInfo is Fail ,error code =" << nRet;
    else {
        qDebug() << "10. ZAZWriteInfo ok";
    }

    nRet = ZAZReadInfo(nhanle, DEV_ADDR, 1, UserContent);
    if (nRet != ZAZ_OK)
        qDebug() << "11. ZAZReadInfo is Fail ,error code =" << nRet;
    else {
        qDebug() << "11. ZAZReadInfo ok";
        printarr(UserContent, 32);
    }
    */

    qDebug() << "*****************END*****************";
}

void ZiangFingerUtility::deletefingerDelChar(HANDLE hHandle,int nAddr,int iStartPageID,int nDelPageNum){
    int nRet = ZAZDelChar( hHandle, nAddr, iStartPageID, nDelPageNum);
}
void ZiangFingerUtility::cleanfinger(HANDLE hHandle,int nAddr){
    int nRet = ZAZEmpty( hHandle, nAddr);
}

void ZiangFingerUtility::startEnroll(){
    initializeDevicebyenroll();
}
void ZiangFingerUtility::startRecognize(){
    initializeDevicebysearch();
}
