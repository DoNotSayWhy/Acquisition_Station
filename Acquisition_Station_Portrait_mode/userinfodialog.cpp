#include "userinfodialog.h"
#include "ui_userinfodialog.h"
#include "qmessagebox.h"
#include "ziangfingerutility.h"

#define FINGER_PRINT_INDEX 1

UserInfoDialog::UserInfoDialog(QString title,NewUserInfo user,QList<DepartmentInfo> departments,MySqlLite *sqltie,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserInfoDialog),
    face(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(title);
    mysql = sqltie;
    this->getDepartments = departments;
    // 是否开启指纹人脸/指纹
#ifdef USE_FACE_FINGER
    ifopen_face_finger(true);
#else
    ifopen_face_finger(false);
#endif

    // 设置下拉框的固定值  启用 禁用 超级管理员 普通管理员 普通用户
    ui->comboBoxuserstatus->addItem("启用");
    ui->comboBoxuserstatus->addItem("禁用");


    ui->comboBoxrole->addItem("超级管理员");
    ui->comboBoxrole->addItem("普通管理员");
    ui->comboBoxrole->addItem("普通用户");

    // 清空下拉框，以防之前的内容
    ui->comboBoxDepartment->clear();

    // 遍历部门信息列表并添加到下拉框
    for (const DepartmentInfo &department : departments)
    {
        ui->comboBoxDepartment->addItem(department.f_name); // 添加部门名称
    }

    if(title == "编辑用户信息"){
        setUserInfoDialogData(user);
    }

    indexFingerForm = -1;
    indexFaceForm = -1;
    fingerForm = NULL;

#ifdef USE_FACE_FINGER
    initFingerStatus();
#endif
}

UserInfoDialog::~UserInfoDialog()
{
    delete ui;
    if (face) {
        delete face;
    }
}

void UserInfoDialog::setUserInfoDialogData(NewUserInfo user){
    ui->lineEdit_UserNo->setText(user.F_UserNo);
    ui->lineEdit_DeviceNo->setText(user.F_DeviceNo);
    ui->lineEdit_UserName->setText(user.F_UserName);
    ui->lineEdit_UserNo->setReadOnly(true);
}

void UserInfoDialog::ifopen_face_finger(bool isopen)
{
    ui->pushButtonface->setEnabled(isopen);  // 禁用按钮
    ui->pushButtonfinger->setEnabled(isopen);  // 禁用按钮
}


QString UserInfoDialog::getUserNo() const {
    return ui->lineEdit_UserNo->text();
}

QString UserInfoDialog::getDeviceNo() const {
    return ui->lineEdit_DeviceNo->text();
}

QString UserInfoDialog::getDepartment() const {
    return ui->comboBoxDepartment->currentText();
}

QString UserInfoDialog::getUserName() const {
    return ui->lineEdit_UserName->text();
}

QString UserInfoDialog::getFaceStatus() const {
    return ui->pushButtonface->text();
}

QString UserInfoDialog::getFingerprintStatus() const {
    return ui->pushButtonfinger->text();
}

QString UserInfoDialog::getUserStatus() const {
    return ui->comboBoxuserstatus->currentText();
}
QString UserInfoDialog::getRole() const {
    return ui->comboBoxrole->currentText();
}

QString UserInfoDialog::getPassWD() const {
    return ui->lineEditpwd->text();
}
QString UserInfoDialog::getPassWDTwo() const {
    return ui->lineEditpwd_1->text();
}



void UserInfoDialog::on_pushButtonok_clicked()
{
    // 验证用户编号
    if (ui->lineEdit_UserNo->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户编号不能为空！");
        return; // 取消对话框关闭
    }

    // 验证设备编号
    if (ui->lineEdit_DeviceNo->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "设备编号不能为空！");
        return; // 取消对话框关闭
    }

    // 验证部门
    if (ui->comboBoxDepartment->currentText().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择部门！");
        return; // 取消对话框关闭
    }

//    // 验证用户名
//    if (ui->lineEdit_UserName->text().isEmpty()) {
//        QMessageBox::warning(this, "输入错误", "用户名不能为空！");
//        return; // 取消对话框关闭
//    }

//    // 验证人脸状态
//    if (ui->pushButtonface->text().isEmpty()) {
//        QMessageBox::warning(this, "输入错误", "人脸状态不能为空！");
//        return; // 取消对话框关闭
//    }

//    // 验证指纹状态
//    if (ui->pushButtonfinger->text().isEmpty()) {
//        QMessageBox::warning(this, "输入错误", "指纹状态不能为空！");
//        return; // 取消对话框关闭
//    }

    // 验证用户状态
    if (ui->comboBoxuserstatus->currentText().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择用户状态！");
        return; // 取消对话框关闭
    }

    // 验证角色
    if (ui->comboBoxrole->currentText().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择角色！");
        return; // 取消对话框关闭
    }

    // 验证密码
    QString password = ui->lineEditpwd->text();
    QString passwordConfirm = ui->lineEditpwd_1->text();
    if (password.isEmpty()) {
        password = "888888"; // 设置默认值
    }
    if (passwordConfirm.isEmpty()) {
        passwordConfirm = "888888"; // 设置默认值
    }

    // 检查密码是否为空
    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "密码不能为空！");
        return; // 取消对话框关闭
    }

    // 检查两个密码是否匹配
    if (password != passwordConfirm) {
        QMessageBox::warning(this, "输入错误", "两次输入的密码不匹配！");
        return; // 取消对话框关闭
    }

    // 如果所有验证通过，关闭对话框
    accept(); // 这将使 exec() 返回 QDialog::Accepted

}

void UserInfoDialog::on_pushButtoncancel_clicked()
{
    reject();
}

void UserInfoDialog::on_pushButtonfinger_clicked()
{
    if(fingerForm == NULL){
        fingerForm = new FingerPrintForm(mysql,this,ui->lineEdit_UserNo->text());
        indexFingerForm = ui->stackedWidget->addWidget(fingerForm);
        fingerForm->show();
        connect(fingerForm, SIGNAL(clickBack()), this, SLOT(goBackUserInfoDialog()));
        connect(fingerForm,SIGNAL(enrollResult(QString)),this,SLOT(fingerEnrollResult(QString)));

    }
    if(indexFingerForm != -1){
        fingerForm->processFingerFeature();
        ui->stackedWidget->setCurrentIndex(indexFingerForm);
    }
}
// 实现更新标签的方法
void UserInfoDialog::updatefingerLabel(const QString& message) {
    qDebug()<<"updatefingerLabel message"<<message;
    ui->labelfingerstauts->setText(message);
}

void UserInfoDialog::on_pushButtonface_clicked()
{
    // 验证用户编号
    if (ui->lineEdit_UserNo->text().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户编号不能为空！");
        return; // 取消对话框关闭
    }
    if (!face) {
        face = new FaceFormUtility(this,mysql,ui->lineEdit_UserNo->text());
        //ui->stackedWidget->insertWidget(1, face);
        indexFaceForm = ui->stackedWidget->addWidget(face);
        face->startCameraPreview();
        face->FaceEnroll();
        face->show();
        connect(face, SIGNAL(goback()), this, SLOT(goBackUserInfoDialog()));
    }

    if(indexFaceForm != -1){
        ui->stackedWidget->setCurrentIndex(indexFaceForm);
    }
}
void UserInfoDialog::goBackUserInfoDialog(){

    ui->stackedWidget->setCurrentIndex(0);

    if (face) {
        disconnect(face, SIGNAL(goback()), this, SLOT(goBackUserInfoDialog()));

        delete face; // 删除 face 对象
        face = nullptr; // 设置为 nullptr
    }

}

void UserInfoDialog::initFingerStatus(){

    if(!ui->lineEdit_UserNo->text().isEmpty()){
       FingerFeatureData data = mysql->queryFingerFeature(getUserNo());
       if(!data.userNo.isEmpty()){
           //updatefingerLabel
           updatefingerLabel("已采集");
       }else{
           updatefingerLabel("未采集");
       }
    }
}

void UserInfoDialog::fingerEnrollResult(QString userNo){
    if(userNo.isEmpty()){
        return;
    }
    FingerFeatureData data = mysql->queryFingerFeature(userNo);
    if(!data.userNo.isEmpty()){
        //updatefingerLabel
        updatefingerLabel(userNo + "已采集");
    }else{
        updatefingerLabel(userNo + "未采集");
    }
    goBackUserInfoDialog();
}
