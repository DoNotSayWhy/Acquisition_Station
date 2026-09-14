#ifndef FACEFORMUTILITY_H
#define FACEFORMUTILITY_H

#include <QWidget>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include<opencv2/face.hpp>
#include <vector>
#include <map>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimerEvent>
#include<QtSerialPort/QtSerialPort>
#include<QtSerialPort/QSerialPortInfo>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <QPointer>

#include "IdFaceSdk.h"
#include "faceenrollthread.h"
#include "facerecognitionthread.h"
#include "mysqllite.h"

using namespace  cv;
using namespace cv::face;
using namespace std;


namespace Ui {
class FaceFormUtility;
}

class FaceFormUtility : public QWidget
{
    Q_OBJECT

public:
    explicit FaceFormUtility(QWidget *parent = 0, MySqlLite *sqltie = nullptr, QString userNo = QString());
    ~FaceFormUtility();


    void FaceRecognition();

    void startCameraPreview();

    void FaceEnroll();



private slots:

    void on_pushButtonCapture_clicked();

    void on_openbtn_clicked();

    void on_pushButtoncencle_clicked();

    void on_pushButtonrecognize_clicked();

//    void on_closebtn_clicked();

//    void on_facebtn_clicked();

//    void on_recognizebtn_clicked();
    void faceFromRMSQ(std::vector<FaceFeature> faceArray);


signals:
    void goback();
    void saveFaceByUserNoToMSQ(QString,QByteArray,int);
    void selectFaceFeatureForMSQ();
    void faceReSuccesToLog(QString);

    void sendUserNo(QString userNo);


private:
    Ui::FaceFormUtility *ui;
private:
    void enroll_face();
    void recognize_face();
    std::string getCascadePath();
    std::vector<int> getAvailableCameras();

private:
    //摄像头相关成员设置
    VideoCapture v;//视频流对象
    Mat src;//存放原图容器
    Mat gray;//存放灰度图
    Mat dst;//存放直方图
    Mat rgb;//存放rgb图
    CascadeClassifier c;//级联分类器类
    vector<Rect> faces;//存放人脸矩形框的容器
    int camera_id = 0;//摄像头的定时器id
    void timerEvent(QTimerEvent *e);//重写定时器事件函数声明

    //人脸录入相关设置
    Ptr<LBPHFaceRecognizer> recognizer;//人脸识别器指针
    vector<Mat> study_faces;//保存录入人脸的数组
    vector<int> study_labels;//保存录入人脸对应标签的数组
    int count;//记录录入人脸次数
    int flag;//区分是人脸录入还是人脸检测
    int face_id;//人脸录入定时器id

    //人脸检测设置
    int check_id;//人脸检测id


    bool loadClassifier();
    void initializeRecognizer();
    bool openCamera(int cameraIndex);
    void closeCamera();
    bool captureFrame();
    void registerFace();
    void recognizeFace();
//    void startCameraPreview();
    MySqlLite *mysql;
    QString myUserNo;
    std::vector<FaceFeature> myFaceArray;
    int cameraIn = 0;



private slots:
    void updateImage(const QImage &image,QByteArray pFeature,int nFeatureSize);
    void handleRecognitionResult(int matchedIndex, int maxScore,QString userNo) ;


    void updateLabelFaceStatusERROR();

    void setFaceRStatus(QString);

    void onRequestNewFrame();

    void onfaceEnrollFrame();

private:
    QPointer<FaceEnrollThread> enrollThread;
    QPointer<FaceRecognitionThread> recognitionThread;


};

#endif // FACEFORMUTILITY_H



