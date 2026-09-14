#ifndef FACEENROLLTHREAD_H
#define FACEENROLLTHREAD_H

#include <QObject>
#include <QThread>
#include <opencv2/opencv.hpp>
#include "IdFaceSdk.h"
#include <QImage>
#include <QElapsedTimer>

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
    void finished();
    void updateImage(const QImage &image,QByteArray face,int nFeatureSize);
    void faceEnrollNewFrame();
signals:
    void enrollmentFailed();

private:
    void enrollFace();
     Mat currentFrame;
};

#endif // FACEENROLLTHREAD_H
