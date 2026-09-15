#include "systemmanagewindow.h"
#include "ui_systemmanagewindow.h"
#include "uiprofile.h"
#include "recordersettingform.h"
#include "workstationsetform.h"
#include "deptmanagesetform.h"
#include "loggerqueryform.h"

int INDEX_RECORDER_FORM = 0;
int INDEX_WORKSTATION_FORM = 1;
int INDEX_DEPT_FORM = 2;
int INDEX_USER_FORM = 3;
int INDEX_LOG_FORM = 4;


SystemManageWindow::SystemManageWindow(QString userid,QString roleId,MySqlLite *sqltie,NetworkUtility *net,QString myipv4Address,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SystemManageWindow),
    userForm(nullptr)
{
    ui->setupUi(this);
    UiProfile::apply(this, "systemmanagewindow");
    this->getSystemIPV4Label = myipv4Address;
    mysql = sqltie;
    mynet = net;
    this->userId = userid;
    this->roleId = roleId;

    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(this->width(),this->height());
    showFullScreen();

    initForms();
    initConnectSignals();

    pushButtonRecorderSet();
}

SystemManageWindow::~SystemManageWindow(){
    qDebug()<<"~SystemManageWindow()";
    delete userForm;
    userForm = nullptr;
}

void SystemManageWindow::setSystemIPV4Label(QString iPV4)
{
//    this->getSystemIPV4Label = iPV4;
    qDebug()<<"SystemManageWindow::setUserFormSystemIPV4 :"<<iPV4;
}

void SystemManageWindow::initConnectSignals(){
    connect(ui->pushButtonBack,SIGNAL(clicked()),this,SLOT(pushButtonBackClick()));
    connect(ui->pushButtonRecorderSet,SIGNAL(clicked()),this,SLOT(pushButtonRecorderSet()));
    connect(ui->pushButtonWorkstationSet,SIGNAL(clicked()),this,SLOT(pushButtonWorkstationSet()));
    connect(ui->pushButtonDeptManage,SIGNAL(clicked()),this,SLOT(pushButtonDeptSet()));
    connect(ui->pushButtonUserManage,SIGNAL(clicked()),this,SLOT(pushButtonUserManage()));
    connect(ui->pushButtonLogQuery,SIGNAL(clicked()),this,SLOT(pushButtonLogQuery()));
}

void SystemManageWindow::pushButtonBackClick(){
    this->close();
}


void SystemManageWindow::initForms(){
    RecorderSettingForm *recordSettingForm = new RecorderSettingForm(this);
    ui->stackedWidget->insertWidget(INDEX_RECORDER_FORM, recordSettingForm);

    WorkStationSetForm *workstationForm = new WorkStationSetForm(this);
    ui->stackedWidget->insertWidget(INDEX_WORKSTATION_FORM, workstationForm);

    deptForm = new DeptManageSetForm(userId,roleId,mysql,this);
    ui->stackedWidget->insertWidget(INDEX_DEPT_FORM, deptForm);

//    UserManageSetForm *userForm = new UserManageSetForm(userId,roleId,mysql,mynet,this);
//    userForm->setUserFormSystemIPV4(getSystemIPV4Label);
//    ui->stackedWidget->insertWidget(INDEX_USER_FORM, userForm);
    // 初始化用户管理窗口指针
    userForm = new UserManageSetForm(userId, roleId, mysql, mynet, this);
    userForm->setUserFormSystemIPV4(getSystemIPV4Label);
    ui->stackedWidget->insertWidget(INDEX_USER_FORM, userForm);

    LoggerQueryForm *loggerForm = new LoggerQueryForm(userId,roleId,mysql,this);
    ui->stackedWidget->insertWidget(INDEX_LOG_FORM, loggerForm);

}


void SystemManageWindow::pushButtonRecorderSet(){
    ui->stackedWidget->setCurrentIndex(INDEX_RECORDER_FORM);
}
void SystemManageWindow::pushButtonWorkstationSet(){
    ui->stackedWidget->setCurrentIndex(INDEX_WORKSTATION_FORM);
}
void SystemManageWindow::pushButtonDeptSet(){
//    DeptManageSetForm *deptForm = new DeptManageSetForm(userId,roleId,mysql,this);
//    ui->stackedWidget->insertWidget(INDEX_DEPT_FORM, deptForm);
    ui->stackedWidget->setCurrentIndex(INDEX_DEPT_FORM);
}
void SystemManageWindow::pushButtonUserManage(){
    if (userForm) {
        userForm->deleteLater();
        userForm = nullptr;
    }

    userForm = new UserManageSetForm(userId, roleId, mysql, mynet, this);
    userForm->setUserFormSystemIPV4(getSystemIPV4Label);
    ui->stackedWidget->insertWidget(INDEX_USER_FORM, userForm);

    ui->stackedWidget->setCurrentIndex(INDEX_USER_FORM);
//    ui->stackedWidget->setCurrentIndex(INDEX_USER_FORM);
}
void SystemManageWindow::pushButtonLogQuery(){
    ui->stackedWidget->setCurrentIndex(INDEX_LOG_FORM);
}








