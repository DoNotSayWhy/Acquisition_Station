#ifndef FACEENROLLTHREAD_H
#define FACEENROLLTHREAD_H

#include <QObject>
#include <QThread>
#include <opencv2/opencv.hpp>
#include "IdFaceSdk.h"
#include <QImage>
#include <QElapsedTimer>
#include <QMutex>
#include <vector>

#include<config.h>

using namespace cv;

class FaceEnrollThread : public QThread {
    Q_OBJECT
public:
    explicit FaceEnrollThread(const Mat &frame, QObject *parent = nullptr);
    void run() override;
    void setCurrentFrame(const Mat &frame);

    QString dirPathFace ;

     // 当前人脸用户信息
    QString  faceUserNo;

public slots:

    void recUserNo(QString userNo);

signals:
    void updateImage(const QImage &image,QByteArray face,int nFeatureSize);
    void faceEnrollNewFrame();
    void enrollmentStatus(const QString &message);
    void enrollmentFailed(const QString &message);

private:
    void enrollFace();
    Mat currentFrame;
    QMutex frameMutex;
};

#endif // FACEENROLLTHREAD_H
