#include "mainform.h"
#include "ui_mainform.h"
#include "uiprofile.h"
#include <QColor>
#include "networkutility.h"
#include "systemmanagewindow.h"
#include <sstream>
#include <QProcess>
#include <QCoreApplication>
#include <QSettings>
#include <QCryptographicHash>
#include <QDateTime>
#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include "wspairdevicethread.h"
#include "searchbytype.h"
#include <QDateTime>


std::map<int, std::string> hsBusMap;
std::mutex hsBusMtx; 
 
bool is_standard_sign = true;

QList<NewUserInfo> global_userinfo_all;


//#define VERSION_NAME "20.24.9.5"
#define MAJOR_VERSION 20 // Fixed major version
MainForm::MainForm(const QVector<DASBuddy *> &buddy, MySqlLite *sqltie,
                   NetworkUtility *net, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainForm)
{
    ui->setupUi(this);
    UiProfile::apply(this, "mainform");


    isstandard = true;
    is_standard_sign = isstandard;

    const int configuredPortCount = Config::getInstance()->Get("portInfo", "port_num").toInt();
    portNum = qBound(1, configuredPortCount, kMaxWorkstationPorts);
    if (portNum != configuredPortCount) {
        qWarning() << "Configured port count is outside the supported range:" << configuredPortCount;
    }
    dasbuddy = buddy;
    runMode = 0;

    initLOGO();


    this->isPairDevice = false;
    ui->pushButton_PersonCenter->setVisible(false);
    ui->pushButton_PersonCenter->setEnabled(false);

    ui->btn_signin->setVisible(false);
    ui->btn_update->setVisible(false);

    loadPairDeviceConfig();
    loadPortConfig();
    initPairDeviceThread();


    initAllWindowProgress(portNum+1);

    // Set number of items per column
    int itemsPerColumn =  Config::getInstance()->Get("wsConfig", "portRowCount").toInt();

    // Calculate number of columns needed
     int columnCount = (portNum + itemsPerColumn - 1) / itemsPerColumn;

    const bool compact1280Layout = UiProfile::isCompact1280();
    if (compact1280Layout) {
        // mainform.ui 中版本信息和存储信息的容器历史上使用了相同的
        // objectName，1280 配置只能命中其中一个，导致存储信息仍停留在
        // 1920 坐标并落到屏幕之外。这里通过标签的实际父容器分别定位。
        QWidget *versionInfoWidget = ui->versionlabel->parentWidget();
        QWidget *storageInfoWidget = ui->label_disksize->parentWidget();
        if (versionInfoWidget) {
            versionInfoWidget->setGeometry(710, 980, 171, 31);
        }
        if (storageInfoWidget) {
            storageInfoWidget->setGeometry(890, 980, 390, 32);
            storageInfoWidget->show();
            storageInfoWidget->raise();
        }

        // Keep the original 1280x1024 main-screen list behavior.
        ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // Calculate item dimensions
    int listWidgetWidth = ui->listWidget->width() - (compact1280Layout ? 2 : 60);
    int listWidgetHeight = ui->listWidget->height() - 2;

    int itemWidth = listWidgetWidth / itemsPerColumn;
    int itemHeight = listWidgetHeight / columnCount;

    if(columnCount > 5){

        itemHeight = listWidgetHeight / 5;
    }

    for (int i=0;i<portNum;i++) {
        //dasbuddy[i] = new DASBuddy(this);
        //dasbuddy[i]->setDasbuddyNumber(i);

        QString text = QString("%1").arg(i + 1, 2, 10, QChar('0'));

//        dasbuddy[i]->setPhotoText(QString::number(i+1));
        dasbuddy[i]->setPhotoText(text);
        if (compact1280Layout) {
            // Match the card size used by the original 1280x1024 version.
            dasbuddy[i]->setFixedSize(QSize(itemWidth - 7, itemHeight - 7));
        }
        QListWidgetItem *newItem = new QListWidgetItem();

        newItem->setSizeHint(QSize(itemWidth, itemHeight));

        ui->listWidget->insertItem(i,newItem);
        ui->listWidget->setItemWidget(newItem, dasbuddy[i]);
        connect(dasbuddy[i],SIGNAL(setPriority(int,bool)),this,SLOT(setPriorityCollect(int,bool)),Qt::QueuedConnection);
    }

    thread1 = new HotPlugThread(dasbuddy.data());
    //thread1->start();

    threadCKDisk = new CheckHsDiskThread(this);

    QTimer * _delayWork = new QTimer(this);
    _delayWork->setSingleShot(true);

    connect(_delayWork,&QTimer::timeout,this,[=](){

        thread1->start();
        threadCKDisk->start();
    });
    _delayWork->start(4000);


    QTimer * _releaseWork = new QTimer(this);
    connect(_releaseWork,&QTimer::timeout,this,[=](){

        releaseSysCaches();

    });


    connect(thread1,SIGNAL(insertZFY()),this,SLOT(toinsertZFYDialog()));

//    connect(ui->btn_exit,&QPushButton::clicked,[=](){
//        exit(0);
//    });

//    connect(ui->pushButton_PersonCenter,SIGNAL(clicked()),this,SLOT(gotoPersonCenter()));



    mysql = sqltie;
    mynet = net;
    naManager = new QNetworkAccessManager(this);
    //处理网络请求的回复
    connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)),Qt::QueuedConnection);

    connect(this,&MainForm::mainToNetSelect,mynet,&NetworkUtility::netTOmain);
    connect(mynet,&NetworkUtility::netdownloadStatustoMain,this,&MainForm::setbtnUpdateText);
    connect(this,&MainForm::insertt_logToMSQ,mysql,&MySqlLite::insertt_logformainF);

//    utils = new lockerutils();
    //实例化一个串口对象
//    m_serialPort = new QSerialPort();

    QVariant runM =  Config::getInstance()->Get("runConfig", "runMode");

    bool isOk = false;
    int runModeTmp = runM.toInt(&isOk);
    if(isOk){
        runMode = runModeTmp;
    }
    if(runMode == 2){
        ui->label_6->setText("服务器");
    }else{
        ui->label_6->setText("单机");
    }
    // Get the current date
    QDate currentDate = QDate::currentDate();

    // Extract the year, month, and day
    int year = currentDate.year() % 100; // Last two digits of the year
    int month = currentDate.month();      // Current month
    int day = currentDate.day();          // Current day

    // Format version string
    QString version = QString("%1.%2.%3.%4")
                        .arg(MAJOR_VERSION)
                        .arg(year, 2, 10, QChar('0'))  // Ensure two digits for year
                        .arg(month, 2, 10, QChar('0')) // Ensure two digits for month
                        .arg(day, 2, 10, QChar('0'));   // Ensure two digits for day



    QString currversion =   Config::getInstance()->Get("wsConfig", "currversion").toString();
    if(currversion.isEmpty()){
         ui->versiontextlabel->setText(version);
    }else{
        ui->versiontextlabel->setText(currversion);
    }


#ifdef LOCK_LAMP
//    openPort();
    if (!lampThread) {
        lampThread = new LampThread(this);
        lampThread->start(); // 启动线程

        lampThread->addTask([this]() { lampThread->turnOnLamp(8); }, 0); // 1.5秒后打开灯
    }

#endif
}

MainForm::~MainForm()
{


        //    if(m_serialPort->isOpen()){
        //        m_serialPort->close();
        //    }
            if (lampThread) {

                lampThread->quit(); // 请求线程退出
                lampThread->wait(); // 等待线程结束
                delete lampThread; // 释放资源
            }

            if(threadCKDisk  != nullptr){
                delete  threadCKDisk;
                threadCKDisk  = nullptr;

            }

            if(thread1  != nullptr){
                delete  thread1;
                thread1  = nullptr;

            }




    delete ui;


}


void MainForm::setlistWidget()
{
    for (int i=0;i<30;i++) {
        QListWidgetItem *newItem = new QListWidgetItem();

        ui->listWidget->insertItem(i,newItem);
        ui->listWidget->setItemWidget(newItem, dasbuddy[i]);


    }
    for (int j=30;j<60;j++) {
        ui->listWidget->takeItem(30);

    }

}

void MainForm::on_pushButton_searchfiles_clicked()
{

    showLoginForm(LoginForm::DataQuery);

    //
//    loginForm = new LoginForm(0);
//    //loginForm->setWindowFlag(Qt::)
//    loginForm->setWindowModality(Qt::ApplicationModal);
//    loginForm->show();
//    connect(loginForm,&LoginForm::sigLogSelectUserNo,mysql,&MySqlLite::sloMQSelectUserNo);
//    connect(mysql,&MySqlLite::MQSelectUserNoSuc,loginForm,&LoginForm::LogSelectUserNoSuc);
//    connect(loginForm,SIGNAL(loginsuccessRoleId(int ,const QString &,const QString &)),this,SLOT(checkjurstr(int ,const QString &,const QString &)));
}
void MainForm::on_pushButton_setup_clicked()
{

    showLoginForm(LoginForm::SystemSettings);
//    loginForm = new LoginForm(1);
//    //loginForm->setWindowFlag(Qt::)
//    loginForm->setWindowModality(Qt::ApplicationModal);
//    loginForm->show();
//    connect(loginForm,&LoginForm::sigLogSelectUserNo,mysql,&MySqlLite::sloMQSelectUserNo);
//    connect(mysql,&MySqlLite::MQSelectUserNoSuc,loginForm,&LoginForm::LogSelectUserNoSuc);
//    connect(loginForm,SIGNAL(loginsuccessRoleId(int ,const QString &,const QString &)),this,SLOT(checkjurstr(int ,const QString &,const QString &)));

}




void MainForm::showLoginForm(int type) {
//qDebug()<<"MainForm howLoginForm"<<type;
    if (loginForm) {
        loginForm->deleteLater();
        loginForm = nullptr;
    }


    loginForm = LoginFormFactory::createLoginForm(type,mysql);
    loginForm->setWindowModality(Qt::ApplicationModal);
    loginForm->show();
//    turnOffLamp(BlueLampChannel);

    // 连接信号和槽
    connect(loginForm, &LoginForm::sigLogSelectUserNo, mysql, &MySqlLite::sloMQSelectUserNo);
    connect(mysql, &MySqlLite::MQSelectUserNoSuc, loginForm, &LoginForm::LogSelectUserNoSuc);
    connect(loginForm, &LoginForm::loginsuccessRoleId, this, &MainForm::checkjurstr);
//    connect(mysql,&MySqlLite::loginError,this,&MainForm::showRedLamp);

}


void MainForm::checkjurstr(int type,const QString &username,const QString &roleId){
#ifdef LOCK_LAMP
    if(type != loginForm->Exit && type != loginForm->Unlock){
//        if (!lampThread) {
//            lampThread = new LampThread(this);
//            lampThread->start(); // 启动线程
//        }
         // 添加任务和各自的延迟时间（单位：毫秒）
        lampThread->addTask([this]() { lampThread->turnOffLamp(8); }, 0); // 1秒后关闭灯
        lampThread->addTask([this]() { lampThread->turnOnLamp(7); }, 1000);  // 0.5秒后打开灯
        lampThread->addTask([this]() { lampThread->turnOffLamp(7); }, 3000); // 2秒后关闭灯
        lampThread->addTask([this]() { lampThread->turnOnLamp(8); }, 1000); // 1.5秒后打开灯
        lampThread->addTask([this]() { lampThread->turnOffLamp(7); }, 500); // 2秒后关闭灯
    }


#endif

     loginForm->hide();
     loginForm->deleteLater();
     loginForm = nullptr;
     if(type == LoginForm::DataQuery){
     bool videojur =false;

        // emit gosearch(username,roleId,videojur);

         SearchByType *searchbytype = new SearchByType(username,roleId,mysql,mynet,videojur);
         searchbytype->show();

     }else if(type == LoginForm::SystemSettings){

         SystemManageWindow *sysWinManage = new SystemManageWindow(username,roleId,mysql,mynet,myipv4Address);
     //    sysWinManage->setSystemIPV4Label(myipv4Address);
         sysWinManage->show();
     }


     else if(type == loginForm->DevicePair){

     }


     else if(type == loginForm->Unlock){
#ifdef LOCK_LAMP
//    pushButtonOpenOneKey();


     lampThread->addTask([this]() { lampThread->OpenOneKey(); }, 0);


#endif
     }else if(type == loginForm->Update){
        // 发送当前版本给服务器
       emit mainToNetSelect(ui->versiontextlabel->text(),myipv4Address);
     }else if(type == loginForm->Exit){


         is_mainwindow_exited = true;


#ifdef LOCK_LAMP

        lampThread->addTask([this]() { lampThread->turnOffLamp(8); }, 0); // 1秒后关闭灯
        lampThread->addTask([this]() { lampThread->turnOffLamp(7); }, 0); // 1秒后关闭灯
        lampThread->addTask([this]() { lampThread->turnOffLamp(6); }, 0); // 1秒后关闭灯
        usleep(2000 * 1000);

        closePort();

#else

        qApp->quit();
#endif

     }else if(type == loginForm->PersonalCenter){

      }
     emit insertt_logToMSQ(username,type);
}

void MainForm::updateNewProgress(qreal progress,int zfynum)
{

   // dasbuddy[zfynum]->setUploadProgressBar(progress);

    //ui->pushButton_searchfiles->setText("fsdfsdfsdfg");

    if(100 - progress <= 0.00001){

      // dasbuddy[zfynum]->setCurrentStatus(3);
    }

}

QString MainForm::getBattery(QString path){

    //zfylist[0].charge
    QRegExp batteryText("Battery0=(\\d{1,2})");
    QString batteryNumber = "-1";
    QFile file(path);
    //Battery0=(\d{1,2})
    QString string;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!file.atEnd())
        {
            string = file.readLine();
            batteryText.indexIn(string);
            //qDebug()<<batteryTest.cap(1);

        }
        file.close();
        batteryNumber = batteryText.cap(1);
        qDebug()<<"batteryNumber"<<batteryText.cap(1);


    }
    return batteryNumber;

}
void MainForm::closeThread(){

    qDebug()<<tr("关闭线程");
    if(thread1->isRunning())
    {
        thread1->quit();            //退出事件循环
        thread1->wait();            //释放线程槽函数资源
    }
}

void MainForm::finishedThreadSlot(){

    qDebug()<<tr("多线程触发了finished信号");
}


void MainForm::toinsertZFYDialog()
{


        if(this->isPairDevice){
            checkUsbDrivesInfos();
        }else{
            checkUsbDrives();
        }






}



void MainForm::requestFinished(QNetworkReply* reply) {

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
                QJsonArray result_data = root_Obj.value("data").toArray();

                QStringList userlist;
                for (int i=0;i<result_data.count();i++) {
                    QString usermsg = result_data.at(i).toObject().value("polno").toString()
                            +":" + result_data.at(i).toObject().value("polName").toString();
                    userlist.append(usermsg);
                }

                emit getUsers(userlist);


            }

        }

    }
}


void MainForm::updatedisksizelabel(double disksize,bool pingresult,double totalSize,double useSize)
{


    QString text = "总共:";
    QString totalStr;
    double totalTmp = totalSize;
    double usePer = 100.0 * useSize / totalSize;

    if(totalTmp > 1000.0){
        totalTmp = totalTmp/1024.0;
        totalStr = QString::number(totalTmp,'f',2) + "TB";
    }else{
        totalStr = QString::number(totalTmp,'f',2) + "GB";
    }

    text = QString("已用:%1GB,%2%,总共:%3")
            .arg(QString::number(useSize,'f',2))
            .arg(QString::number(usePer,'f',0))
            .arg(totalStr);
    ui->label_disksize->setText(text);

}
void MainForm::setIPV4Label(QString ipv4Address){
//    ui->label_hostip->setText(ipv4Address);
    this->myipv4Address=ipv4Address;

    connect(this,&MainForm::MainFselectWeekUserInfoTOMSQ,mysql,&MySqlLite::MSQselectWeekUserInfoTOMainF);
    connect(mysql,&MySqlLite::MSQselectWeekUserInfoTONet,mynet,&NetworkUtility::uploadUserInfoSyncData);

    connect(this,&MainForm::MainFselectWeekFileTOMSQ,mysql,&MySqlLite::MSQselectWeekFileInfoTOMainF);
    connect(mysql,&MySqlLite::MSQselectWeekFileInfoTONet,mynet,&NetworkUtility::uploadFileSyncData);

    emit MainFselectWeekUserInfoTOMSQ( myipv4Address);
    emit MainFselectWeekFileTOMSQ( myipv4Address);
    qDebug()<<"MainForm::setIPV4Label :"<<myipv4Address;
}


void MainForm::setDbusDevicePairStatus(int _idx, bool _sign)
{


    dasbuddy[_idx]->setDevicePairStatus(_sign);
    dasbuddy[_idx]->setDeviceShowAll(_sign);

    if(!isstandard){


    }
}

void MainForm::setDbusDevicePairInfos(int idx,int offval, QString diskPath)
{
    dasbuddy[idx]->setDevicePairInfos(offval,diskPath);
}


void MainForm::deleteCurrentlyCopying(QString _path)
{

    currentlyCopying.remove(_path);



}


bool MainForm::isInteger(const string &str)
{
   // 正则表达式：可选的符号 (+ 或 -)，后跟一个或多个数字
   std::regex pattern("^[+-]?[0-9]+$");

   // 使用 std::regex_match 来判断字符串是否匹配该模式
   return std::regex_match(str, pattern);
}

void MainForm::releaseSysCaches()
{

   sync();

   std::ofstream ofs("/proc/sys/vm/drop_caches", std::ios::out);
   if (!ofs.is_open()) {
       std::cerr << "无法打开文件 /proc/sys/vm/drop_caches" << std::endl;
       return;
   }


   ofs << "1\n";
   ofs << "2\n";
   ofs << "3\n";

   ofs.close();
}



void MainForm::recSigTaskWork(int type,bool stat,int winidx)
{


    emit setSigTaskDeviceType(type,winidx);
    emit setSigTaskWork(stat,winidx);
}



int32_t MainForm::myexec(const char *cmd, vector<string> &resvec)
{


    resvec.clear();
    FILE *pp = popen(cmd, "r");
    if (!pp)
    {
        return -1;
    }
    char tmp[1024];
    while (fgets(tmp, sizeof(tmp), pp) != NULL)
    {
        if (tmp[strlen(tmp) - 1] == '\n')
        {
            tmp[strlen(tmp) - 1] = '\0';
        }
        resvec.push_back(tmp);
    }
    pclose(pp);
    for (int i = 0; i < resvec.size(); i++)
    {
        //cout << resvec.at(i) << endl;
        //cout << "-----------------------------" << endl;
    }
    return resvec.size();
}
QStringList  MainForm::getUsbPaths() {

    printf("%s------------%d\n", __FUNCTION__, __LINE__);
    DIR *dir_ptr;
    QStringList path_list;
    if ((dir_ptr = opendir("/sys/block")) == NULL) {
        printf("No USB storage detected.\n");
        return path_list;
    }

    const char *sdx[] = {"sda", "sdb", "sdc", "sdd", "sde", "sdf"};
    for (int i = 0; i < 6; i++) {
        char open_path[64] = {0};
        sprintf(open_path, "/sys/block/%s/removable", sdx[i]);
        printf("Checking path: [%s]\n", open_path);

        int fd = open(open_path, O_RDONLY);
        if (fd == -1) {
            printf("Failed to open [%s]\n", open_path);
            continue;
        }

        char buf[32] = {0};
        if (read(fd, buf, sizeof(buf)) > 0 && buf[0] == '1') {
            printf("USB found at [%s]\n", sdx[i]);
            vector<string> resvec;
            char df_path[64] = {0};
            sprintf(df_path, "df -h | grep /dev/%s", sdx[i]);
            myexec(df_path, resvec);
            if (resvec.size() == 0) {
                printf("No USB found in df output, manual mount may be required.\n");
            } else {
                cout << resvec.back() << endl;
                string df_output = resvec.back();
                size_t last_space = df_output.find_last_of(" \t");
                if (last_space != string::npos) {
                    string mount_path = df_output.substr(last_space + 1);
                    path_list.append(QString::fromStdString(mount_path));
                    printf("Mount path: [%s]\n", mount_path.c_str());
                }
            }
        }

    }

    return path_list;
}
int32_t MainForm::myexecTest(const char *cmd, std::vector<std::string> &resvec) {

    resvec.clear();
    FILE *pp = popen(cmd, "r"); //建立管道
    if (!pp) {
        return -1;
    }
    char tmp[1024]; //设置一个合适的长度，以存储每一行输出
    while (fgets(tmp, sizeof(tmp), pp) != NULL) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0'; //去除换行符
        }
        resvec.push_back(tmp);
    }
    pclose(pp); //关闭管道
    return resvec.size();
}

QStringList MainForm::getUsbPathsTest() {
    QDir dir("/dev");
    QStringList path_list;

    const char *sd_chars = "abcdefghijklmnopqrstuvwxyz";
    for (const char *c = sd_chars; *c; ++c) {
        QString usb_device = QString("/dev/sd%1").arg(*c);
        QFile usb_device_file(usb_device);
        if (usb_device_file.exists()) {
            std::vector<std::string> resvec;
            char df_path[64];
            sprintf(df_path, "df -h | grep %s", qPrintable(usb_device));
            myexec(df_path, resvec);
            if (!resvec.empty()) {
                std::string df_output = resvec.back();
                size_t last_space = df_output.find_last_of(" \t");
                if (last_space != std::string::npos) {
                    std::string mount_path = df_output.substr(last_space + 1);
                    path_list.append(QString::fromStdString(mount_path));
                }
            }
        }
    }

    return path_list;
}
void MainForm::myRemoveCurrentDrives(QString sourceDirPath){
    currentDrives.removeOne(sourceDirPath);
}

void MainForm::checkUsbDrives() { //checkCustomDrives

    qRegisterMetaType<QList<QVariantMap>>("QList<QVariantMap>");

    QStringList paths = getUsbMountPoints();

//    QStringList paths = getUsbPaths();
    for (const QString &path : paths) {

        if (!path.isEmpty() && currentlyCopying.find(path) == currentlyCopying.end()) {


            currentlyCopying.insert(path);
            qDebug() << "Added to currentlyCopying:" << path;
            qDebug() << "currentlyCopying contents:";
            for (const QString& item : currentlyCopying) {
                qDebug() << item;
            }


            QString  _hsFileNameIni = path + "/hsAuth.ini";


            QFile _hsKeydel(_hsFileNameIni);
            if (_hsKeydel.exists()) {
                _hsKeydel.remove();
                _hsKeydel.close();

            }

            int dasbuddyNum = -1;

            bool is_Jy = false;

            if(!readConfigFile(path)){


                    QString  _jyFileNameDat = path + "/DCIM/100MEDIA/Info.dat";
                    QFile _jyFile(_jyFileNameDat);

                    if (_jyFile.exists()) {

                        if (_jyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                           QByteArray _oneline  = _jyFile.readLine();
                           QString strline = QString::fromUtf8(_oneline);

                           if(!strline.isEmpty()){
                                QStringList strspl = strline.split(";");
                                currentlyCopyHash.insert(path,  strspl[2]);

                                is_Jy = true;

                           }


                        }
                    }





                    if(!is_Jy){

                        QFile _hsKeyFile(_hsFileNameIni);
                        if (!_hsKeyFile.exists()) {

                            if (!_hsKeyFile.open(QIODevice::WriteOnly | QIODevice::Text)) {

                                continue;
                            }

                            // 关闭文件
                            _hsKeyFile.flush();
                            _hsKeyFile.close();

                            QSettings settings(_hsFileNameIni, QSettings::IniFormat);

                            settings.beginGroup("ZFY");
                            // 写入字符串
                            //settings.setValue("General/name", "John Doe");
                            //settings.setValue("General/email", "johndoe@example.com");

                            settings.setValue("DevNo", "000000");
                            settings.setValue("PolNo", "000000");
                            settings.sync();

                            dasbuddyNum = getUsableWindowProgress();
                            setWindowProgressOccupied(dasbuddyNum,true);
                            hsMapDisks[dasbuddyNum] = path;

                            connect(threadCKDisk,&CheckHsDiskThread::sigOfflineDisk,this,&MainForm::setDbusDevicePairInfos, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                            connect(threadCKDisk,&CheckHsDiskThread::sigOffDasbuddy,this,&MainForm::setDbusDevicePairStatus, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                            connect(threadCKDisk,&CheckHsDiskThread::sigOffWindowId,this,&MainForm::setWindowProgressOccupied, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                            connect(threadCKDisk,&CheckHsDiskThread::sigDeleteCopyPtah,this,&MainForm::deleteCurrentlyCopying, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));

                            connect(dasbuddy[dasbuddyNum],&DASBuddy::sigOffWindowId,this,&MainForm::setWindowProgressOccupied, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));


                            dasbuddy[dasbuddyNum]->setDevicePairStatus(true);

                            dasbuddy[dasbuddyNum]->setDevicePairInfos(dasbuddyNum,path);

                            continue;

                        }




                        continue;

                    }

            }


            for (auto _tmpdevpair = hsMapDisks.cbegin(); _tmpdevpair != hsMapDisks.cend(); ++_tmpdevpair) {
                 qDebug() << _tmpdevpair.key() << ":" << _tmpdevpair.value();

                 QString _tmpDiskPath = _tmpdevpair.value().toString();
                 if(_tmpDiskPath == path){

                     dasbuddyNum = _tmpdevpair.key();
                 }


            }

            if(dasbuddyNum == -1){

                dasbuddyNum = getUsableWindowProgress();
                setWindowProgressOccupied(dasbuddyNum,true);
            }



            hsDynamicMapDisks[dasbuddyNum] = path;
            hsMapDisks[dasbuddyNum] = path;
            connect(threadCKDisk,&CheckHsDiskThread::sigOfflineDisk,this,&MainForm::setDbusDevicePairInfos, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
            connect(threadCKDisk,&CheckHsDiskThread::sigOffDasbuddy,this,&MainForm::setDbusDevicePairStatus , Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
            connect(threadCKDisk,&CheckHsDiskThread::sigOffWindowId,this,&MainForm::setWindowProgressOccupied, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
            connect(threadCKDisk,&CheckHsDiskThread::sigDeleteCopyPtah,this,&MainForm::deleteCurrentlyCopying, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
            connect(dasbuddy[dasbuddyNum],&DASBuddy::sigOffWindowId,this,&MainForm::setWindowProgressOccupied, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));


            // 创建一个新的线程
            QThread *thread = new QThread;
            CopyTask *task = new CopyTask(path, dasbuddyNum);
            task->moveToThread(thread);
            copyThreadList.append(task);

            connect(thread, &QThread::started, task, &CopyTask::run);

            disconnect(mysql, &MySqlLite::mysqlWorkFileClientAdd, mynet,&NetworkUtility ::netWorkFileClientAdd);


            connect(task, SIGNAL(progressUpdated(qreal,int)), this, SLOT(updateNewProgress(qreal,int)), Qt::QueuedConnection);
            connect(task, SIGNAL(onCopyFinishedCT(QString,int)), this, SLOT(onCopyFinished(QString,int)), Qt::QueuedConnection);
            connect(task, SIGNAL(CopyTaskinsertfiletestemit(QList<QVariantMap>,bool,QString,int)), mysql, SLOT(doinsertfiles(QList<QVariantMap>,bool,QString,int)), Qt::QueuedConnection);
            connect(task, SIGNAL(deletFileData(QString,int)), mysql, SLOT(selectt_filedata(QString,int)),Qt::QueuedConnection);

            connect(mysql, SIGNAL(readyToCopyFile(QString,int)), task, SLOT(startcopyFile(QString,int)), Qt::QueuedConnection);
            //connect(mysql, SIGNAL(deleteThreadMQ(int)), task, SLOT(deleteThreadCT(int)), Qt::QueuedConnection);
            connect(mysql, SIGNAL(sourceFileErrUnplugUsbDevice(const QString &,int )), task, SLOT(isUnplugUsbDeviceCT(const QString &,int)), Qt::QueuedConnection);
//            connect(mysql, SIGNAL(onCopyFinishedMQ(QString,int)), this, SLOT(onCopyFinished(QString,int)), Qt::QueuedConnection);
            connect(task, SIGNAL(sigCurrentCopyProgressInfo(int,qreal,qint64,qint64)),this,SLOT(currentCopyDeviceProgressInfo(int,qreal,qint64,qint64)),Qt::QueuedConnection);
            connect(task, SIGNAL(sigCurrentCopyDeviceFileTotalCount(int,qint64,qint64)),this,SLOT(currentCopyDeviceFileTotalCount(int,qint64,qint64)),Qt::QueuedConnection);

            connect(task, SIGNAL(sigCurrentCopyDeviceInfo(int,QString,QString,QString,QString)),this,SLOT(currentCopyDeviceInfo(int,QString,QString,QString,QString)),Qt::QueuedConnection);
            connect(mysql,SIGNAL(mysqlClientAdd(const QVariantMap &)),mynet,SLOT(netWorkClientAdd(const QVariantMap &)), Qt::QueuedConnection);
            connect(task, SIGNAL(insertT_UserInfoCTtoMain(const QVariantMap &)),mysql,SLOT(insertT_UserInfoCTtoMSQ(const QVariantMap &)),Qt::QueuedConnection);
            connect(mysql,SIGNAL(mysqlWorkFileClientAdd(QList<QVariantMap> )),mynet,SLOT(netWorkFileClientAdd(QList<QVariantMap> )),Qt::UniqueConnection);
//            connect(mysql,SIGNAL(waitUnplugUsbDevice(const QString &,int )),this,SLOT(isUnplugUsbDeviceMainForm(const QString &,int)),Qt::QueuedConnection);Qt::DirectConnection
            connect(mysql,SIGNAL(waitUnplugUsbDevice(const QString &,int )),task,SLOT(isUnplugUsbDeviceCT(const QString &,int)),Qt::QueuedConnection);

//            qDebug() << "Current connections for mysqlWorkFileClientAdd:" << QObject::receivers("mysqlWorkFileClientAdd");

            // 线程结束时清理
            connect(task, &CopyTask::finished, thread, &QThread::quit);
            connect(task, &CopyTask::finished, task, &CopyTask::deleteLater);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);

            //send signal to CopyTask
//            connect(this, SIGNAL(sigSetPriorityCollect(int,bool)), task, SLOT(setPriorityCollect(int,bool)), Qt::QueuedConnection);

            // 开始线程
            thread->start();
            //qDebug() << "Started thread for path:" << path;
            windownum++;
        }
    }
}




void MainForm::checkUsbDrivesInfos() {

    qRegisterMetaType<QList<QVariantMap>>("QList<QVariantMap>");


    QList<UsbPathAndInfo> pathList  ;

    try{
         pathList = getUsbPathsAndInfo();

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MainForm::getUsbPathsAndInfo 捕获到异常: ============================ " << e.what() << std::endl;
    }



    if(!pathList.isEmpty()){


        std::cout << "pathList.size()=" << pathList.size() << endl;
        for(int i = 0;i < pathList.size();++i){
            QString path = pathList.at(i).path;
            if(QDir(path).exists()){

            }else{

                try{
                    currentlyCopying.remove(path);
                    continue;
                }catch(const std::exception& e){

                    // 处理异常
                     std::cerr << " ============================ currentlyCopying.remove 捕获到异常: ============================ " << e.what() << std::endl;
                }


            }


            if (!path.isEmpty() && currentlyCopying.find(path) == currentlyCopying.end()) {

                currentlyCopying.insert(path);
                qDebug() << "Added to currentlyCopying:" << path;
                qDebug() << "currentlyCopying contents:";
                for (const QString& item : currentlyCopying) {
                    qDebug() << item;
                }

                 qDebug() << "windownumSQL" << windownum;
                 int dasbuddyNum = pathList.at(i).portNum;



                 if(dasbuddyNum < 0){
                     continue;
                 }

                 QString  _hsFileNameIni = path + "/hsAuth.ini";

                 QFile _hsKeydel(_hsFileNameIni);
                 if (_hsKeydel.exists()) {
                     _hsKeydel.remove();
                     _hsKeydel.close();

                 }



                bool is_Jy = false;

                if(!readConfigFile(path)){

                    QString  _jyFileNameDat = path + "/DCIM/100MEDIA/Info.dat";
                    QFile _jyFile(_jyFileNameDat);

                    if (_jyFile.exists()) {
                        // 打开文件
                        if (_jyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                           QByteArray _oneline  = _jyFile.readLine();
                           QString strline = QString::fromUtf8(_oneline);

                           if(!strline.isEmpty()){
                                QStringList strspl = strline.split(";");
                                qDebug() << " =========== info.dat : " << strspl[0] << " , " << strspl[2]  ;
                                currentlyCopyHash.insert(path,  strspl[2]);

                                is_Jy = true;

                           }


                        }
                    }



                      if(!is_Jy){

                          QFile _hsKeyFile(_hsFileNameIni);

                            if (!_hsKeyFile.exists()) {

                                if (!_hsKeyFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {

                                    continue;
                                }

                                QTextStream _stream(&_hsKeyFile);
                                //QString _text = "[ZFY]\r\nDevNo=" + _devNo + "\r\nPolNo=" + _usrNo + "\r\n";
                                QString _text = "[ZFY]\r\nDevNo=000000\r\nPolNo=000000\r\n";


                                _stream.setCodec("UTF-8");
                                _stream << _text;

                                // 关闭文件
                                _hsKeyFile.flush();
                                _hsKeyFile.close();

                                // 2.设置 窗口id 使用中状态

                                if(dasbuddyNum < 0){
                                     //dasbuddyNum = getUsableWindowProgress();
                                }


                                setWindowProgressOccupied(dasbuddyNum,true);
                                hsMapDisks[dasbuddyNum] = path;


                                connect(threadCKDisk,&CheckHsDiskThread::sigOfflineDisk,this,&MainForm::setDbusDevicePairInfos, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                                connect(threadCKDisk,&CheckHsDiskThread::sigOffDasbuddy,this,&MainForm::setDbusDevicePairStatus, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                                connect(threadCKDisk,&CheckHsDiskThread::sigOffWindowId,this,&MainForm::setWindowProgressOccupied, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                                connect(threadCKDisk,&CheckHsDiskThread::sigDeleteCopyPtah,this,&MainForm::deleteCurrentlyCopying, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));


                                connect(dasbuddy[dasbuddyNum],&DASBuddy::sigOffWindowId,this,&MainForm::setWindowProgressOccupied,  Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));

                                dasbuddy[dasbuddyNum]->setDevicePairStatus(true);

                                dasbuddy[dasbuddyNum]->setDevicePairInfos(dasbuddyNum,path);

                                continue;

                            }



                            continue;

                      }

                }


                try {
                    for (auto _tmpdevpair = hsMapDisks.cbegin(); _tmpdevpair != hsMapDisks.cend(); ++_tmpdevpair) {

                         QString _tmpDiskPath = _tmpdevpair.value().toString();
                         if(_tmpDiskPath == path){

                             dasbuddyNum = _tmpdevpair.key();
                         }
                    }

                }catch(const std::exception& e){

                    // 处理异常
                     std::cerr << " ============================  hsMapDisks.cbegin(); _tmpdevpair != hsMapDisks.cend(); 捕获到异常: ============================ " << e.what() << std::endl;
                }



                if(dasbuddyNum < 0){

                    //dasbuddyNum = getUsableWindowProgress();
                    continue;

                }
                setWindowProgressOccupied(dasbuddyNum,true);
                hsDynamicMapDisks[dasbuddyNum] = path;

                hsMapDisks[dasbuddyNum] = path;
                connect(threadCKDisk,&CheckHsDiskThread::sigOfflineDisk,this,&MainForm::setDbusDevicePairInfos, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                connect(threadCKDisk,&CheckHsDiskThread::sigOffDasbuddy,this,&MainForm::setDbusDevicePairStatus , Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                connect(threadCKDisk,&CheckHsDiskThread::sigOffWindowId,this,&MainForm::setWindowProgressOccupied, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
                connect(threadCKDisk,&CheckHsDiskThread::sigDeleteCopyPtah,this,&MainForm::deleteCurrentlyCopying, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));

                connect(dasbuddy[dasbuddyNum],&DASBuddy::sigOffWindowId,this,&MainForm::setWindowProgressOccupied, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));


                currentUsbPortAndWinId.insert(pathList.at(i).usbPort,dasbuddyNum);
                currentUsbPortAndPath.insert(pathList.at(i).usbPort,path);


                QThread *thread = new QThread;
                CopyTask *task = new CopyTask(path, dasbuddyNum);
                task->moveToThread(thread);
                copyThreadList.append(task);


                connect(thread, &QThread::started, task, &CopyTask::run);


                // MySqlLite及其QSqlDatabase连接在主线程创建，不能跟随每个
                // 拷贝任务反复移动线程；跨线程信号会以队列方式回到主线程执行。

                NetworkUtility *myNet = new NetworkUtility();
                // 连接信号和槽，使用 Qt::QueuedConnection
                connect(task, SIGNAL(progressUpdated(qreal,int)), this, SLOT(updateNewProgress(qreal,int)), Qt::QueuedConnection);
                connect(task, SIGNAL(onCopyFinishedCT(QString,int)), this, SLOT(onCopyFinished(QString,int)), Qt::QueuedConnection);
                connect(task, SIGNAL(CopyTaskinsertfiletestemit(QList<QVariantMap>,bool,QString,int)), mysql, SLOT(doinsertfiles(QList<QVariantMap>,bool,QString,int)), Qt::QueuedConnection);
                connect(task, SIGNAL(deletFileData(QString,int)), mysql, SLOT(selectt_filedata(QString,int)), Qt::QueuedConnection);
                connect(mysql, SIGNAL(readyToCopyFile(QString,int)), task, SLOT(startcopyFile(QString,int)), Qt::QueuedConnection);

                //connect(mysql, SIGNAL(deleteThreadMQ(int)), task, SLOT(deleteThreadCT(int)), Qt::QueuedConnection);
                connect(mysql, SIGNAL(sourceFileErrUnplugUsbDevice(const QString &,int )), task, SLOT(isUnplugUsbDeviceCT(const QString &,int)), Qt::QueuedConnection);
//                connect(mysql, SIGNAL(onCopyFinishedMQ(QString,int)), this, SLOT(onCopyFinished(QString,int)), Qt::QueuedConnection);
                connect(task, SIGNAL(sigCurrentCopyProgressInfo(int,qreal,qint64,qint64)),this,SLOT(currentCopyDeviceProgressInfo(int,qreal,qint64,qint64)),Qt::QueuedConnection);
                connect(task, SIGNAL(sigCurrentCopyDeviceFileTotalCount(int,qint64,qint64)),this,SLOT(currentCopyDeviceFileTotalCount(int,qint64,qint64)),Qt::QueuedConnection);
                connect(task, SIGNAL(sigCurrentCopyDeviceInfo(int,QString,QString,QString,QString)),this,SLOT(currentCopyDeviceInfo(int,QString,QString,QString,QString)),Qt::QueuedConnection);
                connect(mysql,SIGNAL(mysqlClientAdd(const QVariantMap &)),myNet,SLOT(netWorkClientAdd(const QVariantMap &)), Qt::QueuedConnection);
                connect(task, SIGNAL(insertT_UserInfoCTtoMain(const QVariantMap &)),mysql,SLOT(insertT_UserInfoCTtoMSQ(const QVariantMap &)),Qt::QueuedConnection);
                connect(mysql,SIGNAL(mysqlWorkFileClientAdd(QList<QVariantMap>)),myNet,SLOT(netWorkFileClientAdd(QList<QVariantMap> )), Qt::QueuedConnection);
    //            connect(mysql,SIGNAL(waitUnplugUsbDevice(const QString &,int )),this,SLOT(isUnplugUsbDeviceMainForm(const QString &,int)),Qt::QueuedConnection);
                connect(mysql,SIGNAL(waitUnplugUsbDevice(const QString &,int )),task,SLOT(isUnplugUsbDeviceCT(const QString &,int)),Qt::QueuedConnection);

                // 线程结束时清理
                connect(task, &CopyTask::finished, thread, &QThread::quit);
                connect(task, &CopyTask::finished, task, &CopyTask::deleteLater);
                connect(thread, &QThread::finished, thread, &QThread::deleteLater);

                //send signal to CopyTask
    //            connect(this, SIGNAL(sigSetPriorityCollect(int,bool)), task, SLOT(setPriorityCollect(int,bool)), Qt::QueuedConnection);

                // 开始线程
                thread->start();
                //qDebug() << "Started thread for path:" << path;
                //windownum++;

            }


        }


    }else{

        qDebug() <<  " ============================================ pathList is  null ============================================ " ;

    }

}





void MainForm::isUnplugUsbDeviceMainForm(const QString &usbpath,int windowNum){

    QStringList usbPaths = getUsbPathsTest();

    if (!usbPaths.contains(usbpath)) {
        // 条件满足，停止定时任务
        if(timerRunningIsUnplug) {
            timerIsUnplug->stop();
            timerRunningIsUnplug = false;
        }
        onCopyFinished(usbpath,windowNum);
    } else {
        // 条件不满足，启动定时任务
        if(!timerRunningIsUnplug) {
            timerIsUnplug = new QTimer(this);
            connect(timerIsUnplug, &QTimer::timeout, this, [=]() {
                isUnplugUsbDeviceMainForm(usbpath, windowNum);
            });
            timerIsUnplug->start(1000); // 1秒钟触发一次
            timerRunningIsUnplug = true;
        }
    }
}

void MainForm:: onCopyFinished(const QString &path,int windowNum) {

    currentlyCopying.remove(path);

    dasbuddy[windowNum]->setUploadProgressBar(0);
    dasbuddy[windowNum]->setCurrentStatus(0);

    setWindowProgressOccupied(windowNum,false);

    if (currentlyCopyHash.contains(path)) {
        QString value = currentlyCopyHash.take(path); // 查找并删除
        emit insertt_logToMSQ(value,7);
        qDebug() << "Removed" << path << "with value:" << value;
    } else {
        qDebug() << "Key not found.";
    }
    qDebug() << "MainForm::onCopyFinished:" << path<<" windowNum:"<<windowNum;
}



void MainForm::currentCopyDeviceInfo(int windowNum, QString userNo, QString deviceNo,QString userName,QString corpName){

    NewUserInfo userInfo = mysql->getUserInfoByUserNo(userNo);
    dasbuddy[windowNum]->setCurrentUserNameAndDepartment(userInfo.F_UserName,userInfo.departmentName);

    dasbuddy[windowNum]->setCurrentUserAndDevice(userNo,deviceNo);
    // dasbuddy[windowNum]->setCurrentUserNameAndDepartment(userName,corpName);

}

void MainForm::currentCopyDeviceFileTotalCount(int windowNum, qint64 fileTotalCount,qint64 cpySuccessFileCount){

     dasbuddy[windowNum]->setCurrentFileTotalCount(fileTotalCount,cpySuccessFileCount);
}

void MainForm::currentCopyDeviceProgressInfo(int windowNum, qreal progress,qint64 copiedCompleteSize,qint64 fileTotalSize){
     dasbuddy[windowNum]->setCurrentCopyProgressInfo(progress,copiedCompleteSize,fileTotalSize);
}



void MainForm::setPriorityCollect(int windowNum,bool isPriority){
    //
    for(const CopyTask *item : copyThreadList){

        qDebug()<<"MainForm::setPriorityCollect getWindowNumber :"<<item->getWindowNumber()<<" :isPriority"<<isPriority;
        if(isPriority){

            if(item->getWindowNumber() != windowNum){

                // 别人暂停
                item->setZFYCPStaus();
            }else{

                //  自己运行
                //item->setZFYCPStausStart();
            }
        }else{

            item->setZFYCPStausStart();
        }
    }
//    emit sigSetPriorityCollect(windowNum,isPriority);

}


void MainForm::gotoPersonCenter(){
    std::string devPath = "sdf";
    std::string usbPort = "";
    std::string insetTime = "";
    checkLogForDevice(devPath,usbPort,insetTime);
    std::cout << "gotoPersonCenter:" <<"usbPort:" << usbPort.c_str() << "insetTime:" << insetTime.c_str() <<endl;


    QList<UsbPathAndInfo> pathList = getUsbPathsAndInfo();
    if(!pathList.isEmpty()){
        std::cout << "pathList.size()=" << pathList.size() << endl;
        for(int i = 0;i < pathList.size();++i){
            std::cout << "path:"<< pathList.at(i).path.toStdString().c_str() << endl;
            std::cout << "usbPort:"<< pathList.at(i).usbPort.toStdString().c_str() << endl;
            std::cout << "portNum:"<< pathList.at(i).portNum << endl;
        }

    }else{
        std::cout << "pathList is empty." << endl;
    }


}


void MainForm::initAllWindowProgress(int windowsCount) {
    // init windows Progress List
    if(!windowProgressList.empty()){
        windowProgressList.clear();
    }
    for (int i = 0; i < windowsCount; i++) {
        WindowProgress windowProgress;
        windowProgress.isOccupied = false;
        windowProgress.windowId = i;
        windowProgressList.push_back(windowProgress);

    }
    return;
}


bool MainForm::isWindowProgressOccupied(int windowsId) {
    bool isOccupied = false;
    for (int i = 0; i < windowProgressList.size(); i++) {
        if(windowsId == windowProgressList.at(i).windowId){
            isOccupied = windowProgressList.at(i).isOccupied;
            break;
        }
    }
    return isOccupied;
}


int MainForm::getUsableWindowProgress() {
    int windowId = -1;
    for (int i = 0; i < windowProgressList.size(); i++) {

        // 从小到大循环 找到一个空闲id 返回
        if(!windowProgressList.at(i).isOccupied){
            windowId = windowProgressList.at(i).windowId;
            break;
        }
    }
    return windowId;
}


void MainForm::setWindowProgressOccupied(int windowsId,bool isOccupied) {

    for (int i = 0; i < windowProgressList.size(); i++) {

        if(windowsId == windowProgressList.at(i).windowId){
            windowProgressList.at(i).isOccupied = isOccupied;
        }
    }
}

int MainForm::insertOrUpdateWindowProgress() {
    // 遍历窗口进度列表，查找未被占用的窗口位置
    for (int i = 0; i < windowProgressList.size(); i++) {
        if (!windowProgressList[i].isOccupied) {
            // 更新找到的空闲窗口信息
            windowProgressList[i].isOccupied = true;
            return i; // 返回找到的窗口位置
        }
    }

    // 如果未找到空闲窗口，则添加新的窗口进度信息
    WindowProgress newWindowProgress;
    newWindowProgress.isOccupied = true;

    if (windowProgressList.empty()) {
        windowProgressList.push_back(newWindowProgress);
        return 0; // 返回新添加的窗口位置（为 0）
    } else {
        windowProgressList.push_back(newWindowProgress);
        return windowProgressList.size() - 1; // 返回新添加的窗口位置
    }
}



bool contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

std::string extractContentBetweenChars(const std::string& str, char startChar, char endChar) {
    size_t startPos = str.find(startChar);
    if (startPos == std::string::npos) {
        throw std::runtime_error("Starting character not found");
    }
    size_t endPos = str.find(endChar, startPos + 1);
    if (endPos == std::string::npos) {
        throw std::runtime_error("Ending character not found");
    }
    return str.substr(startPos + 1, endPos - startPos - 1);
}

std::string extractAfterSubstr(const std::string& str, const std::string& substr) {
    size_t pos = str.find(substr);
    if (pos != std::string::npos) {
        // 截取子字符串位于找到的pos之后
        return str.substr(pos + substr.length());
    }
    return ""; // 如果找不到子字符串，返回空字符串
}

std::string getContentBeforeSubstring(const std::string& str, const std::string& substr) {
    size_t pos = str.find(substr);
    if (pos != std::string::npos) {
        return str.substr(0, pos);
    }
    return str; // 如果找不到子串，返回原字符串
}

std::vector<std::string> split(const std::string &text, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(text);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string replaceChar(std::string str, char find, char replace) {
    std::string::size_type idx = 0;
    while ((idx = str.find(find, idx)) != std::string::npos) {
        str[idx] = replace;
        ++idx;
    }
    return str;
}

std::string extractNumbers(const std::string& input) {
    std::stringstream ss;
    std::locale locale1;
    for (char c : input) {
        if (std::isdigit(c,std::locale(locale1.name()))) {
            ss << c;
        }
    }
    return ss.str();
}

std::string removeSpaces(std::string str) {
    for (auto it = str.begin(); it != str.end(); ) {
        if (*it == ' ') {
            it = str.erase(it);
        } else {
            ++it;
        }
    }
    return str;
}

/*
std::string有多个空格时只保留一个空格
可以使用std::string的成员函数erase和find来实现。
*/
std::string collapse_spaces(const std::string &input) {

    std::string result;
    bool was_space = false; // 用于跟踪之前是否已经添加了空格
    for (char ch : input) {
        if (ch == ' ') {
            if (!was_space) {
                result += ' ';
                was_space = true;
            }
        } else {
            result += ch;
            was_space = false;
        }
    }
    // 如果字符串结束时有空格，需要去除结尾空格
    if (!result.empty() && was_space) {
        result.erase(result.end() - 1);
    }
    return result;
}


void MainForm::checkLogForDevice(const std::string devPath ,std::string& usbPort,std::string& insetTime){

    vector<string> resvec;
    char df_path[128] = {0};


    sprintf(df_path, "dmesg -T  |  grep -v 'FAT read failed' | grep -E \"usb-storage|%s\" |tail -1000 ",devPath.c_str());
    myexec(df_path, resvec);

    if (resvec.size() == 0) {
        printf("No USB found in df output, manual mount may be required.\n");
    } else {
        cout <<"cmd result size() =" << resvec.size() << endl;
        //get last contain "sisc host" and "usb-storage"
        for (auto it = resvec.rbegin(); it != resvec.rend(); ++it) {

                std::string tmp = *it;
                if(contains(tmp,"scsi host") && contains(tmp,"usb-storage")){
                    //get time and port
                    std::string timeStr = extractContentBetweenChars(tmp,'[',']');
                    std::string portStr = extractAfterSubstr(tmp,"usb-storage");
                    std::string busDeviceSeral = extractAfterSubstr(tmp,"scsi host");
                    //timeStr like this:五 7月 19 10:44:35 2024
                    std::string timeStrResult = collapse_spaces(timeStr);
                    std::vector<std::string> timeSplite = split(timeStrResult,' ');
                    string mon = "";
                    string day = "";
                    string year = "";
                    string timeHms = "";
                    if(timeSplite.size() >= 5){
                        mon = timeSplite[1];
                        day = timeSplite[2];
                        year = timeSplite[4];
                        timeHms = timeSplite[3];
                    }
                    //translate to datetime: yyyy-MM-dd hh:mm:ss
                    mon = extractNumbers(mon);
                    char timeBuffer[64] = {0};
                    if(mon.length() > 0 && day.length() > 0){
                    sprintf(timeBuffer,"%s-%02d-%02d %s",year.c_str(),std::stoi(mon),std::stoi(day),
                            timeHms.c_str());
                    }

                    //
                    std::string portWith = getContentBeforeSubstring(portStr,":");
                    portWith = replaceChar(portWith,'-','.');

                    //std::cout << "find usb port num=" << portWith << "  time=" << timeBuffer << std::endl;

                    std::string busNum = busDeviceSeral.substr(0,busDeviceSeral.find_first_of(':'));
                    busNum = removeSpaces(busNum);

                    //[五 7月 19 14:27:37 2024] sd 35:0:0:0: [sdf] Attached SCSI removable disk
                    for (auto itFindSd = resvec.rbegin(); itFindSd != resvec.rend(); ++itFindSd){
                        std::string tmpBuff = *itFindSd;
                        if(contains(tmpBuff,"Attached SCSI removable disk")){
                            std::string strBuf = tmpBuff.substr(tmpBuff.find_first_of(']')+1);

                            std::string devPathEnd = extractContentBetweenChars(strBuf,'[',']');
                            if(0 == devPath.compare(devPathEnd)){
                                //devpath is equal
                                std::string busStrTmp = strBuf.substr(0,strBuf.find_first_of(":"));
                                std::string busStr = busStrTmp.substr(busStrTmp.find_first_of("sd") + 2);
                                //std::string busStr = strBuf.substr(strBuf.find_first_of("sd") + 2,strBuf.find_first_of(":")-2);
                                busStr = removeSpaces(busStr);
                                if(0 == busStr.compare(busNum)){
                                    usbPort = removeSpaces(portWith);
                                    insetTime = string(timeBuffer);
                                    break;
                                }
                            }
                        }
                    }

                    if(usbPort.length() > 0){
                        break;
                    }
                }
            }
    }
}


void MainForm::loadPortConfig(){

    getconfig = new Config();
    //getconfig = Config::getInstance();


    if(!usbhublist.isEmpty()){
        usbhublist.clear();
    }
    int port_num = qBound(0, getconfig->Get("portInfo","port_num").toInt(), kMaxWorkstationPorts);
//    qDebug() <<"HotPlugThread port_num"<< port_num;
    for (int i=0;i<port_num;i++) {
        QString port_no = "port" + QString::number(i);
//        qDebug()<<"HotPlugThread port_no"<<port_no;
        usbhublist.append(getconfig->Get("portInfo",port_no).toString());
//        qDebug()<<"HotPlugThread port"<<i<<":"<<usbhublist.at(i);
    }
    delete getconfig;

}


QList<UsbPathAndInfo> MainForm::getUsbPathsAndInfo() {


    printf("%s------------%d\n", __FUNCTION__, __LINE__);
    DIR *dir_ptr;
    QStringList path_list;
    QStringList dev_path_list;
    QList<UsbPathAndInfo> usbPaths;
    if ((dir_ptr = opendir("/sys/block")) == NULL) {
        // printf("No USB storage detected.\n");
        // return usbPaths;
    }

   // const char *sdx[] = {"sda", "sdb", "sdc", "sdd", "sde", "sdf"};

    const char *sdx[] = {
            "sda", "sdb", "sdc", "sdd", "sde", "sdf","sdg", "sdh", "sdi", "sdj", "sdk", "sdl", "sdm",
            "sdn", "sdo", "sdp", "sdq", "sdr","sds", "sdt", "sdu", "sdv", "sdw", "sdx", "sdy", "sdz",
            "sdaa", "sdab", "sdac", "sdad","sdae", "sdaf", "sdag", "sdah", "sdai", "sdaj", "sdak", "sdal", "sdam",
            "sdan", "sdao", "sdap","sdaq", "sdar", "sdas", "sdat", "sdau", "sdav", "sdaw", "sdax", "sday", "sdaz",
            "sdba", "sdbb","sdbc", "sdbd", "sdbe", "sdbf", "sdbg", "sdbh"
    };



    QStringList _usbblks;

    char _lsdiskcmd[100] = {0};
    sprintf(_lsdiskcmd, "df -h  | grep media |awk '{print $1}'");

    vector<string> _diskresvec;
    myexec(_lsdiskcmd, _diskresvec);

    if (_diskresvec.size() == 0) {
        printf("No USB found in df output, manual mount may be required.\n");

    } else {

        foreach (auto _vard, _diskresvec) {
            qDebug() << _vard.c_str();
            //_usbblks << _vard.c_str();

            QString _qstr = _vard.c_str();
            _qstr.replace("/dev/","");

            QRegularExpression _re("\\d+");
            _qstr.replace(_re, "");

            _usbblks << _qstr;
             qDebug() << _usbblks;

        }
    }



    for (int i = 0; i < _usbblks.size(); i++) {
        char open_path[64] = {0};

        sprintf(open_path, "/sys/block/%s/removable", _usbblks[i].toStdString().c_str() );
        //printf("Checking path: [%s]\n", open_path);

        int fd = open(open_path, O_RDONLY);
        if (fd == -1) {
            //printf("Failed to open [%s]\n", open_path);
            continue;
        }

        char buf[32] = {0};
        if (read(fd, buf, sizeof(buf)) > 0 && buf[0] == '1') {
            //printf("USB found at [%s]\n", sdx[i]);
            vector<string> resvec;
            char df_path[64] = {0};

            sprintf(df_path, "df -h | grep /dev/%s", _usbblks[i].toStdString().c_str());

            myexec(df_path, resvec);
            if (resvec.size() == 0) {
                //printf("No USB found in df output, manual mount may be required.\n");
            } else {
                //cout << resvec.back() << endl;
                string df_output = resvec.back(); //
                size_t last_space = df_output.find_last_of(" \t");
                size_t first_space = df_output.find_first_of(" ");

                if (last_space != string::npos) {
                    string mount_path = df_output.substr(last_space + 1);
                    path_list.append(QString::fromStdString(mount_path));
                    //printf("Mount path: [%s]\n", mount_path.c_str());
                    string dev_path = "";
                    if(first_space != string::npos){
                        dev_path = df_output.substr(0,first_space);
                    } else {
                        dev_path = df_output;
                    }
                    dev_path_list.append(QString::fromStdString(dev_path));
                }
            }
        }

    }


    if(!path_list.isEmpty()
            && !dev_path_list.isEmpty()
            && path_list.size() == dev_path_list.size()){

        for(int i = 0;i < path_list.size();++i){
            QString mountPath = path_list.at(i);

            std::string devEnd = dev_path_list.at(i).toStdString();
            std::string devEndResult = devEnd.substr(devEnd.find_last_of('/')+1);



            int _mountLens = devEndResult.length();
            if( _mountLens > 3){

                //devEndResult = devEndResult.substr(0, _mountLens - 1);
            }


            string _lastchar = devEndResult.substr( _mountLens - 1);

            if (isInteger(_lastchar)) {

                devEndResult = devEndResult.substr(0, _mountLens - 1);
            } else {

            }

            std::string devPath = devEndResult;  // sdf1 - sdf   sdaa1
            std::string usbPort = "";
            std::string insetTime = "";

            try{
                 checkLogForDevice(devPath,usbPort,insetTime);

            }catch(const std::exception& e){

                // 处理异常
                 std::cerr << " ============================ MainForm::checkLogForDevice 捕获到异常: ============================ " << e.what() << std::endl;
            }


            if(usbPort.length() > 0 && insetTime.length() > 0){
                UsbPathAndInfo info;
                info.path = mountPath;
                info.devPath = QString::fromStdString(devEnd);
                info.devPathEnd = QString::fromStdString(devEndResult);
                info.usbPort = QString::fromStdString(usbPort);
                info.insetTime = QString::fromStdString(insetTime);
                info.portNum = usbhublist.indexOf(QString::fromStdString(usbPort));

                usbPaths.append(info);
            }
        }
    }

    return usbPaths;
}


void MainForm::loadPairDeviceConfig(){

    getconfig = new Config();


    QVariant pairDevice = getconfig->Get("wsConfig","pairDevice");
    if(pairDevice.isNull()){
        this->isPairDevice = false;
    }else{
        bool isOk = false;
        int pairDeviceResult = pairDevice.toInt(&isOk);
        if(isOk){
            if(pairDeviceResult == 0){
                this->isPairDevice = false;
            }else if(pairDeviceResult == 1){
                this->isPairDevice = true;
            }
        }

    }
    delete getconfig;
}


void MainForm::initPairDeviceThread(){


     mPairDeviceThread = new WsPairDeviceThread();
     mPairDeviceThread->start();

     qDebug()<<"deviceThread->start()";
     connect(mPairDeviceThread,SIGNAL(insertRecorder(QString)),this,SLOT(toInsertRecorder(QString)),Qt::QueuedConnection);
     connect(mPairDeviceThread,SIGNAL(deleteRecorder(QString)),this,SLOT(toDeleteRecorder(QString)),Qt::QueuedConnection);

}

void MainForm::toInsertRecorder(QString portNum){
    //do nothing.
}


void MainForm::toDeleteRecorder(QString usbPortNum){
    //do delete the windows.
    int willDeleteWinId = -1;
    QString willDeletePath = "";
    if(currentUsbPortAndWinId.contains(usbPortNum)){
       willDeleteWinId = currentUsbPortAndWinId.value(usbPortNum);
        currentUsbPortAndWinId.remove(usbPortNum);
    }
    if(currentUsbPortAndPath.contains(usbPortNum)){
       willDeletePath = currentUsbPortAndPath.value(usbPortNum);
        currentUsbPortAndPath.remove(usbPortNum);
    }
    if(willDeleteWinId != -1){
        onCopyFinished(path,willDeleteWinId);
    }
}


void MainForm::on_pushButton_PersonCenter_clicked()
{
//     showLoginForm(loginForm->PersonalCenter);
}



bool MainForm::isUsbDrive(const QString &devicePath) {
    return devicePath.contains("usb");
}

QString MainForm::getMountPath(const QString &device) {


    QString command = "lsblk -o MOUNTPOINT -n /dev/" + device;
    QProcess process;

    process.start(command);
    if (!process.waitForFinished()) {
//        qWarning() << "Command execution failed:" << command;
        return QString();  // 如果执行失败则返回空字符串
    }

    return QString(process.readAllStandardOutput()).trimmed();
}

QStringList MainForm::getUsbMountPoints() {

    QStringList mountPaths; // 用于存储所有 U 盘的挂载路径
    QDir dir("/sys/block");
    QStringList entries = dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);

    foreach (const QString &entry, entries) {
        QString devicePath = dir.absoluteFilePath(entry);

        QFileInfo fileInfo(devicePath);
        // 检查文件是否存在并是一个符号链接
        if (fileInfo.exists() && fileInfo.isSymLink()) {
            QString realPath = fileInfo.symLinkTarget();
//            qDebug() << "Real path for" << entry << ":" << realPath;

            if (isUsbDrive(realPath)) {
//                qDebug() << "Found USB Drive:" << entry;

                QString mountPath = getMountPath(entry);
//                qDebug() << "Mount path for" << entry << ":" << mountPath;

                if (!mountPath.isEmpty()) {
                    mountPaths.append(mountPath);
                }
            }
        }
    }
    return mountPaths; // 返回挂载路径集合
}




void MainForm::writeTextUTF8File(const QString &filePath,QStringList contents){

    QFile _file(filePath);
    if(_file.exists()){

        _file.remove();
    }

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

    QString _text = "";
    for(auto item : contents){

        _text += item + "\r\n";
    }

    _stream.setCodec("UTF-8");
    _stream << _text;
    _file.flush();
    _file.close();



}



QStringList MainForm::readTextGBKFiles(const QString &filePath) {

   QStringList   content ;
   QFile file(filePath);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
       qWarning() << "Failed to open file for reading:" << filePath;
       return content;
   }


   QTextStream in(&file);
   in.setCodec("GBK");

   while (!in.atEnd()) {
       QString line = in.readLine();
       //qDebug() << line;

       content << line;
   }

   file.close();

   return content;

}


QString  MainForm::findCode(const QString &fileName)
{

    QString flag = "UTF-8";
    QFile file(fileName);

    if(!file.exists()){
        return "NULL";
    }

    if (file.open(QIODevice::ReadOnly)) {


        QByteArray buffer = file.read(3);

        file.close();

        if(buffer.size() < 3){
            flag = "NULL";
            return flag;
        }

        quint8 b1 = buffer.at(0);
        quint8 b2 = buffer.at(1);
        quint8 b3 = buffer.at(2);


        if (b1 == 0xFF && b2 == 0xFE) {
            flag = "UTF-16LE";
        }
        else if (b1 == 0xFE && b2 == 0xFF) {
            flag = "UTF-16BE";
        }
        else if (b1 == 0xEF && b2 == 0xBB && b3 == 0xBF) {
            flag = "UTF-8BOM";
        }
        else {

            QTextCodec::ConverterState state;
            QTextCodec *codec = QTextCodec::codecForName("utf-8");
            codec->toUnicode(buffer.constData(), buffer.size(), &state);
            if (state.invalidChars > 0) {
                flag = "ANSI";
            }

        }


    }

    return flag;

}


void MainForm::convertCode(const QString &path)
{

      QStringList possibleNames = {"PARAM.INI", "param.ini","Param.ini"};
      foreach (const QString &name, possibleNames) {

         QString filePath = path + "/" + name;
         QString codetype = findCode(filePath );
         if(codetype != "NULL"){

             if(codetype != "UTF-8BOM"){

                 QStringList listContent =  readTextGBKFiles(filePath);
                 writeTextUTF8File( filePath, listContent);

             }


         }

      }

}




bool MainForm::readConfigFile(const QString &path) {

    QStringList possibleNames = {"PARAM.INI", "param.ini","Param.ini"};
    foreach (const QString &name, possibleNames) {

        QFile file(path + "/" + name);
        if (file.exists()) {


        currentlyCopying.insert(path);
        QSettings setting(file.fileName(), QSettings::IniFormat);


        setting.setIniCodec("utf-8");

        setting.beginGroup("ZFY");
        QString polNo = setting.value("PolNo","000000").toString();
        setting.endGroup();
        currentlyCopyHash.insert(path, polNo);
#ifdef RELEVANCE
        connect(this,&MainForm::selectPolNoRelevance,mysql,&MySqlLite::setPARAMRelevance);
        emit selectPolNoRelevance(path,polNo);
#endif

        qDebug() << "MainForm::readConfigFile .ini" << name;
#ifdef CAR_DEPOT
    QSettings setting(file.fileName(), QSettings::IniFormat);
    zfyConfig *config = new zfyConfig();
    setting.beginGroup("ZFY");
    QString devNo;
    QString polNo;

    {
//        DevicePolicyDialog dialog; // 局部变量
        DevicePolicyDialog *dialog = new DevicePolicyDialog(mysql);
//        DevicePolicyDialog *dialog = new DevicePolicyDialog(mysql, this);
//        if (dialog->exec() == QDialog::Accepted) {
//            devNo = dialog->getDevNo();
//            polNo = dialog->getPolNo();
//        }
        int result = dialog->exec();

        if (result == QDialog::Accepted) {
            devNo = dialog->getDevNo();
            polNo = dialog->getPolNo();

            // 更新配置文件中的 DevNo 和 PolNo
            if (!devNo.isEmpty()) {
                setting.setValue("DevNo", devNo);
                config->devNo = devNo;
            }
            if (!polNo.isEmpty()) {
                setting.setValue("PolNo", polNo);
                config->polNo = polNo;
            }
            setting.setValue("isFlashLight", "false");
        } else {
            // 对话框被窗口关闭而不是“确定”，写入isFlashLight
            setting.setValue("isFlashLight", "true");
        }
    }

    // 更新配置文件中的 DevNo 和 PolNo
//    if (!devNo.isEmpty()) {
//        setting.setValue("DevNo", devNo);
//        config->devNo = devNo; // 更新 config 的 devNo
//    }

//    if (!polNo.isEmpty()) {
//        setting.setValue("PolNo", polNo);
//        config->polNo = polNo; // 更新 config 的 polNo
//    }

    // 读取配置文件中的其他值
//    config->devNo = setting.value("DevNo", config->devNo).toString();
//    config->polNo = setting.value("PolNo", config->polNo).toString();

    setting.endGroup();
    if (!file.exists()){
        currentlyCopying.erase(path);
        return false;
    }
#endif // CAR_DEPOT
        return true;
        }
    }
    qDebug() << "MainForm::readConfigFile .ini NULL";
    return false;
}

void MainForm::on_btn_unlock_clicked()
{
    showLoginForm(loginForm->Unlock);
}

void MainForm::recDasBusDeviceInfo(int windowNum, QString userNo, QString deviceNo, QString userName, QString corpName)
{

     emit currentTaskDeviceInfo(  windowNum,  userNo,  deviceNo,  userName,  corpName);

}




void MainForm::openPort(){
      //如果串口已经打开了先给他关闭了
//      if(m_serialPort->isOpen())
//      {
//          m_serialPort->clear();
//          m_serialPort->close();
//      }

//      //当前选择的串口名字ttyS1
////      m_serialPort->setPortName("ttyS1");
//      m_serialPort->setPortName("ttyUSB0");


//      if(!m_serialPort->open(QIODevice::ReadWrite))//用ReadWrite的模式尝试打开串口
//      {
//          qDebug()<<"打开失败!";
//          return;
//      }
//      qDebug()<<"串口打开成功!";

//      m_serialPort->setBaudRate(QSerialPort::Baud9600,QSerialPort::AllDirections);//设置波特率和读写方向
//      m_serialPort->setDataBits(QSerialPort::Data8);//数据位为8位
//      m_serialPort->setFlowControl(QSerialPort::NoFlowControl);//无流控制
//      m_serialPort->setParity(QSerialPort::NoParity);//无校验位
//      m_serialPort->setStopBits(QSerialPort::OneStop);//一位停止位


//      connect(m_serialPort,SIGNAL(readyRead()),this,SLOT(receiveInfo()));

//      connect(pQueryThread,SIGNAL(queryLockStatus()),this,SLOT(queryLockStatus()));

//      pQueryThread->start();

  }
QString MainForm::byteArrayToHexStr(const QByteArray &ba) {
    QString hexStr;
    hexStr.reserve(ba.size() * 2);
    for (const char &ch : ba) {
        hexStr += QString::number(static_cast<unsigned char>(ch), 16).toUpper().rightJustified(2, '0');
    }
    return hexStr;
}
void MainForm::queryLockStatus(){
//    if(m_serialPort->isOpen()){
//        std::string data8 = utils->getAllLockerStateCommand(0);

//        QString sendData = QString::fromStdString(data8);

//        QByteArray ba;
//        ba = QByteArray::fromHex(sendData.toLatin1());
//        m_serialPort->write(ba);
//    }
}
//接收到单片机发送的数据进行解析
void MainForm::receiveInfo()
{
//    qDebug()<<"接收";
//        QByteArray info = m_serialPort->readAll();
//        QString result = byteArrayToHexStr(info);
//        QString recvLockStatusData = byteArrayToHexStr(info);
////        QString originText = ui->textEdit_2->toPlainText();
//        result += "\n";
////        result += originText;
////        ui->textEdit_2->setText(result);
//        qDebug()<<"MainForm::receiveInfo()"<<result;

//        if(utils->verifyKeyStatusData(recvLockStatusData.toStdString())){
//            unsigned short boardNum = utils->getBoardNumberFromRecvData(recvLockStatusData.toStdString());
//            std::string stateData = recvLockStatusData.toStdString();
//            //lockStatusData.insert(std::make_pair(boardNum,stateData));
//            lockStatusData[boardNum] = stateData;
//        }

    //qDebug()<<"receive info:"<write("0x55");
    //m_serialPort->write("0xaa");
}


void MainForm::pushButtonOpenOneKey(){

//    QVariant boardNum = ui->comboBoxBoardNum->currentText();
//    QVariant keyId = ui->comboBoxKeyId->currentText();

//    unsigned short boardNo = 0;
//    unsigned short keyNo = 1;
//    if(boardNum.canConvert<unsigned short>()){
//        boardNo = boardNum.value<unsigned short>();
//    }
//    if(keyId.canConvert<unsigned short>()){
//        keyNo = keyId.value<unsigned short>();
//    }
    //(boardNo,keyNo) 01




//    if(m_serialPort->isOpen()){
//        std::string data = utils->openOneKeyCommand(0,1);

//        QString sendData = QString::fromStdString(data);

//        QByteArray ba;
//        ba = QByteArray::fromHex(sendData.toLatin1());
//        m_serialPort->write(ba);
////        if(m_serialPort->isOpen()){
////            m_serialPort->close();
////        }
//    }

//    if(m_serialPort->isOpen()){
//        std::string data = utils->openOneKeyCommand(0,1);

//        QString sendData = QString::fromStdString(data);

//        QByteArray ba;
//        ba = QByteArray::fromHex(sendData.toLatin1());
//        m_serialPort->write(ba);
//    }

}


void MainForm::turnOnLamp(unsigned short keyId) {
//    if (m_serialPort->isOpen()) {
//        std::string data = utils->getTurnOnCommand(0, keyId);
//        QString sendData = QString::fromStdString(data);
//        if(8 == keyId){
//            sendData = "574B4C590900880880";
//        }else if(6 == keyId){
//            sendData = "574B4C59090088068E";
//        }else if(7 == keyId){
//            sendData = "574B4C59090088078F";
//        }
//        QByteArray ba = QByteArray::fromHex(sendData.toLatin1());
//        m_serialPort->write(ba);

//        QDateTime currentDateTime = QDateTime::currentDateTime();
//        qDebug()<<"MainForm::turnOnLamp"<< " keyId" <<keyId <<"sendData"<<sendData<<currentDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
//    }
}

void MainForm::turnOffLamp(unsigned short keyId) {
//    if (m_serialPort->isOpen()) {
//        std::string data = utils->getTurnOffCommand(0, keyId);
//        QString sendData = QString::fromStdString(data);
//        if(8 == keyId){
//            sendData = "574B4C590900890881";
//        }else if(6 == keyId){
//            sendData = "574B4C59090089068F";
//        }else if(7 == keyId){
//            sendData = "574B4C59090089078E";
//        }
//        QByteArray ba = QByteArray::fromHex(sendData.toLatin1());
//        m_serialPort->write(ba);

//        QDateTime currentDateTime = QDateTime::currentDateTime();

//        qDebug()<<"MainForm::turnOffLamp"<< " keyId" <<keyId <<"sendData"<<sendData<<currentDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
//    }
}

void MainForm::turnOffRedLampAndRestoreBlue() {
    turnOffLamp(RedLampChannel);
    turnOnLamp(BlueLampChannel);
}



void MainForm::on_btn_update_clicked()
{
    showLoginForm(loginForm->Update);
}

void MainForm::setbtnUpdateText(QString status)
{

    ui->btn_update->setText(status);
    ui->btn_update->setEnabled(false);
}


void MainForm::on_btn_exit_clicked()
{
    showLoginForm(loginForm->Exit);
}

void MainForm::on_btn_help_clicked()
{

    QString pdfPath = "./GCreadme.pdf"; // 替换为您的 PDF 文件路径

    if (QFile::exists(pdfPath)) {
        // 使用系统默认的 PDF 查看器打开 PDF 文件
        QProcess::startDetached("xdg-open", QStringList() << pdfPath);
    } else {
        // 文件不存在，输出调试信息
        qDebug() << "PDF file does not exist:" << pdfPath;
    }

}

void MainForm::on_pushButton_Activate_clicked()
{

//    checkActivation();




}

// 获取所有网络接口的 MAC 地址，并返回为 "-" 分隔的字符串
QString MainForm::getAllMACAddresses() {

    QProcess process;
    process.start("bash", QStringList() << "-c" << "cat /sys/class/net/e*/address");
    process.waitForFinished();
    QStringList macAddresses = QString(process.readAllStandardOutput()).split('\n', QString::SkipEmptyParts);
//    qDebug() << "MainForm::getAllMACAddresses:" << macAddresses.join("-");
    return macAddresses.join("-");
}

void MainForm::initLOGO()
{

    QString imagePath =  Config::getInstance()->Get("Images", "mainImage").toString();
    QString logoTypePath =  Config::getInstance()->Get("wsConfig", "logo").toString();

    // 创建 QPixmap 对象用于 ui->label
    QPixmap pixmap(imagePath);

    // 检查图像是否有效
    if (pixmap.isNull()) {
        // 如果图像无效，将 label_2 移动到 y = 0
        ui->label_2->move(0, ui->label_2->y());
    } else {
        // 如果图像有效，设置图像到 label
        ui->label->setPixmap(pixmap);
    }

    // 创建 QPixmap 对象用于 ui->label_2
    QPixmap pixLogomap(logoTypePath);

    // 检查 logoTypePath 图像是否有效
    if (pixLogomap.isNull()) {
        // 如果图像无效，使用默认图像
        pixLogomap.load(":/image/title2.png");
    }

    // 设置图像到 label_2
    ui->label_2->setPixmap(pixLogomap);
}

// 生成版本对应的激活码
QString MainForm::generateActivationCode(const QString &hardwareID, const QString &version) {

    QByteArray hash = QCryptographicHash::hash((hardwareID + version).toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex()).left(16);
}

// 保存激活信息到本地设置中
void MainForm::saveActivationInfo(const QString &activationCode, int days) {

//    QSettings settings("wsCompany", "MyApp");
     Config::getInstance()->Set("wsCompany","activated",true);
     Config::getInstance()->Set("wsCompany","activationCode",activationCode);
     Config::getInstance()->Set("wsCompany","hardwareID",getAllMACAddresses());
     Config::getInstance()->Set("wsCompany","firstRun",QDateTime::currentDateTime());
     Config::getInstance()->Set("wsCompany","trialDays",days);


//    settings.setValue("activated", true);
//    settings.setValue("activationCode", activationCode);
//    settings.setValue("hardwareID", getAllMACAddresses());
//    settings.setValue("firstRun", QDateTime::currentDateTime());
//    settings.setValue("trialDays", days);  // 保存版本的使用天数
}

// 从本地设置中加载激活信息
bool MainForm::loadActivationInfo(QString &activationCode, QString &hardwareID, QDateTime &firstRun, int &trialDays) {

    if ( Config::getInstance()->Get("wsCompany","activated").toBool()) {
        activationCode =  Config::getInstance()->Get("wsCompany","activationCode").toString();
        hardwareID =  Config::getInstance()->Get("wsCompany","hardwareID").toString();
        firstRun =  Config::getInstance()->Get("wsCompany","firstRun").toDateTime();
        trialDays =  Config::getInstance()->Get("wsCompany","trialDays").toInt();
        return true;
    }
    return false;
}


int MainForm::getRemainingDays(const QDateTime &firstRun, int totalDays) {

    int daysPassed = firstRun.daysTo(QDateTime::currentDateTime());
    return totalDays - daysPassed;
}

// 检查激活状态，处理激活或试用逻辑
void MainForm::checkActivation() {


    QString hardwareID = getAllMACAddresses();
    QString activationCode, storedHardwareID;
    QDateTime firstRun;
    int trialDays = 0;  // 使用天数

    // 加载激活信息
    if (loadActivationInfo(activationCode, storedHardwareID, firstRun, trialDays)) {
        // 检查设备是否匹配
        if (hardwareID != storedHardwareID) {
            QMessageBox::critical(nullptr, "错误", "程序已被移动到其他设备，无法运行。");
            exit(0);
        }

        int remainingDays = getRemainingDays(firstRun, trialDays);
        // 检查是否还有剩余的使用天数或为高级版（remainingDays < 0 表示无限期）
        if (remainingDays > 0 || trialDays < 0) {
            qDebug() << "程序已激活，剩余使用天数：" << (trialDays < 0 ? "无限制" : QString::number(remainingDays));
            ui->pushButton_Activate->setText(QString::number(trialDays));
            // 允许正常使用
        } else {
            QMessageBox::critical(nullptr, "试用期已过", "试用期已过，请购买激活码以继续使用。");
            exit(0);
        }
    } else {
        int remainingDays = getRemainingDays(QDateTime::currentDateTime(), 30);  // 默认试用为 30 天
        // 提示用户输入激活码
        QString message = QString("您还有 %1 天的试用期。请输入激活码以继续使用。").arg(remainingDays);
        QString inputCode = QInputDialog::getText(nullptr, "激活软件", message);

        // 根据不同版本生成期望的激活码
        QString trialCode = generateActivationCode(hardwareID, "trial");
        QString standardCode = generateActivationCode(hardwareID, "standard");
        QString premiumCode = generateActivationCode(hardwareID, "premium");

        // 检查用户输入的激活码
        if (inputCode == trialCode) {
            saveActivationInfo(inputCode, 30);  // 试用版 30 天
            QMessageBox::information(nullptr, "激活成功", "已激活为试用版，有效期 30 天。");
        } else if (inputCode == standardCode) {
            saveActivationInfo(inputCode, 365);  // 普通版 365 天
            QMessageBox::information(nullptr, "激活成功", "已激活为普通版，有效期 365 天。");
        } else if (inputCode == premiumCode) {
            saveActivationInfo(inputCode, -1);  // 高级版，无限制
            QMessageBox::information(nullptr, "激活成功", "已激活为高级版，无使用期限限制。");
        } else {
            QMessageBox::warning(nullptr, "激活失败", "激活码无效，请检查后重试。");
            exit(0);  // 结束程序
        }
    }
}

void MainForm::closePort() {
//    if (m_serialPort) {
//        if (m_serialPort->isOpen()) {
//            std::vector<unsigned short> channelIdArray;
//            channelIdArray.push_back(RedLampChannel);
//            channelIdArray.push_back(GreenLampChannel);
//            channelIdArray.push_back(BlueLampChannel);
//            std::string data = utils->getTurnOffManyCommand(0, channelIdArray);

//            QString sendData = QString::fromStdString(data);
//            QByteArray ba = QByteArray::fromHex(sendData.toLatin1());

//            // Attempt to write data to the port
//            if (m_serialPort->write(ba) == -1) {
//                qDebug() << "Failed to write to the serial port.";
//            }

//            // Close the port
////            m_serialPort->close();
//            qDebug() << "Serial port closed successfully.";
//        } else {
//            qDebug() << "Serial port is not open.";
//        }
//    } else {
//        qDebug() << "Serial port object is null.";
//    }

    QTimer::singleShot(500, this, &MainForm::quitApplication);
}
void MainForm::quitApplication() {
    qApp->quit();
}
