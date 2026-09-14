#ifndef FACERECOGNITIONTHREAD_H
#define FACERECOGNITIONTHREAD_H

#include <QObject>
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QTime>
#include "IdFaceSdk.h"
#include "mysqllite.h"
#include <QElapsedTimer>

using namespace cv;

class FaceRecognitionThread :  public QThread {
    Q_OBJECT
public:
    explicit FaceRecognitionThread(const Mat &frame, QObject *parent = nullptr,MySqlLite *sqltie = nullptr);
    void run() override;
    void setCurrentFrame(const Mat &frame);
    void setFeatureArray(std::vector<FaceFeature> faceArray) ;

signals:
    void finished();
    void recognitionResult(int matchedIndex, int maxScore,QString userNo);
    void selectFaceFeatureForMSQ();
    void errorFaceR(QString);
    void requestNewFrame();
public slots:
    void recognizeFaceFromMSQ(std::vector<FaceFeature> faceArray);
private:
    Mat currentFrame;
    MySqlLite *mySql;
    std::vector<FaceFeature> MYFaceArray;
private:
    void cleanup(BYTE* pFeature, BYTE* pFeatureAll, HANDLE hList);

};

#endif // FACERECOGNITIONTHREAD_H
