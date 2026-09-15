#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uiprofile.h"
#include <QGuiApplication>
#include <QScreen>
#include "networkutility.h"
#include <QMediaPlayer>
#include <QVBoxLayout>
#include "xlsxdocument.h"
#include "disk.h"
#include <QSoundEffect>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>



bool is_mainwindow_exited = false;

std::mutex hsGlobalMtx;
std::mutex hsDbGlobalMtx;

namespace {
bool readFilesystemUsage(const QString &path, qint64 &totalBytes,
                         qint64 &usedBytes, qint64 &availableBytes)
{
    QProcess process;
    process.start("/bin/df", QStringList() << "-P" << "-B1" << path);
    if (!process.waitForFinished(800) || process.exitStatus() != QProcess::NormalExit
            || process.exitCode() != 0) {
        process.kill();
        return false;
    }

    const QStringList lines = QString::fromLocal8Bit(process.readAllStandardOutput())
            .split('\n', QString::SkipEmptyParts);
    if (lines.size() < 2) return false;
    const QStringList columns = lines.last().simplified().split(' ', QString::SkipEmptyParts);
    if (columns.size() < 6) return false;

    bool totalOk = false;
    bool usedOk = false;
    bool availableOk = false;
    const qint64 total = columns.at(1).toLongLong(&totalOk);
    const qint64 used = columns.at(2).toLongLong(&usedOk);
    const qint64 available = columns.at(3).toLongLong(&availableOk);
    if (!totalOk || !usedOk || !availableOk || total <= 0 || used < 0 || available < 0) return false;

    totalBytes = total;
    usedBytes = used;
    availableBytes = available;
    return true;
}
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    UiProfile::apply(this, "mainwindow");
    // 设置窗口无标题栏
    setWindowFlags(Qt::FramelessWindowHint);

    setFixedSize(this->width(),this->height());

    hsGlobalMtx.lock();
     Config::getInstance()->Set("wsConfig","dataPath",  Config::getInstance()->Get("wsConfig","saveDir").toString() + "/" +  Config::getInstance()->Get("wsConfig","saveFilePath").toString());
    Config::getInstance()->Sync();
    hsGlobalMtx.unlock();



    net = new NetworkUtility(this);
    // 创建 QLabel 显示时间
    timeLabel = new QLabel("00:00:00", this);
    timeLabel->setObjectName("timeLabel");
    timeLabel->setStyleSheet("color: rgb(255, 255, 255); font: 24pt 'Sans Serif';"); // 设置字体大小和颜色

    // 创建 QLabel 显示日期和星期
    dateTimeLabel = new QLabel("2020-8-31   星期一", this);
    dateTimeLabel->setObjectName("dateTimeLabel");
    dateTimeLabel->setStyleSheet("color: rgb(255, 255, 255); font: 12pt 'Sans Serif';");

    // 创建 QVBoxLayout 用于垂直排列时间和日期
    QWidget *dateTimeWidget = new QWidget(this);
    dateTimeWidget->setFixedSize(160, 80);
    // 日期时间仅用于显示。即使1280布局下与按钮区域发生重叠
    // 鼠标事件也应继续传递给下面的数据查询和系统设置按钮。
    dateTimeWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    QVBoxLayout *vBoxLayout = new QVBoxLayout(dateTimeWidget);
    vBoxLayout->addWidget(dateTimeLabel, 0, Qt::AlignCenter);
    vBoxLayout->addWidget(timeLabel, 0, Qt::AlignCenter);

    // 获取屏幕的宽度
    int screenWidth = QGuiApplication::primaryScreen()->geometry().width();

    // 设置时间、日期、星期标签位置偏移
    dateTimeWidget->move(screenWidth - dateTimeLabel->width() - 50, 0);

    // 更新日期时间的定时器
    QTimer *dateTimeTimer = new QTimer(this);
    connect(dateTimeTimer, &QTimer::timeout, [=](){
        QDateTime currentDateTime = QDateTime::currentDateTime();
        timeLabel->setText(currentDateTime.toString("hh:mm:ss"));
        dateTimeLabel->setText(currentDateTime.toString("yyyy-MM-dd   dddd")); // 使用一个 QLabel 更新日期和星期
    });
    dateTimeTimer->start(1000); // 每秒更新一次
    isVoicePlayback =  Config::getInstance()->Get("wsConfig", "isVoicePlayback").toBool();
    qDebug() << " ================= isVoicePlayback :"  << isVoicePlayback;

//    initActiveMq();

    AcStation_starttime = QDateTime::currentDateTime();
    initProcess();


    // 保持设置中选择的存储盘，不在启动时自动切换到其他 /media 分区。



    //通过配置文件是否自动删除执法仪已上传文件，可为true或者false。
    bool autodeletecopyfile =  Config::getInstance()->Get("wsConfig","deleteFile").toBool();
    const int configuredPortCount = Config::getInstance()->Get("portInfo", "port_num").toInt();
    portNumMainWindow = qBound(1, configuredPortCount, kMaxWorkstationPorts);
    if (portNumMainWindow != configuredPortCount) {
        qWarning() << "Configured port count is outside the supported range:" << configuredPortCount;
    }
    dasbuddy.resize(portNumMainWindow);

    for (int i=0;i<portNumMainWindow;i++) {
        dasbuddy[i] = new DASBuddy(autodeletecopyfile,this);
        dasbuddy[i]->setDasbuddyNumber(i);


    }


    timer_uploadDiskSize = new QTimer(this);
    connect(timer_uploadDiskSize, SIGNAL(timeout()), this, SLOT(uploaddisksize()));
    timer_uploadDiskSize->start(600000);


    timer_updateformlabel = new QTimer(this);
    connect(timer_updateformlabel, SIGNAL(timeout()), this, SLOT(getnetworkanddiskmsg()));
    timer_updateformlabel->start(60000);
    //获取当前系统盘(Linux系统就是根目录)的磁盘情况
    storage = QStorageInfo(configuredStoragePath());
    //copypath为执法仪文件拷贝到本地的路径copypath=/mnt/itventi/copyfile，需要注意结尾不含/
    //server为服务端MQ配置。ServerQueue为服务端MQ队列名，serverIp为服务端IP，用于定时发送ping命令判断网络状态，
    //serverManagerIp为后台管理Ip，serverMqIp以tcp://开头，为服务端Mq地址。
    pingip =  Config::getInstance()->Get("server","serverIp").toString();

    pingip = pingip.mid(pingip.indexOf("//")+2);
    pingip = pingip.mid(0,pingip.indexOf(":"));
//    qDebug()<<"pingip"<<pingip;
    storage.setPath(configuredStoragePath());
    //获取计算机根目录的总磁盘大小
    totaldisksize = QString::number(storage.bytesTotal()/1024.00/1024.00/1024.00,'f',2).toDouble();
    storage.refresh();  //获得最新磁盘信息
    freedisksize = QString::number(storage.bytesAvailable()/1024.00/1024.00/1024.00,'f',2).toDouble();


    MyHostIPV4Address = getHostIPV4Address().toString();

    //ui->label_hostip->settext(MyHostIPV4Address);
    mysqlite = new MySqlLite(this);

    connect(this,&MainWindow::deleteFilebySaveDay,mysqlite,&MySqlLite::delete_by_saveday);
    QVariant saveDay =  Config::getInstance()->Get("wsConfig","saveDays");
    int saveDays =0;
    if(!saveDay.isNull() && "-1" != saveDay.toString()){
        bool isOk = false;
        saveDays = saveDay.toInt(&isOk);
         if(isOk){
               emit deleteFilebySaveDay(saveDays);
          }
    };

    QTimer * _rmdiskTimer = new QTimer(this);
    QObject::connect(_rmdiskTimer, &QTimer::timeout, this,[=](){

        emit deleteFilebySaveDay(saveDays);
    });


    QVariant uploadSetting =  Config::getInstance()->Get("wsConfig", "Upload");
    if ("1" == uploadSetting.toString()) {
        connect(this, &MainWindow::UploadFilebyTime, mysqlite, &MySqlLite::UploadFilebyTime);
        connect(mysqlite, &MySqlLite::mysqlWorkFileClientAdd, net, &NetworkUtility::netWorkFileClientAdd);

        QString uploadDateTime =  Config::getInstance()->Get("wsConfig", "UploadDateTime").toString(); // "21:10:57"
        uploadTime = QTime::fromString(uploadDateTime, "HH:mm:ss"); // 将其设置为成员变量

        if (!uploadTime.isValid()) {
            qWarning() << "Invalid time format.";
            return; // 退出程序
        }

        // 创建上传定时器
        QTimer *uploadTimer = new QTimer(this);
        connect(uploadTimer, &QTimer::timeout, this, [this, uploadDateTime,uploadTimer]() {
            QTime currentTime = QTime::currentTime();
            if (currentTime >= uploadTime) {
                qDebug() << "The scheduled time has been reached! currentTime:" << currentTime<<"uploadDateTime:"<< uploadDateTime;
                emit UploadFilebyTime(uploadDateTime);
                uploadTimer->stop(); // 停止定时器
            }
        });

        uploadTimer->start(1000); // 每秒检查一次
    }


    form =new MainForm(dasbuddy,mysqlite,net,this);
    //form->setZFYSettingThread(zfysetting);
    //设置ip地址给页面
    form->setIPV4Label(MyHostIPV4Address);
    ui->stackedWidget->addWidget(form);

    ui->stackedWidget->setCurrentWidget(form);
    firstIndex = ui->stackedWidget->currentIndex();
    qDebug()<<"firstIndex:"<<firstIndex;
    qDebug()<<"currentWidget:"<<ui->stackedWidget->currentWidget();


    QVariantMap workStationInfo;
    workStationInfo["F_WorkStationIp"] =MyHostIPV4Address;
    workStationInfo["F_DiskSize"] =totaldisksize;
    workStationInfo["F_RemainSize"] =freedisksize;
    //connect(connectads,SIGNAL(passwordcorrect()),this,SLOT(changetoUdisk()));

    connect(form,SIGNAL(gosearch(QString,QString,bool)),this,SLOT(gotoSearchForm(QString,QString,bool)));
//    connect(form,SIGNAL(goPersonCenter(int,QString,QString)),this,SLOT(gotoSearchPersonCenter(int,QString,QString)));

    connect(this,SIGNAL(updateDiskSize(double,bool,double,double)),form,SLOT(updatedisksizelabel(double,bool,double,double)));
    connect(this,SIGNAL(sendHeartbeatTONetWork(double,double,const QString &)),net,SLOT(netWorkSendHeartbeat(double,double,const QString &)));
    connect(this,SIGNAL(sendClientUpdateModelTONet(const QVariantMap &)),net,SLOT(netWorkClientUpdateModel(const QVariantMap &)));

    connect(form,SIGNAL(toopenbroswer()),this,SLOT(openbroswer()));

    qDebug()<<"AcStation_starttime"<<AcStation_starttime;

    QString struuid = QUuid::createUuid().toString().remove("{").remove("}");
    QVariantMap map;
    map["TaskId"] = struuid;
    map["OperaType"] = 3;
    map["OperaContent"] = "Acquisition Station open time";
    map["Time"] = AcStation_starttime;

    emit sendClientUpdateModelTONet(workStationInfo);
    emit toinsert_AcStation_log(map);
    uploaddisksize();
    getnetworkanddiskmsg();

    QXlsx::Document xlsx;
    xlsx.write("A1", "Hello Qt!");
    xlsx.saveAs("Test.xlsx");



    //#ifdef USE_EXTERN_DISK

    connect(this,&MainWindow::signalStopDiskStat,this,&MainWindow::recStopDiskStat);

    // 检测很轻量，固定在主线程定时执行，避免后台无限线程操作界面或音频对象。
    QTimer * diskTimer = new QTimer(this);
    connect(diskTimer, &QTimer::timeout, this, [this](){
        recDsikMessage(offlineDropDisk());
    });
    diskTimer->setInterval(20000);
    diskTimer->start(20000);


    // #endif


    is_mainwindow_exited = false;

    int _isOffDisk = offlineDropDisk();

#ifdef USE_DATA_DISK
    if (_isOffDisk != 1) {
        QTimer::singleShot(1500, this, [this](){
            recDsikMessage(offlineDropDisk());
        });
    }

#endif



}



MainWindow::~MainWindow()
{
    // 首先，释放每个指针指向的内存
    for (int i = 0; i < portNumMainWindow; ++i) {
        delete  dasbuddy[i]; // 删除每个动态分配的数组
        dasbuddy[i] = nullptr;
    }

    qDebug()<<"~MainWindow exit!";
    delete ui;
}
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event); // 调用基类的方法以执行默认行为

    showFullScreen(); // 显示全屏
}
void MainWindow::gotoSearchForm(QString userId,QString roleId,bool videojur){
     gotoSearch_By_Files_Type(userId,roleId,videojur);

}


void MainWindow::gotoSearch_By_Files_Type(QString userid,QString roleId,bool videojur){

    if(searchbytype != nullptr){

    }

    searchbytype = new SearchByType(userid,roleId,mysqlite,net,videojur,this);
    connect(searchbytype,SIGNAL(goback()),this,SLOT(goBackMainForm()));

    ui->stackedWidget->addWidget(searchbytype);
    ui->stackedWidget->setCurrentWidget(searchbytype);
}
void MainWindow::goBackMainForm(){
     timeLabel->show();
    dateTimeLabel->show();
    ui->stackedWidget->setCurrentIndex(firstIndex);

    for(int i = ui->stackedWidget->count(); i >= 2; i--)

    {
        qDebug()<<"form i"<<i;
        QWidget* widget = ui->stackedWidget->widget(i-1);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();

    }
}

void MainWindow::goBackSearchForm(){

    ui->stackedWidget->setCurrentIndex(1);

    for(int i = ui->stackedWidget->count(); i >= 3; i--)

    {
        qDebug()<<"form i"<<i;
        QWidget* widget = ui->stackedWidget->widget(i-1);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();

    }
}


void MainWindow::recDsikMessage(int stat)
{
    if (stat == 1) {
        lastDiskWarningStat = 1;
        lastDiskWarningTime = QDateTime();
        if (diskWarningBanner) {
            diskWarningBanner->close();
        }
        return;
    }

    // 参考竖屏和 800x1280 版本：提示不抢占操作，同类告警十分钟内不重复弹出。
    if (diskWarningBanner) {
        return;
    }
    const QDateTime now = QDateTime::currentDateTime();
    const bool isSameWarning = stat == lastDiskWarningStat;
    if (isSameWarning && lastDiskWarningTime.isValid()
            && lastDiskWarningTime.secsTo(now) < 600) {
        return;
    }
    lastDiskWarningStat = stat;
    lastDiskWarningTime = now;

    const QString diskPath = configuredStoragePath();
    const QString message = stat == -1
            ? QString("存储盘不可用：%1，请检查挂载；软件继续运行。").arg(diskPath)
            : QString("存储盘空间不足：%1 剩余少于 10GB；软件继续运行。").arg(diskPath);

    QFrame *banner = new QFrame(this);
    banner->setObjectName("diskWarningBanner");
    banner->setAttribute(Qt::WA_DeleteOnClose);
    banner->setStyleSheet(
                "QFrame#diskWarningBanner { background: rgba(28, 75, 101, 235);"
                " border: 1px solid #44c7e8; border-radius: 6px; }"
                "QLabel { color: white; font: 12pt 'Sans Serif'; }"
                "QPushButton { color: white; background: #168bc0; border: 1px solid #62d8f5;"
                " border-radius: 4px; padding: 4px 14px; }"
                "QPushButton:pressed { background: #0c6290; }");
    QHBoxLayout *layout = new QHBoxLayout(banner);
    layout->setContentsMargins(14, 8, 10, 8);
    QLabel *messageLabel = new QLabel(message, banner);
    messageLabel->setWordWrap(true);
    QPushButton *closeButton = new QPushButton("知道了", banner);
    closeButton->setFixedHeight(32);
    layout->addWidget(messageLabel, 1);
    layout->addWidget(closeButton);
    banner->setGeometry(qMax(12, width() - 560), 68, 548, 58);
    connect(closeButton, &QPushButton::clicked, banner, &QWidget::close);
    connect(banner, &QObject::destroyed, this,
            [this](){ diskWarningBanner = nullptr; });
    diskWarningBanner = banner;
    banner->show();
    banner->raise();

}



void MainWindow::recStopDiskStat()
{
    // 磁盘告警只做提示，不终止后台磁盘监控。
}


void MainWindow::goBackMainFormaddwidget(){

    ui->stackedWidget->setCurrentIndex(firstIndex);
    form->setlistWidget();

    for(int i = ui->stackedWidget->count(); i >= 2; i--)

    {
        qDebug()<<"form i"<<i;
        QWidget* widget = ui->stackedWidget->widget(i-1);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();

    }


}


void MainWindow::openbroswer()
{
    QString url =  Config::getInstance()->Get("server","serverManagerIp").toString();
    QString writetest = "chromium --chrome-frame " + url+ " --no-sandbox";
    m_proces_bash->write(writetest.toLocal8Bit() + '\n');
}
void MainWindow::goBackSearch(int search_type)
{

    ui->stackedWidget->setCurrentIndex(search_type);
    for(int i = ui->stackedWidget->count(); i >= 4; i--)

    {
        qDebug()<<"form i"<<i;
        QWidget* widget = ui->stackedWidget->widget(i-1);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();

    }
}


void MainWindow::checkHubConnect(){


    ADSDeviceslist = getusb->getADSDevices();
    //ADSDeviceslist.at(0).path


    if(!ADSDeviceslist.isEmpty()){

        ZFYConnectNumber.clear();
        //监测port口，将对应位置的连接状态设置为true,拔出后需要重新设置为false
        for(int i=0;i<getusb->getADSDevices().count();i++){
            qDebug()<<getusb->getADSDevices().at(i).path;

            ZFYConnectNumber.append(usbhublist.indexOf(getusb->getADSDevices().at(i).path));

        }

        emit sendConnectPortNumber(ZFYConnectNumber);

    }

}

void MainWindow::initActiveMq()
{

    connect(&myconsumer,SIGNAL(comsumerreceiver(QString)),this,SLOT(getConsumerMessage(QString)));
    connect(&myproducer,SIGNAL(producer_connectfail()),this,SLOT(toexitApp()));
    activemq::library::ActiveMQCPP::initializeLibrary();
    std::string brokerURI;
    std::string destURI;
    bool useTopics;
    bool clientAck;
    //brokerURI ="tcp://127.0.0.1:61616?transport.useAsyncSend=true&maxReconnectDelay=10000";
    brokerURI ="tcp://127.0.0.1:61616";
    useTopics = false;
    unsigned int numMessages = 99999999;
    clientAck = false;

    std::string consumerdestURI =  Config::getInstance()->Get("server","ServerQueue").toString().toStdString();
//    qDebug()<<"consumerdestURI"<<QString::fromStdString(consumerdestURI);
    std::string consumerbrokerURI =  Config::getInstance()->Get("server","serverMqIp").toString().toStdString();
//    qDebug()<<"consumerbrokerURI"<<QString::fromStdString(consumerbrokerURI);
    destURI = "ServerQueue";

    bool connectresult = myproducer.start(brokerURI, numMessages, destURI, useTopics ,clientAck);
//    qDebug()<<"myproducer_connectresult"<<connectresult;
    bool myconsumerresult = myconsumer.start(consumerbrokerURI, consumerdestURI, useTopics, clientAck);
//    qDebug()<<"myconsumer_connectresult"<<myconsumerresult;


}

void MainWindow::doinsertActiveMq(QVariant dataVar)
{

    // qDebug()<<"insert copyfile ActiveMq start!!";
    FileAllInfo file_info = dataVar.value<FileAllInfo>();
    QVariantMap map;
    //QVariantMap mapOut;
    QVariantList varlist;
    QVariantMap map2;
    map["ClientCode"] = MyHostIPV4Address;
    map["OperaType"] = 1;

    map2["TaskId"] = file_info.file_uuid;
    map2["FileName"] = file_info.file_name;
    map2["CreateTime"] = file_info.create_time;
    map2["UploadTime"] = file_info.upload_time;
    map2["CaseNo"] = file_info.file_caseNo;
    map2["FileLength"] = file_info.file_size;
    map2["IsImportant"] = file_info.is_important;
    map2["PolNo"] = file_info.user_id;
    map2["CheckCode"] = file_info.file_uuid;
    map2["SaveMaxDay"] = file_info.save_max_day;
    map2["DevNo"] = file_info.driver_id;
    map2["FileType"] = file_info.file_type;
    map2["FileShowPath"] = file_info.fileshowpath;
    map2["Coordinates"] = file_info.file_gps;
    varlist.append(map2);
    map["OperaModels"] = varlist;
    map["Sendtime"] = QDateTime::currentDateTime();

    //mapOut["AddDocInfo"] = map;
    QString data = QJsonDocument::fromVariant(map).toJson(QJsonDocument::Indented);//带有格式
//    myproducer.sendTxtMsg("Client2Server:"+data.toStdString());
    map.clear();
    map2.clear();
    varlist.clear();


}

void MainWindow::doMqinsertLog(int type,QVariantMap map){


    // qDebug()<<"insert ActiveMq Log start!!";
    QVariantMap maptop;
    maptop["ClientCode"] = MyHostIPV4Address;
    maptop["OperaType"] = type;
    maptop["Sendtime"] = QDateTime::currentDateTime();
    QVariantList varlist;
    varlist.append(map);
    maptop["OperaModels"] = varlist;
    QString data = QJsonDocument::fromVariant(maptop).toJson(QJsonDocument::Indented);//带有格式
    myproducer.sendTxtMsg("Client2Server:"+data.toStdString());
    maptop.clear();
    map.clear();
    varlist.clear();

}
void MainWindow::getConsumerMessage(QString str)
{

    // qDebug()<<"message str"<<str;
    str = str.mid(str.indexOf(":")+1);
    qDebug()<<"message str"<<str;
    //QVariantMap map ;
    //str.to
    //map.value()
    //Client2Server[""]
    QJsonParseError err_rpt;

    QJsonDocument  root_Doc = QJsonDocument::fromJson(str.toLocal8Bit(), &err_rpt);//字符串格式化为JSON
    if(err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON格式错误";
        //return -1;
    }else{
        //        qDebug() << "JSON格式正确：\n" << root_Doc;
        QJsonObject root_Obj = root_Doc.object();
        int result_OperaType = root_Obj.value("OperaType").toInt();
        QString result_Datetime = root_Obj.value("Datetime").toString();
        QJsonValue result_OperaModels = root_Obj.value("OperaModels");
        if(result_OperaType  == 1){
            //update file
            if(result_OperaModels.isArray())
            {
                QVariantMap map;
                QJsonObject result_Obj = result_OperaModels.toArray().at(0).toObject();

                QString result_CheckCode = result_Obj.value("CheckCode").toString();
                qDebug() << result_CheckCode;

                QString result_CreateTime = result_Obj.value("CreateTime").toString();
                qDebug() << result_CreateTime;

                QString result_DevNo = result_Obj.value("DevNo").toString();
                qDebug() << result_DevNo;

                QString result_TaskId = result_Obj.value("TaskId").toString();
                qDebug() << result_TaskId;


                map["CheckCode"] = result_CheckCode;
                map["CreateTime"] = result_CreateTime;
                map["DevNo"] = result_DevNo;
                map["TaskId"] = result_TaskId;

                emit toupdateSql(map);
            }
        }
        if(result_OperaType == 2){
            //delete file
            if(result_OperaModels.isArray())
            {
                QVariantMap map;
                QJsonObject result_Obj = result_OperaModels.toArray().at(0).toObject();
                QString result_CheckCode = result_Obj.value("CheckCode").toString();
                qDebug() << result_CheckCode;

                QString result_TaskId = result_Obj.value("TaskId").toString();
                qDebug() << result_TaskId;

                map["CheckCode"] = result_CheckCode;
                map["TaskId"] = result_TaskId;


                emit todeleteSql(map);
            }

        }


    }

}

void MainWindow::toexitApp()
{

    qDebug()<<"connect local activemq fail,to exit!!!";

    QMessageBox::StandardButton rb = QMessageBox::warning(NULL, tr("Warning"), tr("Can not Connect the local ActiveMq!click button to exit"), QMessageBox::Ok );
    if(rb == QMessageBox::Ok)
    {
        exit(0);
    }
}


QHostAddress MainWindow::getHostIPV4Address()
{

    //获取第一个非本地回环的IPv4地址，如果找不到符合条件的地址，则返回本地回环地址
    foreach(const QHostAddress& hostAddress,QNetworkInterface::allAddresses())
        if ( hostAddress != QHostAddress::LocalHost && hostAddress.toIPv4Address() )
            return hostAddress;

    return QHostAddress::LocalHost;
}
//通过QProcess类与外部进程bash进行交互，执行特定的命令，并处理其输出和错误信息。在这里，命令who -b用于获取系统的最后一次启动时间。
void MainWindow::initProcess()
{

    m_proces_bash = new QProcess(this);
    m_proces_bash->start("bash");
    m_proces_bash->waitForStarted();
    qDebug()<<"initProcess";
    connect(m_proces_bash,SIGNAL(readyReadStandardOutput()),this,SLOT(readBashStandardOutputInfo()));
    connect(m_proces_bash,SIGNAL(readyReadStandardError()),this,SLOT(readBashStandardErrorInfo()));

    //m_proces_bash->close()

    QString text_start = "who -b";
    //QString text_start = "last -x|grep shutdown | head -1";
    m_proces_bash->write(text_start.toLocal8Bit() + '\n');
}


void MainWindow::readBashStandardOutputInfo()
{


    //qDebug()<<"readBashStandardOutputInfo";
    QByteArray cmdout = m_proces_bash->readAllStandardOutput();
    if(!cmdout.isEmpty()){
        if(cmdout.contains("system boot")){

        }
        //ui->textEdit_bashmsg->append(QString::fromLocal8Bit(cmdout));
        qDebug()<<"readBashStandardOutputInfo"<<QString::fromLocal8Bit(cmdout);

        QRegExp reg1("([0-9]{4}-[0-9]{2}-[0-9]{2}\\s[0-9]{2}:[0-9]{2})");
        reg1.indexIn(QString::fromLocal8Bit(cmdout));
        qDebug()<<"reg1.cap"<<reg1.cap(1);

        if(reg1.cap(1).length() >0){
            qDebug()<<"reg1.cap(1).length()"<<reg1.cap(1).length();
            QString strBuffer = reg1.cap(1) + ":00";
            systemstarttime = QDateTime::fromString(strBuffer, "yyyy-MM-dd hh:mm:ss");


            qDebug()<<"systemstarttime"<<systemstarttime;

            QString struuid = QUuid::createUuid().toString().remove("{").remove("}");
            QVariantMap map;
            map["TaskId"] = struuid;
            map["OperaType"] = 1;
            map["OperaContent"] = "system start time";
            map["Time"] = systemstarttime;

            emit toinsert_AcStation_log(map);

            //to get shutdown time
            QString text_shutdown = "last -x|grep shutdown | head -1";
            m_proces_bash->write(text_shutdown.toLocal8Bit() + '\n');
        }else{
            QRegExp myshutdown("(Mon|Tue|Wed|Thu|Fri|Sat|Sun)\\s(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\s{1,2}(\\d{1,2})\\s(\\d{2}):(\\d{2})");
            myshutdown.indexIn(QString::fromLocal8Bit(cmdout));
            qDebug()<<"myex.capturedTexts()"<<myshutdown.capturedTexts();
            qDebug()<<"myex.cap(0)"<<myshutdown.cap(0);
            QString date1 = QString("%1 %2 %3 %4:%5")
                    .arg(myshutdown.cap(1)).arg(myshutdown.cap(2))
                    .arg(myshutdown.cap(3).toInt(),2,10,QLatin1Char('0'))
                    .arg(myshutdown.cap(4)).arg(myshutdown.cap(5));

            //QString date1 = myshutdown.cap(0);

            QString year = QDate::currentDate().toString("yyyy");
            QString datelast = date1 + ":00 "+year;

            QLocale locale = QLocale::English;
            QDateTime myshutdowntime = locale.toDateTime(datelast,"ddd MMM dd hh:mm:ss yyyy");
            qDebug()<<"myshutdowntime"<<myshutdowntime;

            QString struuid = QUuid::createUuid().toString().remove("{").remove("}");
            QVariantMap map;
            map["TaskId"] = struuid;
            map["OperaType"] = 2;
            map["OperaContent"] = "system shutdown time";
            map["Time"] = myshutdowntime;

            emit toinsert_AcStation_log(map);



        }


    }
}

void MainWindow::readBashStandardErrorInfo()
{


    //qDebug()<<"readBashStandardErrorInfo";
    QByteArray cmdout = m_proces_bash->readAllStandardError();
    if(!cmdout.isEmpty()){
        //ui->textEdit_bashmsg->append(QString::fromLocal8Bit(cmdout));
        qDebug()<<"readBashStandardErrorInfo"<<QString::fromLocal8Bit(cmdout);

    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"MainWindow close event";

    exit(0);
    event->ignore();
}


void MainWindow::tosendConfirmtoSerVer(int status, QString taskid)
{

    // qDebug()<<"send confirm taskid back to server ActiveMq !!";
    QVariantMap map;

    map["Receipt"] = taskid;
    map["Status"] = status;

    QString data = QJsonDocument::fromVariant(map).toJson(QJsonDocument::Indented);//带有格式
//    myproducer.sendTxtMsg("ConfirmMsg:"+data.toStdString());
    map.clear();
}

void MainWindow::uploaddisksize()
{
    storage.setPath(configuredStoragePath());
    storage.refresh();
    qint64 totalBytes = 0;
    qint64 usedBytes = 0;
    qint64 availableBytes = 0;
    if (readFilesystemUsage(configuredStoragePath(), totalBytes, usedBytes, availableBytes)) {
        freedisksize = availableBytes / 1024.0 / 1024.0 / 1024.0;
    } else {
        freedisksize = storage.isValid() && storage.isReady()
                ? storage.bytesAvailable() / 1024.0 / 1024.0 / 1024.0 : 0.0;
    }

    QVariantMap map;
    QString struuid = QUuid::createUuid().toString().remove("{").remove("}");
    map["TaskId"] = struuid;
    map["OperaType"] = 1;
    map["DiskFreeSize"] = freedisksize;
    // 更新可用磁盘容量
//    doMqinsertLog(4,map);
}

void MainWindow::getnetworkanddiskmsg()
{
    storage.setPath(configuredStoragePath());
    storage.refresh();
    qint64 totalBytes = 0;
    qint64 usedBytes = 0;
    qint64 availableBytes = 0;
    if (!readFilesystemUsage(configuredStoragePath(), totalBytes, usedBytes, availableBytes)) {
        totalBytes = storage.isValid() && storage.isReady() ? storage.bytesTotal() : 0;
        availableBytes = storage.isValid() && storage.isReady() ? storage.bytesAvailable() : 0;
        usedBytes = storage.isValid() && storage.isReady()
                ? qMax<qint64>(0, totalBytes - storage.bytesFree()) : 0;
    }
    freedisksize = availableBytes / 1024.0 / 1024.0 / 1024.0;
    totaldisksize = totalBytes / 1024.0 / 1024.0 / 1024.0;
    const double totalSize = totaldisksize;
    const double useSize = usedBytes / 1024.0 / 1024.0 / 1024.0;
    const double usePer = totalBytes > 0 ? usedBytes * 100.0 / totalBytes : 0.0;


   emit sendHeartbeatTONetWork(freedisksize,totaldisksize,pingip);

    bool pingresult = pingOk(pingip);
    if(pingresult){
//        emit sendHeartbeatTONetWork(freedisksize,totaldisksize,pingip);
//        QApplication::beep();
    }else{
//        QApplication::beep();
//        player->playAudio("./mp3/error.mp3", "网络已断开。", true);
//        player->playAudio("./mp3/error.mp3",  "网络已断开");
    }

    emit updateDiskSize(usePer,pingresult,totalSize,useSize);
}
bool MainWindow::pingOk(QString sIp)
{
    //判断输入IP是否为空
    if (sIp.isEmpty()) return false;

    //构建ping命令
    QString sCmd = QString("ping -s 1 -c 1 %1").arg(sIp);
    QProcess proc;
    proc.start(sCmd);
    proc.waitForReadyRead(500);
    proc.waitForFinished(500);

    //读取ping命令返回的所有信息
    QString sRet = proc.readAll();

    //如果网络连通,ping命令返回的信息,包含 "TTL"
    bool bPing = (sRet.indexOf("TTL", 0, Qt::CaseInsensitive) >= 0);
//    qDebug() << sCmd << sRet << bPing;
    proc.finished(0);
    proc.terminate();

    return bPing;
}




QString MainWindow::getMediaMountPoints(QString mymountPoint) {


    // 首先检查传入的挂载点是否有超过100GB的剩余空间
    QString freeSpace = getRemainingSpace(mymountPoint);
    if (!freeSpace.isEmpty()) {
        bool ok;
        double spaceValue = 0.0;

        // 判断剩余空间的单位并进行转换
        if (freeSpace.endsWith("G")) {
            spaceValue = freeSpace.left(freeSpace.length() - 1).toDouble(&ok); //
        } else if (freeSpace.endsWith("T")) {
            spaceValue = freeSpace.left(freeSpace.length() - 1).toDouble(&ok) * 1024; // 转换为GB
        }

        if (ok && spaceValue > 100) {
            qDebug() << "剩余空间" << mymountPoint<< "大于100GB:"<<spaceValue;
            return mymountPoint; // 如果剩余空间大于100GB，直接返回
        }
    }

    // 创建 QProcess 对象
    QProcess process;

    // 设置要执行的命令
    process.start("/bin/bash", QStringList() << "-c" << "/bin/lsblk -o NAME,TYPE,SIZE,VENDOR,MODEL,MOUNTPOINT | grep -i part | grep '/media'");


    // 等待命令执行完成
    process.waitForFinished();

    // 获取命令输出
    QString output = process.readAllStandardOutput();
    //qDebug() << "Command Output:" << output; // 调试输出

    // 按行分割输出
    QStringList lines = output.split('\n', QString::SkipEmptyParts);

    // 提取挂载点
    for (const QString &line : lines) {
        QStringList columns = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
       // qDebug() << "Columns:" << columns; // 调试输出
        if (columns.size() == 4) { // 确保列的数量为4
            QString sizeStr = columns.at(2).trimmed(); // 获取大小列
            QString mountPoint = columns.last().trimmed(); // 获取挂载点

            // 判断大小是否大于100G
            if (sizeStr.endsWith("G") || sizeStr.endsWith("T")) {
                bool ok;
                double sizeValue = 0.0;

                // 判断大小的单位并进行转换
                if (sizeStr.endsWith("G")) {
                    sizeValue = sizeStr.left(sizeStr.length() - 1).toDouble(&ok); //
                } else if (sizeStr.endsWith("T")) {
                    sizeValue = sizeStr.left(sizeStr.length() - 1).toDouble(&ok) * 1024; // 转换为GB
                }

                if (ok && sizeValue > 100) {
                    // 打印磁盘剩余容量
                    QString remainingSpace = getRemainingSpace(mountPoint);
                    qDebug() << "Free space on" << mountPoint << ":" << remainingSpace;

                    return mountPoint; // 返回第一个符合条件的挂载点
                }
            }
        }
    }
    // 磁盘异常统一由 recDsikMessage() 在主线程显示非阻塞提示。
    // 此处禁止创建提示音线程，避免音频文件缺失及悬空引用造成崩溃。
    qWarning() << "磁盘容量不足，无法使用任何挂载点。";
    return QString(); // 返回空字符串
}



QString MainWindow::getRemainingSpace(const QString &mountPoint) {

    QProcess dfProcess;
    dfProcess.start("/bin/bash", QStringList() << "-c" << QString("/bin/df -h %1 | awk 'NR==2 {print $4}'").arg(mountPoint));
    dfProcess.waitForFinished();

    QString output = dfProcess.readAllStandardOutput().trimmed(); //
    return output; //
}




QString MainWindow::getDiskNodeSpace(const QString &mountPoint,int node) {


    QProcess dfProcess;
    QString _strArg1 = QString("/bin/df -h %1 | awk 'NR==2 {print $").arg(mountPoint);
    QString _strArg2 = QString("%1}'").arg(node);
    QString _strArg = _strArg1 +_strArg2;

    dfProcess.start("/bin/bash", QStringList() << "-c" << _strArg);
    dfProcess.waitForFinished();

    QString output = dfProcess.readAllStandardOutput().trimmed(); //
    return output; // 返回容量
}



double MainWindow::sumDiskSize(QString freeSpace)
{


    bool ok;
    double spaceValue = 0.0;

    // 判断剩余空间的单位并进行转换
    if (freeSpace.endsWith("G")) {
        spaceValue = freeSpace.left(freeSpace.length() - 1).toDouble(&ok); //
    } else if (freeSpace.endsWith("T")) {
        spaceValue = freeSpace.left(freeSpace.length() - 1).toDouble(&ok) * 1024; // 转换为GB

    }

    return  spaceValue;

}





QString MainWindow::configuredStoragePath() const
{
    const QString dataPath = Config::getInstance()->Get("wsConfig", "dataPath")
            .toString().trimmed();
    const QString saveDir = Config::getInstance()->Get("wsConfig", "saveDir")
            .toString().trimmed();

    if (!dataPath.isEmpty() && QFileInfo(dataPath).exists()) {
        return QDir::cleanPath(dataPath);
    }
    if (!saveDir.isEmpty() && QFileInfo(saveDir).exists()) {
        return QDir::cleanPath(saveDir);
    }
    if (!dataPath.isEmpty()) {
        return QDir::cleanPath(dataPath);
    }
    if (!saveDir.isEmpty()) {
        return QDir::cleanPath(saveDir);
    }
    return "/data";
}

int MainWindow::offlineDropDisk()
{
    const QString savePath = configuredStoragePath();
    const QFileInfo pathInfo(savePath);
    if (!pathInfo.exists()) {
        qWarning() << "configured collection path is unavailable:" << savePath;
        return -1;
    }

    QStorageInfo saveStorage(pathInfo.absoluteFilePath());
    saveStorage.refresh();
    const qint64 lowSpaceLimitBytes = 10LL * 1024 * 1024 * 1024;
    qint64 totalBytes = 0;
    qint64 usedBytes = 0;
    qint64 availableBytes = 0;
    const bool hasDfUsage =
            readFilesystemUsage(savePath, totalBytes, usedBytes, availableBytes);
    const int result = hasDfUsage
            ? (availableBytes < lowSpaceLimitBytes ? 0 : 1)
            : ((!saveStorage.isValid() || !saveStorage.isReady())
               ? -1 : (saveStorage.bytesAvailable() < lowSpaceLimitBytes ? 0 : 1));
    qDebug() << "disk check" << savePath
             << "volume=" << saveStorage.rootPath()
             << "available=" << (hasDfUsage ? availableBytes : saveStorage.bytesAvailable())
             << "threshold=" << lowSpaceLimitBytes
             << "result=" << result;
    return result;

#if 0

    QStringList mountPoints;
    // 创建 QProcess 对象
    QProcess process;
    // 设置要执行的命令
    process.start("/bin/bash", QStringList() << "-c" << "/bin/lsblk -o NAME,TYPE,SIZE,VENDOR,MODEL,MOUNTPOINT | grep -i part | grep '/media'");


    process.waitForFinished();

    // 获取命令输出
    QString output = process.readAllStandardOutput();
    //qDebug() << "Command Output:" << output; // 调试输出

    // 按行分割输出
    QStringList lines = output.split('\n', QString::SkipEmptyParts);

    // 提取挂载点
    for (const QString &line : lines) {
        QStringList columns = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        //qDebug() << "Columns:" << columns; // 调试输出
        if (columns.size() == 4) { // 确保列的数量为4
            QString mountPoint = columns.last().trimmed(); // 去除两端空白字符和换行符
            mountPoints.append(mountPoint);
        }
    }

    int _resStat = 1;
    int _limit = 10;
    int _minSizes = 1000;

    QString _saveDir =  Config::getInstance()->Get("wsConfig","saveDir").toString();
    bool _isInDisks = mountPoints.contains(_saveDir);

    if(!_isInDisks){
        _resStat = -1;
    }else{

        QString _currDsik =  getDiskNodeSpace(_saveDir, 4);
        double _currTotalSize = sumDiskSize(_currDsik);
        if( _currTotalSize < _limit){
             _resStat = 0;
        }
    }

    if(_resStat != 1){

        for (const QString &mountPoint : mountPoints) {

           //break;
           QString _tmpTotalDsik =  getDiskNodeSpace(mountPoint, 2);
           double _totalSize = sumDiskSize(_tmpTotalDsik);

           if(_totalSize > _minSizes){
                QString _tmpModDsik =  getDiskNodeSpace(mountPoint, 4);
                double _modSize = sumDiskSize(_tmpModDsik);

                if(_modSize > _limit){

                    hsGlobalMtx.lock();

                     Config::getInstance()->Set("wsConfig","saveDir",mountPoint);
                    QString _saveFilePath =  Config::getInstance()->Get("wsConfig","saveFilePath").toString();
                    QString _dataPath = mountPoint + "/" + _saveFilePath;
                     Config::getInstance()->Set("wsConfig","dataPath",_dataPath);

                   Config::getInstance()->Sync();

                    hsGlobalMtx.unlock();

                    _resStat = 1;
                    break;
                }

           }

        }

    }


    #ifdef USE_DATA_DISK

        _saveDir =  Config::getInstance()->Get("wsConfig","saveDir").toString();
        QString _dataDsik =  getDiskNodeSpace(_saveDir, 4);
        //_dataDsik = "1.7T";

        double _dataTotalSize = sumDiskSize(_dataDsik);
        if( _dataTotalSize < _limit){
             _resStat = 0;
        }else{

             _resStat = 1;
        }

    #endif






#ifdef USE_EXTERN_DISK


    if(mountPoints.isEmpty()){

        mountPoints.append("/data");
        //emit systemLoseDisk();

         return -1;
    }

#endif

    qDebug() << "Mount Points in /media:";
    return _resStat;
#endif

}




void MainWindow::watchDisk()
{

    while(true){
        if(is_mainwindow_exited){
            break;
        }

        int _rId = offlineDropDisk();
        emit signalDsikMessage(_rId);

        usleep(20000 * 1000);
    }

}


//void MainWindow::pushButtonOpenOneKey(){

//    if(m_serialPort->isOpen()){
//        std::string data = utils->openOneKeyCommand(0,);

//        QString sendData = QString::fromStdString(data);

//        QByteArray ba;
//        ba = QByteArray::fromHex(sendData.toLatin1());
//        m_serialPort->write(ba);
//    }

//}
