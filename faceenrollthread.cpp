#include "faceenrollthread.h"

#include <QDebug>
#include <QDir>

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
    QMutexLocker locker(&frameMutex);
    currentFrame = frame.clone();
}

void FaceEnrollThread::recUserNo(QString userNo)
{
    this->faceUserNo = userNo;
}

void FaceEnrollThread::run() {
    bool hasFrame = false;
    {
        QMutexLocker locker(&frameMutex);
        hasFrame = !currentFrame.empty();
    }

    if (!hasFrame) {
        qDebug() << "Error: currentFrame is empty in run method.";
        emit enrollmentFailed("未获取到摄像头画面，请检查摄像头后重试。");
        return;
    }

    enrollFace();
}

void FaceEnrollThread::enrollFace() {
    emit enrollmentStatus("正在初始化人脸录入，请正对摄像头…");

    const int initResult = IdFaceSdkInit(".", ".");
    if (initResult < 0) {
        qDebug() << "IdFaceSdkInit failed! (ret = " << initResult << ")";
        // 标签宽度固定，使用短提示以确保错误码不会被截断。
        emit enrollmentFailed(QStringLiteral("SDK 初始化失败：%1").arg(initResult));
        return;
    }

    const int featureSize = IdFaceSdkFeatureSize();
    if (featureSize <= 0) {
        IdFaceSdkUninit();
        emit enrollmentFailed("人脸 SDK 未返回有效特征长度。");
        return;
    }

    std::vector<BYTE> feature(featureSize);
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 10000) {
        if (isInterruptionRequested()) {
            IdFaceSdkUninit();
            emit enrollmentFailed("人脸录入已取消。");
            return;
        }

        Mat frame;
        {
            QMutexLocker locker(&frameMutex);
            frame = currentFrame.clone();
        }

        if (frame.empty()) {
            emit enrollmentStatus("等待摄像头画面…");
            emit faceEnrollNewFrame();
            QThread::msleep(200);
            continue;
        }

        Mat sampleGray;
        try {
            cvtColor(frame, sampleGray, COLOR_BGR2GRAY);
        } catch (const cv::Exception& e) {
            qDebug() << "OpenCV error:" << e.what();
            IdFaceSdkUninit();
            emit enrollmentFailed("摄像头画面格式错误，无法录入人脸。");
            return;
        }

        FACE_DETECT_RESULT face;
        const int detectResult = IdFaceSdkDetectFace(sampleGray.data, sampleGray.cols, sampleGray.rows, &face);
        if (detectResult != 1) {
            emit enrollmentStatus("未检测到清晰人脸，请正对摄像头并保持稳定…");
            emit faceEnrollNewFrame();
            QThread::msleep(200);
            continue;
        }

        const int extractResult = IdFaceSdkFeatureGet(sampleGray.data, sampleGray.cols, sampleGray.rows, &face, feature.data());
        if (extractResult != 0) {
            qDebug() << "IdFaceSdkFeatureGet failed! (ret = " << extractResult << ")";
            emit enrollmentStatus("人脸特征提取失败，正在重新尝试…");
            emit faceEnrollNewFrame();
            QThread::msleep(200);
            continue;
        }

        QImage image(sampleGray.data, sampleGray.cols, sampleGray.rows, sampleGray.step, QImage::Format_Grayscale8);
        const QString faceFilepath = dirPathFace + "/" + faceUserNo + ".jpg";
        imwrite(faceFilepath.toStdString(), frame);

        const QByteArray featureData(reinterpret_cast<const char *>(feature.data()), featureSize);
        emit updateImage(image.copy(), featureData, featureSize);
        IdFaceSdkUninit();
        return;
    }

    IdFaceSdkUninit();
    emit enrollmentFailed("10 秒内未检测到可用人脸，请调整光线和姿势后重试。");
}
