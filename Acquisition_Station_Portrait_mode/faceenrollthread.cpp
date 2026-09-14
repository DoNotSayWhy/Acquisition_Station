#include "faceenrollthread.h"

#include <QDebug>
#include <QDir>
#include <QTime>

FaceEnrollThread::FaceEnrollThread (const Mat &frame, QObject *parent)
    : QThread(parent), currentFrame(frame.clone())
{
    if (currentFrame.empty()) {
        qDebug() << "Error: Received an empty frame in FaceEnrollThread constructor.";
    }


    this->dirPathFace = QString("%1/CaptureFaces").arg( Config::getInstance()->Get("wsConfig","dataPath").toString());

    if(!QDir().exists(this->dirPathFace)){
        QDir().mkpath(this->dirPathFace);
    }


}

void FaceEnrollThread::setCurrentFrame(const Mat &frame) {
//    qDebug() << "FaceEnrollThread::setCurrentFrame";
    currentFrame = frame.clone();
}

void FaceEnrollThread::recUserNo(QString userNo)
{
    this->faceUserNo = userNo;
}

void FaceEnrollThread::run() {
    if (currentFrame.empty()) {
        qDebug() << "Error: currentFrame is empty in run method.";
        emit finished();
        return;
    }


    enrollFace();


    emit finished();
}


void FaceEnrollThread::enrollFace() {

    if (currentFrame.empty()) {
        qDebug() << "Error: currentFrame is empty in enrollFace method.";
        return;
    }

    // 初始化SDK
//    printf("SDK version: %08X\r\n", IdFaceSdkVer());

    // 权限/参数设置在此添加
    int ret = IdFaceSdkInit(".", ".");
    if (ret < 0) {
        qDebug() << "IdFaceSdkInit failed! (ret = " << ret << ")";
        emit enrollmentFailed();
        return;
    }

    // 获取特征大小
    int nFeatureSize = IdFaceSdkFeatureSize();
    printf("feature size = %d\r\n", nFeatureSize);

//    double compare_start, compare_end;
    HANDLE hList = IdFaceSdkListCreate(10);
    if (hList == (HANDLE)NULL) {
        printf("IdFaceSdkListCreate failed.\r\n");
        emit enrollmentFailed();
        return;
    }

    BYTE* pFeature = new BYTE[nFeatureSize];
    if (pFeature == NULL) {
        printf("Malloc features buffer failed.\r\n");
        IdFaceSdkListDestroy(hList); // 确保列表被销毁
        emit enrollmentFailed();
        return;
    }
    QElapsedTimer timer;
    timer.start(); // 开始计时

    // 进行重试，直到达到超时限制
    while (timer.elapsed() < 10000) {

        // 此处确保 currentFrame 转为灰度图像
        Mat sampleGray;
        try {
            cvtColor(currentFrame, sampleGray, COLOR_BGR2GRAY); // 将当前帧转换为灰度图

        } catch (const cv::Exception& e) {

            qDebug() << "OpenCV error: " << e.what();
            delete[] pFeature; // 清理内存
            IdFaceSdkListDestroy(hList); // 清理
            emit enrollmentFailed();
            return;
        }

        FACE_DETECT_RESULT face;

        // 人脸检测
        //    compare_start = QTime::currentTime().msecsSinceStartOfDay();
        ret = IdFaceSdkDetectFace(sampleGray.data, sampleGray.cols, sampleGray.rows, &face);
        //    compare_end = QTime::currentTime().msecsSinceStartOfDay();

        if (ret != 1) {
            qDebug() << "IdFaceSdkDetectFace failed. (ret = " << ret << ")";
            // 请求重新捕获图像
            emit faceEnrollNewFrame();
            QThread::sleep(1); // 等待一秒后重试（可选）
            continue; // 继续下一个循环
        }

        //    printf("IdFaceSdkDetectFace time = %0.4lf ms\r\n", compare_end - compare_start);

        // 人脸特征提取
        //    compare_start = QTime::currentTime().msecsSinceStartOfDay();
        ret = IdFaceSdkFeatureGet(sampleGray.data, sampleGray.cols, sampleGray.rows, &face, pFeature);
        //    compare_end = QTime::currentTime().msecsSinceStartOfDay();

        if (ret != 0) {
            qDebug() << "IdFaceSdkFeatureGet failed. (ret = " << ret << ")";
            // 请求重新捕获图像
            emit faceEnrollNewFrame();
            QThread::sleep(1); // 等待一秒后重试（可选）
            continue; // 继续下一个循环
        }

        //    printf("IdFaceSdkFeatureGet time = %0.4lf ms\r\n", compare_end - compare_start);

        // 将特征插入列表中
        int nPos = -1;
        if (IdFaceSdkListInsert(hList, &nPos, pFeature, 1) != 1) {

            qDebug() << "IdFaceSdkListInsert failed.";
            delete[] pFeature; // 清理内存
            IdFaceSdkListDestroy(hList); // 清理
            emit enrollmentFailed();
            return;

        } else {
            printf("IdFaceSdkListInsert succeeded. position = %d\r\n", nPos);
        }

        int index = 1;

        BYTE* pScores = new BYTE[index];
        int j = IdFaceSdkListCompare(hList, pFeature, 0, -1, pScores);

        if (j != index) {

            emit enrollmentFailed();
            qDebug() << "IdFaceSdkListFeatureCompare failed. return " << j;

        } else {

            BYTE nMaxScore = 0, nMatchIndex = 0;
            for (int i = 0; i < index && i < 20; i++) {
                printf(" %d", (unsigned char)pScores[j]);
                if (pScores[i] > nMaxScore) {
                    nMaxScore = pScores[i];
                    nMatchIndex = i;
                }
            }
            printf("    Compare %d features, Max score: %d, Match user: %d\r\n", index, nMaxScore, nMatchIndex + 1);

        }

        // 发送图像更新信号
        QImage img(sampleGray.data, sampleGray.cols, sampleGray.rows, sampleGray.step, QImage::Format_Grayscale8);


        QString  faceFilepath(this->dirPathFace + "/" + faceUserNo + ".jpg");
        //QImage facesimg(currentFrame.data, currentFrame.cols, currentFrame.rows, currentFrame.step, QImage::Format_RGB32);
        //facesimg.save(faceFilepath);
        imwrite(faceFilepath.toStdString().c_str(), currentFrame);



        QByteArray byteArray(reinterpret_cast<const char*>(pFeature), nFeatureSize);
        emit updateImage(img,byteArray,nFeatureSize);

        // 清理资源
        delete[] pFeature;
        IdFaceSdkListDestroy(hList);
        IdFaceSdkUninit();
        return;
    }


    // 清理资源
    delete[] pFeature;
    IdFaceSdkListDestroy(hList);
    IdFaceSdkUninit();
    return;


}
