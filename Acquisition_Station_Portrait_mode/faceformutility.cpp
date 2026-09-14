#include "faceformutility.h"
#include "ui_faceformutility.h"

static double msecond()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1.0e3 + tv.tv_usec * 1.0e-3;
}

FaceFormUtility::FaceFormUtility(QWidget *parent,MySqlLite *sqltie,QString userNo) :
    QWidget(parent),
    ui(new Ui::FaceFormUtility)
{
    mysql = sqltie;
    myUserNo = userNo;
    ui->setupUi(this);
    cameraIn =  Config::getInstance()->Get("wsConfig","cameraIndex").toInt();

    // 设置窗口标志，禁用关闭按钮
//    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    const std::string cascadePath = getCascadePath();
//    if (!c.load("/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt2.xml")) {
//        QMessageBox::information(this, "错误", "加载级联分类器失败");
//        return;
//    }
    if (!c.load(cascadePath)) {
        QMessageBox::information(nullptr, "错误", "加载级联分类器失败");
        return;
    }
//    std::vector<int> cameras = getAvailableCameras();
//    for (int cameraIndex : cameras) {
//        ui->comboBox->addItem("Camera " + QString::number(cameraIndex), cameraIndex);
//    }

    ui->labelfacestatus->clear();
    ui->openbtn->setVisible(false);
    ui->comboBox->setVisible(false);
    ui->labelfacestatus->setVisible(false);
    ui->pushButtonCapture->setVisible(false);
    ui->pushButtonrecognize->setVisible(false);

    initializeRecognizer();
    connect(this,&FaceFormUtility::saveFaceByUserNoToMSQ,mysql,&MySqlLite::saveFaceByUserNoForFaceForm);

}


std::string FaceFormUtility::getCascadePath() {
#if defined(__aarch64__)  // 检查是否是 AArch64 架构
return "/usr/local/aarch64opencv3.4/share/OpenCV/haarcascades/haarcascade_frontalface_alt2.xml";
#elif defined(__GNUC__)  // GNU 编译器（g++）
return "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt2.xml";
#else
return ""; // 默认情况下返回空字符串
#endif
}

FaceFormUtility::~FaceFormUtility()
{
    if (camera_id) killTimer(camera_id);
    v.release();
    delete ui;
}

void FaceFormUtility::on_pushButtoncencle_clicked()
{

    if (camera_id) {
        killTimer(camera_id);
        camera_id = 0;
    }

    if (v.isOpened()) {
        v.release();
    }


    if (enrollThread && enrollThread->isRunning()) {

        enrollThread->quit();
        enrollThread->wait();
        enrollThread->deleteLater();
        enrollThread = nullptr;
    }


    if (recognitionThread && recognitionThread->isRunning()) {
        recognitionThread->quit();
        recognitionThread->wait();
        recognitionThread->deleteLater();
        recognitionThread = nullptr;
    }


    emit goback();
}

void FaceFormUtility::on_pushButtonCapture_clicked()
{
    enroll_face();
}

void FaceFormUtility::initializeRecognizer()
{
    QDir dir("/usr/local/image");
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::critical(this, "错误", "无法创建目录 /usr/local/image");
            return;
        }
    }

    QFile file("/usr/local/image/my_face.xml");
    if (file.exists()) {
        recognizer = LBPHFaceRecognizer::load<LBPHFaceRecognizer>("/usr/local/image/my_face.xml");
    } else {
        recognizer = LBPHFaceRecognizer::create();
    }
    recognizer->setThreshold(100);
}

bool FaceFormUtility::openCamera(int cameraIndex)
{
    if (!v.open(cameraIn)) {
        QMessageBox::critical(this, "错误", "无法打开摄像头");
        return false;
    }
    return true;
}

bool FaceFormUtility::captureFrame()
{
    if (!v.read(src)) {
        qDebug() << "无法读取摄像头数据";
        return false;
    }
    flip(src, src, 1);
    cvtColor(src, rgb, CV_BGR2RGB);
    cv::resize(rgb, rgb, Size(350, 350));
    cvtColor(rgb, gray, CV_RGB2GRAY);
    equalizeHist(gray, dst);

    c.detectMultiScale(dst, faces);
//    qDebug() << "检测到的人脸数量: " << faces.size();

    for (int i = 0; i < faces.size(); i++) {
        rectangle(rgb, faces[i], Scalar(255, 0, 0), 1);
    }

    QImage img(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    ui->labelCamera->setPixmap(QPixmap::fromImage(img));
    // ui->labelCamera->winId();

    return true;
}

void FaceFormUtility::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == camera_id) {
        captureFrame(); // 负责实时展示捕获到的画面
    }
}
void FaceFormUtility::startCameraPreview()
{
    if (!v.isOpened() && openCamera(0)) {
        camera_id = startTimer(30); // 启动定时器以刷新显示
    }
}

void FaceFormUtility::on_openbtn_clicked()
{
    startCameraPreview(); // 只用于预览
}

//void FaceFormUtility::on_closebtn_clicked()
//{
//    if (camera_id != 0) {
//        killTimer(camera_id);
//        camera_id = 0;
//    }
//    v.release(); // 关闭摄像头
//}
///**
// * @brief FaceFormUtility::on_facebtn_clicked
// * opencv3.4.20 开始人脸注册
// */
//void FaceFormUtility::on_facebtn_clicked()
//{
//    startCameraPreview(); // 启动预览，确保能够执行captureFrame()
//    count = 0;
//    flag = 0;
//    face_id = startTimer(100); // 开始人脸注册过程
//}
///**
// * @brief FaceFormUtility::on_recognizebtn_clicked
// * opencv3.4.20 开始人脸识别
// */

//void FaceFormUtility::on_recognizebtn_clicked()
//{
//    startCameraPreview(); // 启动预览，确保能够执行captureFrame()
//    flag = 1;
//    check_id = startTimer(100); // 开始人脸识别过程
//}
/**
 * @brief FaceFormUtility::enroll_face
 * sdk 开始人脸注册过程
 */
void FaceFormUtility::enroll_face() {
    startCameraPreview(); // 确保相机被启动

    if (!captureFrame()) {
        QMessageBox::critical(this, "错误", "无法捕获当前帧");
        return;
    }

    if (src.empty()) {
        qDebug() << "Error: Captured frame is empty.";
        return;
    }
    ui->labelfacestatus->clear();

    if (!enrollThread || !enrollThread->isRunning()) {
        // 创建并启动人脸录入线程
        enrollThread = new FaceEnrollThread(src.clone(), this);
        connect(enrollThread, &FaceEnrollThread::faceEnrollNewFrame, this, &FaceFormUtility::onfaceEnrollFrame);
        connect(enrollThread, &FaceEnrollThread::updateImage, this, &FaceFormUtility::updateImage);
        connect(enrollThread, &FaceEnrollThread::finished, this, [=]() {
            enrollThread->deleteLater();
            enrollThread = nullptr;
        });
        connect(enrollThread, &FaceEnrollThread::enrollmentFailed, this,&FaceFormUtility::updateLabelFaceStatusERROR);

        connect(this, &FaceFormUtility::sendUserNo, enrollThread,&FaceEnrollThread::recUserNo);

        emit sendUserNo(myUserNo);


        enrollThread->start();
    } else {
        QMessageBox::information(this, "信息", "人脸录入已经在运行中");
    }
}


void FaceFormUtility::updateImage(const QImage &image,QByteArray pFeature,int nFeatureSize) {
    emit saveFaceByUserNoToMSQ(myUserNo,pFeature,nFeatureSize);
    ui->labelfacestatus->setVisible(true);
    ui->labelfacestatus->setText("人脸录入成功");
    ui->labelCamera->setPixmap(QPixmap::fromImage(image));
}


void FaceFormUtility::updateLabelFaceStatusERROR() {
    ui->labelfacestatus->setText("人脸录入失败,请重新尝试人脸录入。");
}

void FaceFormUtility::setFaceRStatus(QString text)
{
    ui->labelfacestatus->setVisible(true);
    ui->pushButtonrecognize->setVisible(true);
    ui->labelfacestatus->setText(text);
}

void FaceFormUtility::onRequestNewFrame()
{
    if (!captureFrame()) {
        qDebug() << "无法重新捕获图像";
        if (recognitionThread && recognitionThread->isRunning()) {
            recognitionThread->quit();
            recognitionThread->wait();
            recognitionThread->deleteLater();
            recognitionThread = nullptr;
        }
    } else {
        if (recognitionThread && recognitionThread->isRunning()) {
            recognitionThread->setCurrentFrame(src); // 更新当前帧
        }
    }
}

void FaceFormUtility::onfaceEnrollFrame()
{
    if (!captureFrame()) {
        qDebug() << "无法重新捕获图像";
        if (enrollThread && enrollThread->isRunning()) {
            enrollThread->quit();
            enrollThread->wait();
            enrollThread->deleteLater();
            enrollThread = nullptr;
        }
    } else {
        if (enrollThread && enrollThread->isRunning()) {
            enrollThread->setCurrentFrame(src); // 更新当前帧
        }
    }
}

/**
 * @brief FaceFormUtility::recognize_face
 * sdk 开始人脸识别过程
 */
void FaceFormUtility::recognize_face(){
    startCameraPreview(); // 确保相机被启动

    if (!captureFrame()) {
        QMessageBox::critical(this, "错误", "无法捕获当前帧");
        return;
    }
    if (src.empty()) {
        QMessageBox::critical(this, "错误", "无法获取图像进行识别");
        return;
    }
    ui->labelfacestatus->clear();
    if (!recognitionThread || !recognitionThread->isRunning()) {
        recognitionThread = new FaceRecognitionThread(src, this,mysql);
        recognitionThread->setFeatureArray(myFaceArray);

        connect(recognitionThread, &FaceRecognitionThread::recognitionResult, this, &FaceFormUtility::handleRecognitionResult);

        connect(recognitionThread, &FaceRecognitionThread::finished, recognitionThread, &QObject::deleteLater);
        connect(recognitionThread,&FaceRecognitionThread::errorFaceR,this,&FaceFormUtility::setFaceRStatus);
        connect(recognitionThread, &FaceRecognitionThread::requestNewFrame, this, &FaceFormUtility::onRequestNewFrame);

        recognitionThread->start();
    }else {
        QMessageBox::information(this, "信息", "人脸已经在运行中");
    }
}

void FaceFormUtility::handleRecognitionResult(int matchedIndex, int maxScore,QString userNo) {
    if (matchedIndex >= 0) {
//        QMessageBox::information(this, "识别结果", QString("找到匹配用户: %1\n最大得分: %2").arg(matchedIndex + 1).arg(maxScore));
        emit faceReSuccesToLog(userNo);
        if (recognitionThread && recognitionThread->isRunning()) {
            recognitionThread->quit();
            recognitionThread->wait();
            recognitionThread->deleteLater();
            recognitionThread = nullptr;
        }
        qDebug() << QString("找到匹配用户: %1\n最大得分: %2").arg(matchedIndex + 1).arg(maxScore);
    } else {
        QMessageBox::information(this, "识别结果", "没有找到匹配的用户");
    }
}

void FaceFormUtility::on_pushButtonrecognize_clicked()
{
    recognize_face();
}

void FaceFormUtility::faceFromRMSQ(std::vector<FaceFeature> faceArray)
{
     myFaceArray = faceArray;
}


void FaceFormUtility::FaceRecognition(){

    connect(this,&FaceFormUtility::selectFaceFeatureForMSQ,mysql,&MySqlLite::fetchAllFaceFeaturesTOFaceFrom);
    connect(mysql,&MySqlLite::allFaceFeaturesTOFaceR,this,&FaceFormUtility::faceFromRMSQ);

    emit selectFaceFeatureForMSQ();
//    ui->labelfacestatus->clear();
//    ui->openbtn->setVisible(false);
//    ui->comboBox->setVisible(false);
//    ui->labelfacestatus->setVisible(false);
//    ui->pushButtonCapture->setVisible(false);
//    ui->pushButtonrecognize->setVisible(true);

    recognize_face();
}


void FaceFormUtility::FaceEnroll(){
    ui->pushButtonCapture->setVisible(true);
}

//bool FaceFormUtility::openCamera(int cameraIndex) {
//    if (!v.open(cameraIndex)) {
//        QMessageBox::critical(this, "错误", "无法打开摄像头");
//        return false;
//    }
//    return true;
//}

std::vector<int> FaceFormUtility::getAvailableCameras() {
    std::vector<int> availableCameras;
    for (int i = 0; i < 10; ++i) { // 假设最多有10个摄像头
        cv::VideoCapture tempCamera;
        if (tempCamera.open(i)) {
            availableCameras.push_back(i);
            tempCamera.release(); // 释放摄像头
        }
    }
    return availableCameras;
}

//void FaceFormUtility::onCameraSelected(int index) {
//    int cameraIndex = cameraComboBox->itemData(index).toInt();
//    if (!openCamera(cameraIndex)) {
//        // 处理打开摄像头失败的情况
//    } else {
//        // 摄像头成功打开，可以开始处理视频流
//        qDebug() << "Camera opened successfully: " << cameraIndex;
//    }
//}
