#include "dasbuddy.h"
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QStackedLayout>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include "ui_dasbuddy.h" 


#define RETRY_MAX                     5
#define BOMS_GET_MAX_LUN              0xFE
static char* MANUFACTURER= "NOVATEK\0";
//static uint8_t ENDPOINT_OUT = 0x02;
static uint8_t ENDPOINT_OUT = 0x01;
static uint8_t ENDPOINT_IN = 0x81;

// Section 5.2: Command Status Wrapper (CSW)
struct command_status_wrapper {
    uint8_t dCSWSignature[4];
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t bCSWStatus;
};


static const uint8_t cdb_length[256] = {
    //	 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  0
    06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  1
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  2
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  3
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  4
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  5
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  6
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  7
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  8
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  9
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  A
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  B
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  C
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  D
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  E
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  F
};
#define ERR_EXIT(errcode) do { qDebug()<<"error: "<<errcode<<"   "<<libusb_strerror((enum libusb_error)errcode); return -1; } while (0)

#define CALL_CHECK(fcall) do { int _r=fcall; if (_r < 0) ERR_EXIT(_r); } while (0)





DASBuddy::DASBuddy(bool deletefile,QWidget *parent) : QWidget(parent) ,
  ui(new Ui::DASBuddy)
{

    ui->setupUi(this);

    initConnectSignalSlot();
    initProgressBarStyle();
    setUploadProgressBar(0);
    initLableTextStyle();


    isConnect = false;

    ui->devicePairButton->setVisible(false);

    ui->pushButton_zfy->setVisible(false);
    ui->pushButton_sdt->setVisible(false);

    mainIdx = -1;
    mainDiskPath = "";
    beginCalcTime = -1;

    QTimer * _dbusTime = new QTimer(this);
    connect(_dbusTime,&QTimer::timeout,this,[=](){

        qDebug() << __FUNCTION__ << " ======================= mainIdx:" <<  mainIdx << ", mainDiskPath:" <<  mainDiskPath << " ======================= " ;
    });


}

DASBuddy::~DASBuddy()
{
//    qDebug()<<"DASBuddy::~DASBuddy "<<this->num<<endl;
}

void DASBuddy::setphoto(QString path)
{

    QPixmap *image = new QPixmap(path);
//    QPixmap fitpixmap=image->scaled(label_photo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//    label_photo->setPixmap(fitpixmap);
}
void DASBuddy::setNetworkPic(const QString &szUrl)
{
    QUrl url(szUrl);
    QNetworkAccessManager manager;
    QEventLoop loop;

    // qDebug() << "Reading picture form " << url;
    QNetworkReply *reply = manager.get(QNetworkRequest(url));
    //请求结束并下载完成后，退出子事件循环
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();

    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);



}

void DASBuddy::setPhotoText(QString text){
       ui->serlabel->setText(text);
       ui->serlabel->setAlignment(Qt::AlignCenter);

       QString l_text = QString("%1空闲").arg(text);
        //ui->labelpage1->setText(text + "空闲");
       ui->labelpage1->setText(l_text);
        ui->labelpage1->setAlignment(Qt::AlignCenter);
}

void DASBuddy::setUploadProgressBar(int progress){
    if(progress == 100){
        ui->uploadprogressBar->setStyleSheet(progressBarStyleComplete);
    }else{
        ui->uploadprogressBar->setStyleSheet(progressBarStyleCollecting);
    }
    ui->uploadprogressBar->setValue(progress);
}
void DASBuddy::setBackgroundColor(QColor color){
    //QString("%1").arg(path[0], 0, 10)
    //"background-color: white"
    frame_background->setStyleSheet(QString("background-color: %1").arg(converRGB2HexStr(color)));
}

QString DASBuddy::converRGB2HexStr(QColor _rgb)

{
    QString redStr = QString("%1").arg(_rgb.red(),2,16,QChar('0'));

    QString greenStr = QString("%1").arg(_rgb.green(),2,16,QChar('0'));

    QString blueStr = QString("%1").arg(_rgb.blue(),2,16,QChar('0'));

    QString hexStr = "#"+redStr+greenStr+blueStr;

    return hexStr;

}

int DASBuddy::getMybattery() const
{
    return mybattery;
}

void DASBuddy::setMybattery(const int &value)
{
    mybattery = value;
//    Battery->setText(QString("%1%2").arg(mybattery).arg("%"));
}

static int get_mass_storage_status(libusb_device_handle *handle, uint8_t endpoint, uint32_t expected_tag)
{
    int i, r, size;
    struct command_status_wrapper csw;

    // The device is allowed to STALL this transfer. If it does, you have to
    // clear the stall and try again.
    i = 0;
    do {
        r = libusb_bulk_transfer(handle, endpoint, (unsigned char*)&csw, 13, &size, 1000);
        if (r == LIBUSB_ERROR_PIPE) {
            libusb_clear_halt(handle, endpoint);
        }
        i++;
    } while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));
    if (r != LIBUSB_SUCCESS) {
        qDebug()<<"   get_mass_storage_status:" << libusb_strerror((enum libusb_error)r);
        return -1;
    }
    if (size != 13) {
        qDebug()<<"   get_mass_storage_status: received "<<size<<" bytes (expected 13)";
        return -1;
    }
    if (csw.dCSWTag != expected_tag) {
        qDebug()<<"   get_mass_storage_status: mismatched tags (expected"<<expected_tag<<", received " << csw.dCSWTag<<")";
        return -1;
    }
    // For this test, we ignore the dCSWSignature check for validity...
    qDebug()<<"   Mass Storage Status: " << csw.bCSWStatus <<" "<< (csw.bCSWStatus?"FAILED":"Success\n");
    if (csw.dCSWTag != expected_tag)
        return -1;
    if (csw.bCSWStatus) {
        // REQUEST SENSE is appropriate only if bCSWStatus is 1, meaning that the
        // command failed somehow.  Larger values (2 in particular) mean that
        // the command couldn't be understood.
        if (csw.bCSWStatus == 1)
            return -2;	// request Get Sense
        else
            return -1;
    }

    // In theory we also should check dCSWDataResidue.  But lots of devices
    // set it wrongly.
    return 0;
}

int DASBuddy::transferBulkData(libusb_device_handle * usbHandle,command_block_wrapper * cbw,int data_length,unsigned char * buffer ) {

    //    QThread::msleep(500);
    int transferredLength = 0;


    int i = 0,r = 0;

    do {

        r = libusb_bulk_transfer(usbHandle,ENDPOINT_OUT,(unsigned char *)cbw,31,&transferredLength,1000);
        QString errLib = libusb_error_name(r);
        int cbw_b = sizeof(cbw);
        qDebug()<<"cbw"<<cbw_b<<"transferredLength"<<transferredLength<<"errLib"<<errLib;
        if (r == LIBUSB_ERROR_PIPE) {
            libusb_clear_halt(usbHandle, ENDPOINT_OUT);
            qDebug()<<"LIBUSB_ERROR_PIPE ERROR, clear";
        }
        i++;
    } while ((r == LIBUSB_ERROR_PIPE) && (i<RETRY_MAX));

    if (r==0 && transferredLength == 31) {
        qDebug()<<"\ntransferBulkData:Write USBC successful!";
    }
    else {
        qDebug()<<("\ntransferBulkData:Write USBC error, code")<<r;
        return r;
    }


    if (buffer) {
        memset(buffer, 0, 64);

        CALL_CHECK(libusb_bulk_transfer(usbHandle, ENDPOINT_IN, (unsigned char *)buffer, data_length, &transferredLength, 1000));



        if (r==0) {
            qDebug()<<"transferBulkData:Received:"<<transferredLength<<" chars";
            qDebug()<<QLatin1String((const char *)buffer,transferredLength);
            //            ui->lineResponse->setText(QLatin1String((char *)strResponse,transferredLength));

        }
        else {
            qDebug()<<"transferBulkData:Read error"<<r;
            return r;
        }
    }

    return get_mass_storage_status(usbHandle,ENDPOINT_IN,cbw->dCBWTag);

}

void DASBuddy::setWritingnow(const QString &value)
{
    writingnow = value;
}

QString DASBuddy::getWritingnow() const
{
    return writingnow;
}


void DASBuddy::closeThread(){


    if(controlthread !=nullptr){
        if(controlthread->isRunning())
        {
            qDebug()<<tr("关闭线程");
            controlthread->quit();            //退出事件循环
            controlthread->wait();            //释放线程槽函数资源

        }
    }

}

void DASBuddy::finishedThreadSlot(){

    qDebug()<<tr("多线程触发了finished信号");
}

void DASBuddy::requestFinished(QNetworkReply* reply) {

    // 获取http状态码
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        qDebug() << "reason=" << reason.toString();

    QNetworkReply::NetworkError err = reply->error();
    if(err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    }
    else {
        // 获取返回内容
        QString str = reply->readAll();
        qDebug() << str;
        QJsonParseError err_rpt;
        QJsonDocument  root_Doc = QJsonDocument::fromJson(str.toLocal8Bit(), &err_rpt);//字符串格式化为JSON

        if(err_rpt.error != QJsonParseError::NoError)
        {
            qDebug() << "JSON格式错误";
            //return -1;
        }else{
            QJsonObject root_Obj = root_Doc.object();
            int result_status = root_Obj.value("status").toInt();
            if(result_status == 0){
                //fail
                QString result_msg = root_Obj.value("msg").toString();
                qDebug()<<"result_status:"<<result_status<<"msg:"<<result_msg;

                //QMessageBox::warning(NULL, "Login Fail", result_msg, QMessageBox::Ok );
            }
            if(result_status == 1){
                //success
                QJsonObject result_data = root_Obj.value("data").toObject();
                QString username = result_data.value("username").toString();
                QString imagepath = result_data.value("imagepath").toString();

                qDebug()<<"username:"<<username<<"imagepath:"<<imagepath;

                this->setNetworkPic(imagepath);

            }

        }

    }
}



void DASBuddy::initConnectSignalSlot(){

    connect(ui->prioritypushButton, SIGNAL(clicked()), this, SLOT(onPriorityPushButtonClicked()));
}



void DASBuddy::setDevicePairStatus(bool stat)
{
    ui->devicePairButton->setVisible(stat);
}


void DASBuddy::setDeviceShowAll(bool stat)
{

    ui->pushButton_zfy->setVisible(stat);
    ui->pushButton_sdt->setVisible(stat);
}


void DASBuddy::setDevicePairInfos(int idx, QString diskPath)
{
    this->mainIdx = idx;
    this->mainDiskPath = diskPath;

}


void DASBuddy::writeHsParamIni()
{
    int _idx = this->mainIdx;
    QString _path = this->mainDiskPath;

    QSettings settings(_path + "/Param.ini", QSettings::IniFormat);

    QLineEdit *deviceLineEdit = this->findChild<QLineEdit *>("deviceLineEdit");
    QLineEdit *numberLineEdit = this->findChild<QLineEdit *>("numberLineEdit");
    QLineEdit *nameLineEdit = this->findChild<QLineEdit *>("nameLineEdit");
    QLineEdit *passwdLineEdit = this->findChild<QLineEdit *>("passwdLineEdit");

    QString _devNo = deviceLineEdit->text();
    QString _usrNo = numberLineEdit->text();
    QString _admNo = nameLineEdit->text();
    QString _pwdNo = passwdLineEdit->text();

    settings.beginGroup("ZFY");
    // 写入字符串
    settings.setValue("DevNo", _devNo);
    settings.setValue("PolNo", _usrNo);



}



void DASBuddy::initProgressBarStyle(){
    lastCopySize = 0;
    lastCalcTime = 0;
    progressBarStyleCollecting = QString("QProgressBar {"
                                                 "  text-align: center;"
                                                 "  border: 1px solid grey;"
                                                 "  border-radius: 9px;"
                                                 "  background:qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, "
                                                 "  stop:0 #d0d0d0, stop:0.5 #f1f1f1, stop:0.98 #7d7d7d);"
                                                 "}"
                                                 "QProgressBar::chunk {"
                                                "  border-radius: 9px;"
                                                "  background:qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, "
                                                "  stop:0 #F48B28, stop:0.5 #FED776, stop:0.98 #F48B28);"
                                                 "}");
    progressBarStyleComplete = QString("QProgressBar {"
                                                 "  text-align: center;"
                                                 "  border: 1px solid grey;"
                                                 "  border-radius: 9px;"
                                                 "  background:qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, "
                                                 "  stop:0 #d0d0d0, stop:0.5 #f1f1f1, stop:0.98 #7d7d7d);"
                                                 "}"
                                                 "QProgressBar::chunk {"
                                                "  border-radius: 9px;"
                                                "  background:qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, "
                                                "  stop:0 #08DE16, stop:0.5 #8CE194, stop:0.98 #08DE16);"
                                                 "}");
    ui->uploadprogressBar->setStyleSheet(progressBarStyleCollecting);
}


void DASBuddy::initLableTextStyle(){

//     ui->uploadtimetextlabel->setStyleSheet("color: white;font-size: 9pt;");
//     ui->filestextlabel->setStyleSheet("color: white;font-size: 9pt;");
//     ui->acspeedlabel->setStyleSheet("color: white;font-size: 9pt;");
//     ui->acstatelabel->setStyleSheet("color: white;font-size: 9pt;");

//     ui->pertextlabel->setStyleSheet("color: white;font-size: 9pt;");
//     ui->devtextlabel->setStyleSheet("color: white;font-size: 9pt;");
//     ui->sectiontextlabel->setStyleSheet("color: white;font-size: 9pt;");
//     ui->nametextlabel->setStyleSheet("color: white;font-size: 9pt;");


     ui->uploadtimetextlabel->setStyleSheet("color: white;");
     ui->filestextlabel->setStyleSheet("color: white;");
     ui->acspeedlabel->setStyleSheet("color: white;");
     ui->acstatelabel->setStyleSheet("color: white;");

     ui->pertextlabel->setStyleSheet("color: white;");
     ui->devtextlabel->setStyleSheet("color: white;");
     ui->sectiontextlabel->setStyleSheet("color: white;");
     ui->nametextlabel->setStyleSheet("color: white;");
     ui->perlabel->setStyleSheet("color: white;");
     ui->perlabel->setText("人员编号：");
     ui->devlabel->setStyleSheet("color: white;");
     ui->devlabel->setText("设备编号：");
     ui->namelabel->setStyleSheet("color: white;");
     ui->namelabel->setText("姓名：");
     ui->sectionlabel->setStyleSheet("color: white;");
     ui->sectionlabel->setText("部门：");
     ui->fileslabel->setStyleSheet("color: white;");
     ui->fileslabel->setText("文件总数:");

     //ui->acspeedlabel->setText("采集速度");

     ui->uploadtimelabel->setStyleSheet("color: white;");
     //ui->uploadtimelabel->setText("剩余时长:");
     ui->uploadtimelabel->setText("已用时长:");

}


void DASBuddy::updateToalTime()
{

    if(beginCalcTime == -1){

        beginCalcTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    qint64 endCalcTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 workTime = endCalcTime - beginCalcTime;
    QString strSumTime = "";
    if(workTime > 60 * 1000 ){
       //QString::number(workTime) + "" + "分钟"
       strSumTime = QString::number(workTime /  (60 * 1000.0) ,'f',1) + "" + "分";

    }else{
       //QString::number(workTime) + "" + "秒"
       strSumTime = QString::number(workTime / 1000) + "" + "秒";
    }
    ui->uploadtimetextlabel->setText(strSumTime);


}

void DASBuddy::testThmodthid()
{

     int a = 100;

     ui->pushButton_zfy->setVisible(true);
     ui->pushButton_sdt->setVisible(true);
}


void DASBuddy::setCurrentCopyProgressInfo(qreal progress,qint64 copiedCompleteSize,qint64 fileTotalSize){


    updateToalTime();

    qreal _totalSizes = fileTotalSize/1024.0/1024.0;  //unit: MB
    qreal _currSizes = copiedCompleteSize/1024.0/1024.0;  //unit: MB
    QString _currStrSizes = QString::number(_currSizes,'f',2);
    QString _totalStrSizes = QString::number(_totalSizes,'f',2);
    ui->totalsizeslabel->setStyleSheet("color: white;");
    ui->totalsizeslabel->setText(_currStrSizes + "/" + _totalStrSizes +"M");

    if(copiedCompleteSize >= fileTotalSize){

        ui->uploadprogressBar->setStyleSheet(progressBarStyleComplete);
        ui->uploadprogressBar->setValue(100);
        setCurrentStatus(3);

    }else{
        double _progress = static_cast<double>(copiedCompleteSize) / fileTotalSize * 100;
         ui->uploadprogressBar->setValue(_progress);

    }

    return ;



    //time million second
    qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if(lastCalcTime == 0){
        lastCalcTime = timestamp;
        lastCopySize = copiedCompleteSize;
    }
    qreal realSpeed = 0.0;
    if(timestamp - lastCalcTime  > 1000){
        //calc the speed.  copy speed = detal(copiedCompeleteSize) / detal(time)
        qint64 dSize = copiedCompleteSize - lastCopySize;
        if(dSize < 0){
            return;
        }
        qint64 dTime = timestamp - lastCalcTime;
        qreal realSize = dSize/1024.0/1024.0;  //unit: MB
        qreal realTime = dTime / 1000.0;
        if(realTime >= 1.0){
            realSpeed = realSize / realTime;
            QString valueStr = QString::number(realSpeed, 'f', 2);


            //ui->acspeedlabel->setText(valueStr + "" + "MB/s");

            lastCalcTime = timestamp;
            lastCopySize = copiedCompleteSize;
        }
    }
    //calc copy time
    if(realSpeed != 0.0){
        qint64 notCopySize = fileTotalSize - copiedCompleteSize;
        if(notCopySize < 0){
            return;
        }
        qreal realNotCopySize = notCopySize/1024/1024;
        qreal notCpyTime = realNotCopySize / realSpeed / 60; //minutes
        qreal roundedValue = qRound(notCpyTime);


        //ui->uploadtimetextlabel->setText(QString::number(roundedValue) + "" + "分钟");

    }


}



void DASBuddy::setCurrentFileTotalCount(qint64 fileTotalCount,qint64 cpySuccessFileCount){

    QString str;
    str = QString::number(cpySuccessFileCount) + "/" + QString::number(fileTotalCount);
    ui->filestextlabel->setText(str);

    //ui->totalsizeslabel->setText("");

}

void DASBuddy::setCurrentUserAndDevice(QString userNo, QString deviceNo){

    ui->pertextlabel->setText(userNo);
    ui->devtextlabel->setText(deviceNo);
    setCurrentStatus(1);
}

void DASBuddy::setCurrentUserNameAndDepartment(QString userName, QString department){

    ui->nametextlabel->setText(userName);
    ui->sectiontextlabel->setText(department);
}

void DASBuddy::setCurrentStatus(int status){

    QString imagePath = ":/image/collectionbackground.png";

    switch (status) {
    case 0:
        //do nohting.
        imagePath = ":/image/collectionbackground.png";
        statusDoNothing();

        beginCalcTime = -1;

        break;
    case 1:
        //now copy files.
        imagePath = ":/image/start.png";
        statusCopyFils();

        beginCalcTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

        break;
    case 3:
        //copy complete.
        imagePath = ":/image/done.png";

        beginCalcTime = -1;

        statusCopyComplete();




        break;
    default:
        break;
    }

    // 设置背景图片
    QPalette p;
    p.setBrush(this->backgroundRole(),
            QBrush(QPixmap(imagePath).scaled(    // 缩放背景图.
                this->size(),
                Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation)));    // 使用平滑的缩放方式
    this->setPalette(p);
    this->setAutoFillBackground(true);



}

void DASBuddy::statusDoNothing(){

    QString _idxText = QString("%1").arg(nNumber +1,2,10,QChar('0'));

    ui->stackedWidget->setCurrentIndex(0);
    QString l_text = QString("%1空闲").arg(_idxText);
     //ui->labelpage1->setText(text + "空闲");
    ui->labelpage1->setText(l_text);
    ui->labelpage1->setAlignment(Qt::AlignCenter);
    ui->labelpage1->show();

//    ui->perlabel->hide();
//    ui->pertextlabel->hide();
//    ui->devlabel->hide();
//    ui->devtextlabel->hide();
//    ui->namelabel->hide();
//    ui->nametextlabel->hide();
//    ui->sectionlabel->hide();
//    ui->sectiontextlabel->hide();
////    ui->powerlabel->hide();
//    ui->prioritypushButton->hide();
//    ui->fileslabel->hide();
//    ui->filestextlabel->hide();
//    ui->acstatelabel->hide();
//    ui->acspeedlabel->hide();
//    ui->uploadprogressBar->hide();
//    ui->uploadtimelabel->hide();
//    ui->uploadtimetextlabel->hide();
//    ui->acstatelabel->setText("");
//    ui->powerlabel->hide();
////    ui->powerlabel->setText("空闲");
////    ui->powerlabel->setAlignment(Qt::AlignCenter);
////    ui->powerlabel->show();

////    ui->labelDoNothing->show();

//    ui->namelabel->hide();
//    ui->nametextlabel->hide();
//    ui->sectionlabel->hide();
//    ui->sectiontextlabel->hide();
//    ui->powerlabel->hide();

//    // 准备新的 QLabel 对象来显示 "空闲"
//    idleLabel = new QLabel("空闲"); // 'this' 指向当前的 QWidget
//    idleLabel->setAlignment(Qt::AlignCenter); // 设置文本居中
//    idleLabel->setStyleSheet("font-size: 24px; color: white;"); // 设置样式（可选）

//    // 在 grid layout 的指定位置添加 idleLabel
//    int rowCount = ui->gridLayout->rowCount();
//    int colCount = ui->gridLayout->columnCount();

//    // 将 idleLabel 添加到 GridLayout 的中心位置
//    ui->gridLayout->addWidget(idleLabel, rowCount / 3, colCount / 2, 1, 1, Qt::AlignCenter);

    // 检查 idleLabel 是否已存在，如果不存在则创建它
//    if (idleLabel == nullptr) {
//        idleLabel = new QLabel("空闲", this); // 用当前对象作为父级
//        idleLabel->setAlignment(Qt::AlignCenter);
//        idleLabel->setStyleSheet("font-size: 24px; color: white;");


//        // 在 grid layout 的指定位置添加 idleLabel
//        int rowCount = ui->gridLayout->rowCount();
//        int targetRow = (rowCount / 2 + rowCount / 3) / 2;
//        int colCount = ui->gridLayout->columnCount();
//        ui->gridLayout->addWidget(idleLabel, targetRow, colCount / 2, 1, 1, Qt::AlignCenter);
//    }

//    // 显示 idleLabel
//    idleLabel->show();


}

void DASBuddy::statusCopyFils(){


//    if (idleLabel != nullptr) {
//        idleLabel->hide();
//    }
    ui->stackedWidget->setCurrentIndex(1);
    ui->perlabel->show();
    ui->pertextlabel->show();
    ui->devlabel->show();
    ui->devtextlabel->show();
    ui->namelabel->show();
    ui->nametextlabel->show();
    ui->sectionlabel->show();
    ui->sectiontextlabel->show();
    ui->prioritypushButton->show();
    ui->fileslabel->show();
    ui->filestextlabel->show();
    ui->acstatelabel->show();
    ui->acspeedlabel->show();
    ui->uploadprogressBar->show();
    ui->uploadtimelabel->show();
    ui->uploadtimetextlabel->show();
    ui->acstatelabel->setText("正在采集");
    ui->powerlabel->setText("充电中");
    ui->powerlabel->setAlignment(Qt::AlignCenter);

//    ui->labelDoNothing->hide();

    ui->namelabel->show();
    ui->nametextlabel->show();
    ui->sectionlabel->show();
    ui->sectiontextlabel->show();
    ui->powerlabel->show();
}

void DASBuddy::statusCopyComplete(){
    ui->stackedWidget->setCurrentIndex(1);
    ui->perlabel->show();
    ui->pertextlabel->show();
    ui->devlabel->show();
    ui->devtextlabel->show();
    ui->namelabel->show();
    ui->nametextlabel->show();
    ui->sectionlabel->show();
    ui->sectiontextlabel->show();
//    ui->powerlabel->show();

    ui->fileslabel->show();
    ui->filestextlabel->show();
    ui->acstatelabel->show();
    ui->acspeedlabel->show();
    ui->uploadprogressBar->show();
    ui->uploadtimelabel->show();
    ui->uploadtimetextlabel->show();
    ui->acstatelabel->setText("采集完成");

    ui->uploadprogressBar->setStyleSheet(progressBarStyleComplete);
    ui->uploadprogressBar->setValue(100);

    ui->prioritypushButton->hide();
//    ui->powerlabel->hide();
//    ui->labelDoNothing->hide();

    ui->namelabel->show();
    ui->nametextlabel->show();
    ui->sectionlabel->show();
    ui->sectiontextlabel->show();
    ui->powerlabel->show();

}



void DASBuddy::onPriorityPushButtonClicked(){

    printf("DASBuddy::onPriorityPushButtonClicked()--\n");
    isPriority = !isPriority;


    emit setPriority(getDasbuddyNumber(),isPriority);
    if(isPriority){
        ui->prioritypushButton->setText("取消优先采集");
    }else{
        ui->prioritypushButton->setText("优先采集");
    }
}




void DASBuddy::on_devicePairButton_clicked()
{

        QDialog dialog;
       //dialog.setParent(this);
       dialog.setWindowTitle("关联设备");
       QFormLayout *formLayout = new QFormLayout;

       QLabel *nameLabel = new QLabel("管理员:");
       nameLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");
       QLineEdit *nameLineEdit = new QLineEdit;
       nameLineEdit->setObjectName("nameLineEdit");
       nameLineEdit->setStyleSheet("QLineEdit{ border: none; color: rgb(255, 255, 255); font: 16pt \"Sans Serif\"; opacity: 0; }");

       //formLayout->addRow(nameLabel, nameLineEdit);

       QLabel *passwdLabel = new QLabel("密  码:");
        passwdLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");
       QLineEdit *passwdLineEdit = new QLineEdit;
       passwdLineEdit->setObjectName("passwdLineEdit");
       passwdLineEdit->setStyleSheet("QLineEdit{ border: none; color: rgb(255, 255, 255); font: 16pt \"Sans Serif\"; opacity: 0; }");
       //passwdLineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
       passwdLineEdit->setEchoMode(QLineEdit::Password);

       //formLayout->addRow(passwdLabel, passwdLineEdit);

       QLabel *deviceLabel = new QLabel("设备号:");
        deviceLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");
       QLineEdit *deviceLineEdit = new QLineEdit;
       deviceLineEdit->setObjectName("deviceLineEdit");
        deviceLineEdit->setStyleSheet("QLineEdit{ border: none; color: rgb(255, 255, 255); font: 16pt \"Sans Serif\"; opacity: 0; }");
       formLayout->addRow(deviceLabel, deviceLineEdit);

       QLabel *numberLabel = new QLabel("用户号:");
        numberLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");
       QLineEdit *numberLineEdit = new QLineEdit;
       numberLineEdit->setObjectName("numberLineEdit");
       numberLineEdit->setStyleSheet("QLineEdit{ border: none; color: rgb(255, 255, 255); font: 16pt \"Sans Serif\"; opacity: 0; }");
       formLayout->addRow(numberLabel, numberLineEdit);


       QPushButton *okButton = new QPushButton("关联");
       okButton->setStyleSheet("QPushButton{background-image: url(:/image/loginreveal.png);color: rgb(255, 255, 255);}"
                               "QPushButton:pressed{background-color:rgb(85, 170, 255); border-style: inset;}");
       okButton->resize(150,41);
       okButton->setFixedSize(150,41);

       QPushButton *cancelButton = new QPushButton("取消");
       cancelButton->setStyleSheet("QPushButton{background-image: url(:/image/loginreveal.png);color: rgb(255, 255, 255);}"
                                "QPushButton:pressed{background-color:rgb(85, 170, 255); border-style: inset;}");
       cancelButton->resize(81,41);
       cancelButton->setFixedSize(81,41);

       formLayout->addRow(cancelButton,okButton);

       dialog.setLayout(formLayout);

       //dialog.setAttribute(Qt::WA_DeleteOnClose);
       //dialog.resize(300,200);

       dialog.resize(300,100);
       dialog.setStyleSheet("background-image: url(:/image/usermanagement8321.png);");
       dialog.setWindowFlag(Qt::FramelessWindowHint);




      // connect(okButton,&QPushButton::clicked,this,&DASBuddy::writeHsParamIni);
       connect(okButton,&QPushButton::clicked,this,[=,&dialog](){

           int _idx = this->mainIdx;
           QString _path = this->mainDiskPath;

           QString _devNo = deviceLineEdit->text();
           QString _usrNo = numberLineEdit->text();
           QString  _fileName = _path + "/Param.ini";
           QFile _file(_fileName);
           if (!_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {
               // 处理打开文件失败的情况
               QMessageBox::information(nullptr, "权限异常", "无法创建文件,请确开启写入权限!");
               return;
           }

           QByteArray bom;
           bom.append(char(0xEF));
           bom.append(char(0xBB));
           bom.append(char(0xBF));
           _file.write(bom);

           //2.
           QTextStream _stream(&_file);
           //QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\n";
           QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\nUserName=\r\nDepartment=";

           _stream.setCodec("UTF-8");
           _stream << _text;
           _file.flush();
           _file.close();


           QString _hsFileNameIni(_path + "/hsAuth.ini");
           QFile _hsKeyFile(_hsFileNameIni);
           if (_hsKeyFile.exists()) {
                _hsKeyFile.remove();
           }


           setDevicePairStatus(false);

           emit  sigOffWindowId(_idx,false);
           emit  sigDelCurrentlyCopying(_path);


           dialog.close();



       });

       connect(cancelButton,&QPushButton::clicked,&dialog,&QDialog::close);

       dialog.exec();


}



void DASBuddy::on_pushButton_zfy_clicked()
{


    QProcess onboard;
    onboard.start("bash", QStringList() << "-c" << "onboard");
    onboard.waitForStarted();

    emit senduserlist();


    ///////
    QDialog  dialog;
    //QDialog * dialog = new QDialog;

    int winidx =  getDasbuddyNumber();


    QHBoxLayout *horizontalLayout = new QHBoxLayout();

    QLabel *carLabel = new QLabel("车次:");
    carLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QLineEdit *carLineEdit = new QLineEdit;
    carLineEdit->setObjectName("carLineEdit");
    carLineEdit->setMinimumSize(160, 35);
    carLineEdit->setStyleSheet("QLineEdit {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");

    horizontalLayout->addWidget(carLabel);
    horizontalLayout->addWidget(carLineEdit);

    // 设置伸缩系数，button1: 1, button2: 2, button3: 1
    horizontalLayout->setStretch(0, 1);
    horizontalLayout->setStretch(1, 5);

    // 4.2 用户号
    QHBoxLayout *horizontalLayout1 = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("编号:");
    nameLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QLineEdit *nameLineEdit = new QLineEdit;
    nameLineEdit->setObjectName("nameLineEdit");
    nameLineEdit->setStyleSheet("QLineEdit {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");

    nameLineEdit->setMinimumSize(160, 35);
    // nameLineEdit->setEnabled(false);

    horizontalLayout1->addWidget(nameLabel);
    horizontalLayout1->addWidget(nameLineEdit);


    // 设置伸缩系数，button1: 1, button2: 2, button3: 1
    horizontalLayout1->setStretch(0, 1);
    horizontalLayout1->setStretch(1, 5);
    //horizontalLayout->setStretch(2, 1);



    QHBoxLayout *horizontalLayout2 = new QHBoxLayout();
    QLabel *userListLabel = new QLabel("");
    //numberLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QTableWidget*     tableWidget = new QTableWidget;


    // 表格
    int _userRows = global_userinfo_all.size();

    tableWidget->setColumnCount(2);
    tableWidget->setRowCount(_userRows);


    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setStyleSheet( "QScrollBar:vertical { width: 38px; background-color: #2a7fbf;}");


    QHeaderView* horizontalHeader = tableWidget->horizontalHeader();
    horizontalHeader->setStyleSheet("QHeaderView::section { background-color: #2a7fbf; font: 18pt \"Sans Serif\";  }");

    QHeaderView* verticalHeader = tableWidget->verticalHeader();
    verticalHeader->setStyleSheet("QHeaderView::section { background-color: #2a7fbf;   }");
    verticalHeader->setVisible(false);

    QStringList headers;
    headers << "编号" << "名字"  ;
    //headers << "" << ""  ;
    tableWidget->setHorizontalHeaderLabels(headers);

    // 行数
    int rowCount = tableWidget->rowCount();

    // 添加列
    int columnCount = tableWidget->columnCount();


    // 设置字体样式
    QFont font;
    //font.setBold(true); // 加粗
    //font.setItalic(true); // 斜体
    font.setFamily("Sans Serif");
    font.setPointSize(14); // 设置字号为12


    for (int j = 0; j < rowCount; ++j) {

       for (int i = 0; i < columnCount; ++i) {
           //     QString F_UserNo;
           //     QString F_DeviceNo;
           //     QString F_UserName;
           //     global_userinfo_all

           QTableWidgetItem *useritem = new QTableWidgetItem(global_userinfo_all[j].F_UserNo);
           // 设置item为不可编辑
           useritem->setFlags(useritem->flags() & ~Qt::ItemIsEditable);
           tableWidget->setItem(j, 0, useritem);
           useritem->setFont(font);

           tableWidget->setColumnWidth(0, 180);
           // tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // 让列自动拉伸以填充剩余空间


           QTableWidgetItem *devitem = new QTableWidgetItem(global_userinfo_all[j].F_UserName);
           // 设置item为不可编辑
           devitem->setFlags(devitem->flags() & ~Qt::ItemIsEditable);
           tableWidget->setItem(j, 1, devitem);
           devitem->setFont(font);

          // tableWidget->setColumnWidth(1, 180);
          tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // 让列自动拉伸以填充剩余空间

           // tableWidget->setItem(j, i, devitem);
       }
    }



    horizontalLayout2->addWidget(userListLabel);
    horizontalLayout2->addWidget(tableWidget);

    horizontalLayout2->setStretch(0, 1);
    horizontalLayout2->setStretch(1, 5);


    // 4.4 按钮 栏
    QHBoxLayout *horizontalLayout3 = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    okButton->setStyleSheet("QPushButton {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");
    okButton->resize(150,41);
    okButton->setFixedSize(150,41);

    QPushButton *cancelButton = new QPushButton("取消");
    cancelButton->setStyleSheet("QPushButton {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");
    cancelButton->resize(150,41);
    cancelButton->setFixedSize(150,41);

    horizontalLayout3->addWidget(okButton);
    horizontalLayout3->addWidget(cancelButton);

    // 4.5 占位布局
    QHBoxLayout*     h_layout = new QHBoxLayout;

    // 创建垂直布局
    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addLayout(horizontalLayout); // 将水平布局添加到垂直布局
    verticalLayout->addLayout(horizontalLayout1);
    verticalLayout->addLayout(horizontalLayout2);
    verticalLayout->addLayout(horizontalLayout3);
    //verticalLayout->addLayout(h_layout);

    //verticalLayout->setStretchFactor(horizontalLayout,1);
    //verticalLayout->setStretchFactor(horizontalLayout2,8);

    verticalLayout->setStretch(0,1);
    verticalLayout->setStretch(1,1);
    verticalLayout->setStretch(2,8);
    verticalLayout->setStretch(3,1);
    //verticalLayout->setStretch(4,1);


    QObject::connect(tableWidget, &QTableWidget::itemSelectionChanged, this, [=](){

       QList<QTableWidgetItem*> list= tableWidget->selectedItems();
       if(list.count() <= 0)
       {
            //QMessageBox::warning(nullptr, "警告", "请选择一条数据!");

       }else{

            QList<QTableWidgetItem*> list = tableWidget->selectedItems();
            QSet<int> _selectedRows;

            for (QTableWidgetItem* item : list) {
                int row = item->row();
                _selectedRows.insert(row);
            }

            for (int _row : _selectedRows) {

                QWidget* pWidget = 0;
                pWidget = tableWidget->cellWidget(_row,0);

                QString cellText = tableWidget->item(_row, 0)->text();
                nameLineEdit->setText(cellText);

                //QMessageBox::warning(nullptr, "警告", cellText);
            }

       }



    });


    QObject::connect(okButton, &QPushButton::clicked ,[=,&dialog](){

       QString _usrNo =  nameLineEdit->text();
       QString _devNo =  carLineEdit->text();

       if(_usrNo.isEmpty() || _devNo.isEmpty()){
           QMessageBox::warning(nullptr, "警告", "请填写用户、车次!");
           return;

       }

       /*获取选中的列表里的所有条目*/
      QList<QTableWidgetItem*> list= tableWidget->selectedItems();
      if(list.count() <= 0)
      {
           QMessageBox::warning(nullptr, "警告", "请选择一条数据!");
           return;

      }

      QSet<int> _selectedRows;


      for (QTableWidgetItem* item : list) {
          int row = item->row();
          _selectedRows.insert(row);
      }



      QString _undata = "";
      for (int _row : _selectedRows) {

          qDebug() << "Selected row:" << _row;
          //qDebug() << tableWidget->item(_row,1) ;
           _undata = tableWidget->item(_row,1)->text();

           qDebug() << "Selected row _undata :" <<  _undata;

      }


      int _idx = this->mainIdx;
      QString _path = this->mainDiskPath;


      if(true){

          // delete Param.ini
          QString  _paramFileNameIni = _path + "/Param.ini";

          QFile _paramKeydel(_paramFileNameIni);
          if (_paramKeydel.exists()) {
              bool _brm = _paramKeydel.remove();
              _paramKeydel.close();

              if(!_brm){
                  QMessageBox::information(nullptr, "权限异常", "请确开启写入权限!");
                  return;

              }

          }

          QString  _fileName = _path + "/Param.ini";
          QFile _file(_fileName);
          if (!_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {
              // 处理打开文件失败的情况
              QMessageBox::information(nullptr, "权限异常", "无法创建文件,请确开启写入权限!");
              return;
          }


          QByteArray bom;
          bom.append(char(0xEF));
          bom.append(char(0xBB));
          bom.append(char(0xBF));
          _file.write(bom);

          //2.
          QTextStream _stream(&_file);
          //QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\n";
          //QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\nUserName=我的姓名001\r\nDepartment=我的单位名称001";

          QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\n";

          _stream.setCodec("UTF-8");
          _stream << _text;
          _file.flush();
          _file.close();


      }


      emit  currentDasBusDeviceInfo(this->nNumber,  _usrNo,  _devNo ,  _undata,  "");


      emit  sendDeviceMessage(0,true,winidx);

      ui->devicePairButton->setVisible(false);
      ui->pushButton_zfy->setVisible(false);
      ui->pushButton_sdt->setVisible(false);


      dialog.close();






    });





    // 应用布局到窗口
    dialog.setLayout(verticalLayout);
    //dialog.setParent(this);

    dialog.resize(500,700);
    dialog.setStyleSheet("background-image: url(:/image/usermanagement8321.png);");
    dialog.setWindowFlag(Qt::FramelessWindowHint);
    //dialog.setAttribute(Qt::WA_DeleteOnClose); // 55

    connect(cancelButton,&QPushButton::clicked,&dialog,&QDialog::close);



    dialog.exec();




}


void DASBuddy::on_pushButton_sdt_clicked()
{

    QProcess onboard;
    onboard.start("bash", QStringList() << "-c" << "onboard");
    onboard.waitForStarted();

    emit senduserlist();


    ///////
    QDialog dialog;

    int winidx =  getDasbuddyNumber();

    QHBoxLayout *horizontalLayout = new QHBoxLayout();

    QLabel *carLabel = new QLabel("车次:");
    carLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QLineEdit *carLineEdit = new QLineEdit;
    carLineEdit->setObjectName("carLineEdit");
    carLineEdit->setMinimumSize(160, 35);
    carLineEdit->setStyleSheet("QLineEdit {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");

    horizontalLayout->addWidget(carLabel);
    horizontalLayout->addWidget(carLineEdit);


    horizontalLayout->setStretch(0, 1);
    horizontalLayout->setStretch(1, 5);

    // 4.2 用户号
    QHBoxLayout *horizontalLayout1 = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("编号:");
    nameLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QLineEdit *nameLineEdit = new QLineEdit;
    nameLineEdit->setObjectName("nameLineEdit");
    nameLineEdit->setStyleSheet("QLineEdit {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");

    nameLineEdit->setMinimumSize(160, 35);
    // nameLineEdit->setEnabled(false);

    horizontalLayout1->addWidget(nameLabel);
    horizontalLayout1->addWidget(nameLineEdit);


    // 设置伸缩系数，button1: 1, button2: 2, button3: 1
    horizontalLayout1->setStretch(0, 1);
    horizontalLayout1->setStretch(1, 5);
    //horizontalLayout->setStretch(2, 1);

    QHBoxLayout *horizontalLayout2 = new QHBoxLayout();
    QLabel *userListLabel = new QLabel("");
    //numberLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QTableWidget*     tableWidget = new QTableWidget;



    int _userRows = global_userinfo_all.size();

    tableWidget->setColumnCount(2);
    tableWidget->setRowCount(_userRows);


    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    tableWidget->setStyleSheet( "QScrollBar:vertical { width: 38px; background-color: #2a7fbf;}");

    QHeaderView* horizontalHeader = tableWidget->horizontalHeader();
    horizontalHeader->setStyleSheet("QHeaderView::section { background-color: #2a7fbf; font: 18pt \"Sans Serif\";  }");

    QHeaderView* verticalHeader = tableWidget->verticalHeader();
    verticalHeader->setStyleSheet("QHeaderView::section { background-color: #2a7fbf;   }");
    verticalHeader->setVisible(false);


    QStringList headers;
    headers << "编号" << "名字"  ;
    //headers << "" << ""  ;
    tableWidget->setHorizontalHeaderLabels(headers);

    // 行数
    int rowCount = tableWidget->rowCount();

    // 添加列
    int columnCount = tableWidget->columnCount();


    // 设置字体样式
    QFont font;
    //font.setBold(true); // 加粗
    //font.setItalic(true); // 斜体
    font.setFamily("Sans Serif");
    font.setPointSize(14); // 设置字号为12


    for (int j = 0; j < rowCount; ++j) {

       for (int i = 0; i < columnCount; ++i) {
           //     QString F_UserNo;
           //     QString F_DeviceNo;
           //     QString F_UserName;
           //     global_userinfo_all

           QTableWidgetItem *useritem = new QTableWidgetItem(global_userinfo_all[j].F_UserNo);
           // 设置item为不可编辑
           useritem->setFlags(useritem->flags() & ~Qt::ItemIsEditable);
           tableWidget->setItem(j, 0, useritem);
           useritem->setFont(font);

           // tableWidget->setColumnWidth(0, 180);
           tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // 让列自动拉伸以填充剩余空间


           QTableWidgetItem *devitem = new QTableWidgetItem(global_userinfo_all[j].F_UserName);
           // 设置item为不可编辑
           devitem->setFlags(devitem->flags() & ~Qt::ItemIsEditable);
           tableWidget->setItem(j, 1, devitem);
           devitem->setFont(font);

           // tableWidget->setColumnWidth(1, 180);
           tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // 让列自动拉伸以填充剩余空间

           // tableWidget->setItem(j, i, devitem);
       }
    }



    horizontalLayout2->addWidget(userListLabel);
    horizontalLayout2->addWidget(tableWidget);

    horizontalLayout2->setStretch(0, 1);
    horizontalLayout2->setStretch(1, 5);


    // 4.4 按钮 栏
    QHBoxLayout *horizontalLayout3 = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    okButton->setStyleSheet("QPushButton {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");
    okButton->resize(150,41);
    okButton->setFixedSize(150,41);

    QPushButton *cancelButton = new QPushButton("取消");
    cancelButton->setStyleSheet("QPushButton {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");
    cancelButton->resize(150,41);
    cancelButton->setFixedSize(150,41);

    horizontalLayout3->addWidget(okButton);
    horizontalLayout3->addWidget(cancelButton);

    // 4.5 占位布局
    QHBoxLayout*     h_layout = new QHBoxLayout;

    // 创建垂直布局
    QVBoxLayout *verticalLayout = new QVBoxLayout();


    // verticalLayout->addLayout(horizontalLayout); // 将水平布局添加到垂直布局
    verticalLayout->addLayout(horizontalLayout1);
    verticalLayout->addLayout(horizontalLayout2);
    verticalLayout->addLayout(horizontalLayout3);
    //verticalLayout->addLayout(h_layout);

    //verticalLayout->setStretchFactor(horizontalLayout,1);
    //verticalLayout->setStretchFactor(horizontalLayout2,8);

    verticalLayout->setStretch(0,1);
    verticalLayout->setStretch(1,1);
    verticalLayout->setStretch(2,8);
    verticalLayout->setStretch(3,1);
    //verticalLayout->setStretch(4,1);



    QObject::connect(tableWidget, &QTableWidget::itemSelectionChanged, this, [=](){


        /*获取选中的列表里的所有条目*/
       QList<QTableWidgetItem*> list= tableWidget->selectedItems();
       if(list.count() <= 0)
       {
            //QMessageBox::warning(nullptr, "警告", "请选择一条数据!");

       }else{


           /*获取选中的列表里的所有条目*/
            QList<QTableWidgetItem*> list = tableWidget->selectedItems();


            // 存储行数
            QSet<int> _selectedRows;

            // 遍历选中的项目，获取它们的行数，并存储在selectedRows中
            for (QTableWidgetItem* item : list) {
                int row = item->row();
                _selectedRows.insert(row);
            }


            // 打印所有选中的行
            for (int _row : _selectedRows) {

                QWidget* pWidget = 0;
                pWidget = tableWidget->cellWidget(_row,0);

                QString cellText = tableWidget->item(_row, 0)->text();
                nameLineEdit->setText(cellText);

                //QMessageBox::warning(nullptr, "警告", cellText);
            }

       }



    });


    QObject::connect(okButton, &QPushButton::clicked ,[=,&dialog](){

       QString _usrNo =  nameLineEdit->text();
       QString _devNo =  carLineEdit->text();


       if(_usrNo.isEmpty()  ){
           QMessageBox::warning(nullptr, "警告", "请填写用户 !");
           return;

       }



      QList<QTableWidgetItem*> list= tableWidget->selectedItems();
      if(list.count() <= 0)
      {
           QMessageBox::warning(nullptr, "警告", "请选择一条数据!");
           return;

      }


      QSet<int> _selectedRows;


      for (QTableWidgetItem* item : list) {
          int row = item->row();
          _selectedRows.insert(row);
      }



      QString _undata = "";
      for (int _row : _selectedRows) {

          qDebug() << "Selected row:" << _row;
          //qDebug() << tableWidget->item(_row,1) ;
           _undata = tableWidget->item(_row,1)->text(); // 例如获取第2列的数据

           qDebug() << "Selected row _undata :" <<  _undata;

      }


      int _idx = this->mainIdx;
      QString _path = this->mainDiskPath;



      if(true){

          // delete Param.ini
          QString  _paramFileNameIni = _path + "/Param.ini";

          QFile _paramKeydel(_paramFileNameIni);
          if (_paramKeydel.exists()) {
              bool _brm = _paramKeydel.remove();
              _paramKeydel.close();

              if(!_brm){
                  QMessageBox::information(nullptr, "权限异常", "请确开启写入权限!");
                  return;

              }

          }



          QString  _fileName = _path + "/Param.ini";
          QFile _file(_fileName);
          if (!_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {
              // 处理打开文件失败的情况
              QMessageBox::information(nullptr, "权限异常", "无法创建文件,请确开启写入权限!");
              return;
          }


          QByteArray bom;
          bom.append(char(0xEF));
          bom.append(char(0xBB));
          bom.append(char(0xBF));
          _file.write(bom);

          //2.
          QTextStream _stream(&_file);
          //QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\n";
          //QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\nUserName=我的姓名001\r\nDepartment=我的单位名称001";

          QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\n";

          _stream.setCodec("UTF-8");
          _stream << _text;
          _file.flush();
          _file.close();



      }


      emit  currentDasBusDeviceInfo(this->nNumber,  _usrNo,  _devNo ,  _undata,  "");

      emit  sendDeviceMessage(1,true,winidx);

      ui->devicePairButton->setVisible(false);
      ui->pushButton_zfy->setVisible(false);
      ui->pushButton_sdt->setVisible(false);


      dialog.close();




    });




    // 应用布局到窗口
    dialog.setLayout(verticalLayout);
    //dialog.setParent(this);

    dialog.resize(500,700);
    dialog.setStyleSheet("background-image: url(:/image/usermanagement8321.png);");
    dialog.setWindowFlag(Qt::FramelessWindowHint);

    connect(cancelButton,&QPushButton::clicked,&dialog,&QDialog::close);


    dialog.exec();
    //dialog.show();

}



