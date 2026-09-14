#include "systemmanagewindow.h"
#include "ui_systemmanagewindow.h"
#include "recordersettingform.h"
#include "workstationsetform.h"
#include "deptmanagesetform.h"
#include "loggerqueryform.h"
#include <QFont>

int INDEX_RECORDER_FORM = 0;
int INDEX_WORKSTATION_FORM = 1;
int INDEX_DEPT_FORM = 2;
int INDEX_USER_FORM = 3;
int INDEX_LOG_FORM = 4;

namespace {

void applyPortraitSystemLayout(Ui::SystemManageWindow *ui, QWidget *page)
{
    const int width = page->width();
    const int height = page->height();
    const int margin = 28;
    const int sidebarWidth = 150;
    const int sidebarGap = 18;
    const int cardHeight = 68;

    ui->label->setGeometry(margin, 22, 420, 76);
    ui->label->setText(QString::fromUtf8("系统管理"));
    ui->label->setStyleSheet("QLabel { color: white; font: 24pt 'Sans Serif'; font-weight: 600; }");
    ui->pushButtonBack->setGeometry(width - 154, 20, 118, 58);
    ui->pushButtonBack->setText(QString::fromUtf8("返回"));
    ui->pushButtonBack->setStyleSheet(
                "QPushButton { color: white; border: 1px solid rgba(255,255,255,150); "
                "border-radius: 8px; background: #196caf; font: 15pt 'Sans Serif'; }");

    // Keep the navigation order identical to the reference layout: recorder,
    // department, log, workstation, and user management.
    QPushButton *buttons[] = {
        ui->pushButtonRecorderSet, ui->pushButtonDeptManage,
        ui->pushButtonLogQuery, ui->pushButtonWorkstationSet,
        ui->pushButtonUserManage
    };
    const QString cardStyle =
            "QPushButton { color: white; border: none; border-radius: 12px; "
            "background: #1f76bd; font: 18pt 'Sans Serif'; font-weight: 600; }"
            "QPushButton:hover { background: #2d8bd7; }"
            "QPushButton:pressed { background: #13578f; }";
    for (int i = 0; i < 5; ++i) {
        buttons[i]->setGeometry(margin, 122 + i * (cardHeight + sidebarGap), sidebarWidth, cardHeight);
        buttons[i]->setStyleSheet(cardStyle);
        buttons[i]->setFont(QFont("Sans Serif", 16, QFont::DemiBold));
    }
    ui->pushButtonRecorderSet->setText(QString::fromUtf8("执法仪设置"));
    ui->pushButtonWorkstationSet->setText(QString::fromUtf8("采集站设置"));
    ui->pushButtonDeptManage->setText(QString::fromUtf8("部门管理"));
    ui->pushButtonUserManage->setText(QString::fromUtf8("用户管理"));
    ui->pushButtonLogQuery->setText(QString::fromUtf8("日志查询"));

    ui->stackedWidget->setGeometry(margin + sidebarWidth + 10, 122,
                                   width - (margin + sidebarWidth + 28) - margin,
                                   height - 158);
    ui->stackedWidget->raise();
    ui->label->raise();
    ui->pushButtonBack->raise();
    for (int i = 0; i < 5; ++i) {
        buttons[i]->raise();
    }
}

}


SystemManageWindow::SystemManageWindow(QString userid,QString roleId,MySqlLite *sqltie,NetworkUtility *net,QString myipv4Address,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SystemManageWindow),
    userForm(nullptr)
{
    ui->setupUi(this);
    // The legacy background artwork is 1080 pixels tall.  Qt tiled it a
    // second time in portrait mode, creating an unwanted blue bar midway
    // down the settings page.  Paint it once at the top only.
    setStyleSheet("QMainWindow { background-color: #eef7fb; "
                  "background-image: url(:/image/asm8315.png); "
                  "background-repeat: no-repeat; background-position: top left; }");
    this->getSystemIPV4Label = myipv4Address;
    mysql = sqltie;
    mynet = net;
    this->userId = userid;
    this->roleId = roleId;

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFixedSize(this->width(),this->height());
    showFullScreen();

    applyPortraitSystemLayout(ui, this);

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



