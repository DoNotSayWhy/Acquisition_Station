#include "usermanagesetform.h"
#include "ui_usermanagesetform.h"
#include <qcheckbox.h>
#include <QFileDialog>
#include <QMessageBox>
#include "userinfodialog.h"
#include "xlsxdocument.h"

#define IMPORT_MAX_COUNT 1000
QString columnA1Name = "PolNo";
QString columnB1Name = "DevNo";
QString columnC1Name = "Name";
QString columnD1Name = "DepName";
QString columnE1Name = "Remark";
QString columnF1Name = "CreateDate";

UserManageSetForm::UserManageSetForm(QString userid, QString roleId, MySqlLite *sqltie,NetworkUtility *net, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserManageSetForm)
{
    ui->setupUi(this);
    mysql = sqltie;
    mynet = net;
    this->userId = userid;
    this->roleId = roleId;

    ui->btntestpushButton->setVisible(false);


    ui->tableWidget->setColumnCount(11);
    ui->tableWidget->setHorizontalHeaderLabels({" ","人员编号", "设备编号", "部门", "姓名", "性别","人脸状态","指纹状态", "用户状态", "添加时间","角色"});


    int _w_cum = 36;

    ui->tableWidget->setColumnWidth(0, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(1, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(2, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(3, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(4, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(5, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(6, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(7, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(8, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(9, 165 + _w_cum);
    ui->tableWidget->setColumnWidth(10, 165 + _w_cum);


    ui->tableWidget->setColumnHidden(6,true);
    ui->tableWidget->hideColumn(7);


    int rowHeight = 100;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        ui->tableWidget->setRowHeight(i, rowHeight);
    }


     ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);


     ui->tableWidget->setStyleSheet( "QScrollBar:vertical { width: 40px; }");

     ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
     ui->tableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);


    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QObject::connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &UserManageSetForm::onUserItemSelectionChanged);
    QObject::connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &UserManageSetForm::onUserItemClickChanged);


    disconnect(mysql,&MySqlLite::department_successTOUSER,this,&UserManageSetForm::gett_department);

    connect(this, &UserManageSetForm::userSelectt_userinfoALLTOMSQ, mysql, &MySqlLite::MSQSelectt_userinfoALLTOUSER);
    connect(mysql, &MySqlLite::gett_userinfo_success, this, &UserManageSetForm::showTableWidgetData);
    connect(this, &UserManageSetForm::userGetDeptForMSQ, mysql, &MySqlLite::MSQSelectAllDeptManageTOUSER);
    connect(mysql,&MySqlLite::department_successTOUSER,this,&UserManageSetForm::gett_department,Qt::UniqueConnection);
    connect(this,&UserManageSetForm::userInsertToMSQ,mysql,&MySqlLite::MSQInsertt_userinfoToUSER);
    connect(this, &UserManageSetForm::userGetUpdateDeptForMSQ, mysql, &MySqlLite::MSQSelectAllDeptManageTOUSER);
    connect(this,&UserManageSetForm::userupdatet_userInfoToMSQ,mysql,&MySqlLite::MSQupdatet_userInfoToUSER);
    connect(this,&UserManageSetForm::USERdelett_userinfo_byidtoMSQ,mysql,&MySqlLite::MSQRdelett_userinfo_byidtoUSER);
    connect(mysql,SIGNAL(mysqlUserInfo_ClientEditTONET(const QVariantMap &)),mynet,SLOT(netWorkClientEdit(const QVariantMap &)));
    connect(mysql,&MySqlLite::mysqlUserInfo_ClientDeleteTONET,mynet,&NetworkUtility::netWorkClientDelete);
    connect(this,&UserManageSetForm::userListInsertToMSQ,mysql,&MySqlLite::BatchInsertt_userinfoToUSER);
    connect(mysql,SIGNAL(batchInsertUserInfoListResult(int)),this,SLOT(batchImportUserInfoResult(int)));


    if(roleId.toInt() != 1){
        ui->addpushButton->setVisible(false);
        ui->addpushButton->setEnabled(false);
        ui->updatepushButton->setVisible(false);
        ui->updatepushButton->setEnabled(false);
        ui->deletepushButton->setVisible(false);
        ui->deletepushButton->setEnabled(false);

//        emit userSelectt_departmentAllTOMSQ( userid,  roleId);
    }

    emit userSelectt_userinfoALLTOMSQ(userid, roleId,"","");
}
void UserManageSetForm::gett_department(QList<DepartmentInfo> departments,int stauts){

    getDepartmentData = departments;
    if(0 == stauts){
    setUserDialog(false) ;
    }else if(1 == stauts){
    setUserDialog(true) ;
    }
    disconnect(mysql,&MySqlLite::department_successTOUSER,this,&UserManageSetForm::gett_department);
}


void UserManageSetForm::showTableWidgetData(QList<NewUserInfo> userInfoData) {
    getUserInfoData = userInfoData;
    checkBoxes.clear();


    QTableWidget *tableWidget = ui->tableWidget;
    tableWidget->clearContents();

    tableWidget->setRowCount(userInfoData.size());


    for (int i = 0; i < userInfoData.size(); ++i) {

        QCheckBox *checkBox = new QCheckBox();
        tableWidget->setCellWidget(i, 0, checkBox);
        checkBoxes.append(checkBox);

        tableWidget->setItem(i, 1, new QTableWidgetItem(userInfoData[i].F_UserNo));
        tableWidget->setItem(i, 2, new QTableWidgetItem(userInfoData[i].F_DeviceNo));
        tableWidget->setItem(i, 3, new QTableWidgetItem(userInfoData[i].f_name));
        tableWidget->setItem(i, 4, new QTableWidgetItem(userInfoData[i].F_UserName));
        tableWidget->setItem(i, 5, new QTableWidgetItem(userInfoData[i].F_Gender));
//        tableWidget->setItem(i, 5, new QTableWidgetItem(userInfoData[i].F_Status));
//        tableWidget->setItem(i, 6, new QTableWidgetItem(userInfoData[i].userType));
        tableWidget->setItem(i, 8, new QTableWidgetItem(userInfoData[i].F_Status));
        tableWidget->setItem(i, 9, new QTableWidgetItem(userInfoData[i].create_time));
        tableWidget->setItem(i, 10, new QTableWidgetItem(userInfoData[i].F_RoleId));
    }


    QLabel *totalLabel = ui->labelsum;
    totalLabel->setText(QString("总量: %1").arg(userInfoData.size()));
}

UserManageSetForm::~UserManageSetForm()
{
    disconnect(mysql, &MySqlLite::department_successTOUSER, this, &UserManageSetForm::gett_department);
    delete ui;
}

void UserManageSetForm::setUserFormSystemIPV4(QString systemIPV4)
{
    this->getUserFormSystemIPV4 = systemIPV4;
    qDebug()<<"UserManageSetForm::setUserFormSystemIPV4 :"<<getUserFormSystemIPV4;
}

void UserManageSetForm::on_addpushButton_clicked()
{
    if(getDepartmentData.isEmpty()){
       emit userGetDeptForMSQ( userId,  roleId,0);
        disconnect(mysql,&MySqlLite::department_successTOUSER,this,&UserManageSetForm::gett_department);
    }else{
        setUserDialog(false) ;
    }

}

void UserManageSetForm::on_updatepushButton_clicked()
{
    if(getDepartmentData.isEmpty()){
        emit userGetDeptForMSQ( userId,  roleId,1);
        disconnect(mysql,&MySqlLite::department_successTOUSER,this,&UserManageSetForm::gett_department);
    }else{

        setUserDialog(true) ;
    }

}
void UserManageSetForm::setUserDialog(bool isUpdate) {
    NewUserInfo userNull;
    UserInfoDialog *dialog = nullptr;

    if (isUpdate) {
        int checkedCount = 0;
        int checkedId = -1;

        for (int i = 0; i < checkBoxes.size(); ++i) {
            if (checkBoxes[i]->isChecked()) {
                checkedCount++;
                checkedId = i;
            }
        }

        if (checkedCount == 0) {
            QMessageBox::warning(this, "警告", "请至少选择一条数据！");
            return;
        } else if (checkedCount > 1) {
            QMessageBox::warning(this, "警告", "只能选择一条数据进行修改！");
            return;
        } else {
            userNull = getUserInfoData[checkedId];
        }
    }

    dialog = new UserInfoDialog(isUpdate ? "编辑用户信息" : "添加用户信息", userNull, getDepartmentData, mysql, this);

    if (dialog->exec() == QDialog::Accepted) {
        QMap<QString, QVariant> userInfoMap;
       userInfoMap["F_WorkStationIp"]= getUserFormSystemIPV4;

        if (isUpdate) {
            userInfoMap["userId"] = userNull.id;
        }

        userInfoMap["userNo"] = dialog->getUserNo();
        userInfoMap["deviceNo"] = dialog->getDeviceNo();
        userInfoMap["department"] = dialog->getDepartment();
        userInfoMap["userName"] = dialog->getUserName();

        // Handle user status
        QString userStatus = dialog->getUserStatus();
        userInfoMap["statusValue"] = (userStatus == "启用") ? "1" : "0";

        // Handle user role
        QString role = dialog->getRole();
        userInfoMap["roleValue"] = (role == "超级管理员") ? 1 : (role == "普通管理员") ? 2 : (role == "普通用户") ? 3 : -1;

        userInfoMap["password"] = dialog->getPassWD();
        userInfoMap["passwordConfirm"] = dialog->getPassWDTwo();

        // Find department id
        int departmentId = -1;
        for (const DepartmentInfo& info : getDepartmentData) {
            if (info.f_name == userInfoMap["department"].toString()) {
                departmentId = info.id.toInt();
                break;
            }
        }
        userInfoMap["departmentId"] = departmentId;

        // Emit necessary signal based on whether it is an update or add
        if (isUpdate) {
            bool isDataChanged = false; // Check if data is changed
            if (userNull.F_UserNo != userInfoMap["userNo"].toString() ||
                userNull.F_DeviceNo != userInfoMap["deviceNo"].toString() ||
                userNull.f_name != userInfoMap["department"].toString() ||
                userNull.F_UserName != userInfoMap["userName"].toString() ||
                userNull.F_RoleId != role ||
                userNull.F_Status != userStatus ||
                userNull.F_Password != userInfoMap["password"].toString())
            {
                isDataChanged = true;
            }

            if (isDataChanged) {

                emit userupdatet_userInfoToMSQ(userInfoMap);

            } else {
                qDebug() << "用户信息未改变，不必发送到数据库。";
            }
        } else {
            connect(mysql,SIGNAL(mysqlClientAdd(const QVariantMap &)),mynet,SLOT(netWorkClientAdd(const QVariantMap &)), Qt::QueuedConnection);

            emit userInsertToMSQ(userInfoMap);

        }


        emit userSelectt_userinfoALLTOMSQ(this->userId, this->roleId,"","");

    }
    if(dialog){
        delete dialog;
        dialog = nullptr;
    }
}


void UserManageSetForm::on_deletepushButton_clicked()
{
    QList<QMap<QString, QString>> userInfoList;

    for (int i = 0; i < checkBoxes.size(); ++i) {
        if (checkBoxes[i]->isChecked()) {
            QMap<QString, QString> userInfo;
            userInfo["id"] = getUserInfoData[i].id;
            userInfo["F_UserNo"] = getUserInfoData[i].F_UserNo;
            if("888888" == getUserInfoData[i].F_UserNo){
                qDebug() << "888888,默认不能删除";
                return;
            }
            userInfoList.append(userInfo);
        }
    }
    if(!userInfoList.isEmpty()){
    emit USERdelett_userinfo_byidtoMSQ(getUserFormSystemIPV4,userInfoList);

        for (const auto &user : userInfoList) {
            QString userIdToDelete = user["id"];

            // 遍历 getUserInfoData，删除对应的用户
            for (int j = 0; j < getUserInfoData.size(); ) {
                if (getUserInfoData[j].id == userIdToDelete) {
                    getUserInfoData.removeAt(j); // 删除当前用户
                } else {
                    ++j; // 仅在未删除时增加索引
                }
            }
        }

        // 重新展示当前用户数据
        showTableWidgetData(getUserInfoData);
    }
}

QString UserManageSetForm::openExcelFileDialog(QWidget *parent) {
    QString fileName = QFileDialog::getOpenFileName(parent, QStringLiteral("打开Excel文件"),
                                                    QString(),
                                                    QStringLiteral("Excel Files (*.xlsx *.xls)"));
    return fileName;
}

void UserManageSetForm::on_btnBatchImport_clicked(){
    QString path = openExcelFileDialog(this);
    QXlsx::Document xlsx(path);

    //verify the file data.
    QVariant columnA1 = xlsx.read("A1");
    QVariant columnB1 = xlsx.read("B1");
    QVariant columnC1 = xlsx.read("C1");
    QVariant columnD1 = xlsx.read("D1");
    QVariant columnE1 = xlsx.read("E1");
    QVariant columnF1 = xlsx.read("F1");

    QList<ImportUserInfo> tmpInfoList;

    if(columnA1Name == columnA1.toString().remove(QChar::Space)
        && columnB1Name == columnB1.toString().remove(QChar::Space)
            && columnC1Name == columnC1.toString().remove(QChar::Space)
            && columnD1Name == columnD1.toString().remove(QChar::Space)
            && columnE1Name == columnE1.toString().remove(QChar::Space)
            && columnF1Name == columnF1.toString().remove(QChar::Space)
            ){
        //data right.

        int index = 2;
        bool willStop = false;
        do{
            struct ImportUserInfo tmpInfo;

            QString columnANameTmp = "A" + QString::number(index);
            QString columnBNameTmp = "B" + QString::number(index);
            QString columnCNameTmp = "C" + QString::number(index);
            QString columnDNameTmp = "D" + QString::number(index);
            QString columnENameTmp = "E" + QString::number(index);
            QString columnFNameTmp = "F" + QString::number(index);


            tmpInfo.PolNo = xlsx.read(columnANameTmp).toString();
            tmpInfo.DevNo = xlsx.read(columnBNameTmp).toString();
            tmpInfo.Name = xlsx.read(columnCNameTmp).toString();
            tmpInfo.DepName = xlsx.read(columnDNameTmp).toString();
            tmpInfo.Remark = xlsx.read(columnENameTmp).toString();
            tmpInfo.CreateDate = xlsx.read(columnFNameTmp).toString();

            tmpInfoList.append(tmpInfo);
            if(tmpInfo.PolNo.isNull()
                    || tmpInfo.PolNo == ""
                    || index > IMPORT_MAX_COUNT){
                willStop = true;
            }

            index ++;

        } while (!willStop);

    }else{
        QMessageBox::warning(this,"提示","文件格式不识别，无法导入！");
        return;
    }

    if(tmpInfoList.size() > 0){
        //to do insert.
        qDebug() << "QStringList.size() = " << tmpInfoList.size() ;
        setUserInfoList(tmpInfoList);
    }else{
        QMessageBox::warning(this,"提示","用户已存在，无需导入！");
    }

}


void UserManageSetForm::setUserInfoList(QList<ImportUserInfo> userInfoList) {
    if (userInfoList.isEmpty()) {
        return;
    }

    QList<QMap<QString, QVariant>> userInfoMapList;
    QSet<QString> existingUserNos;

    for (const auto &userInfo : userInfoList) {
        if (userInfo.PolNo.isEmpty()) {
            continue;
        }
        NewUserInfo existingUser = mysql->getUserInfoByUserNo(userInfo.PolNo);
        if (!existingUser.id.isEmpty() || !existingUser.F_UserName.isEmpty()) {
            continue; // 用户已存在
        }

        if (existingUserNos.contains(userInfo.PolNo)) {
            continue; // 跳过重复的用户
        }

        QMap<QString, QVariant> userInfoMap;
        userInfoMap["userNo"] = userInfo.PolNo;
        userInfoMap["deviceNo"] = userInfo.DevNo;
        userInfoMap["department"] = userInfo.DepName;
        userInfoMap["userName"] = userInfo.Name;
        userInfoMap["statusValue"] = "1"; // 启用
        userInfoMap["roleValue"] = 3; // 普通用户
        userInfoMap["password"] = "";
        userInfoMap["passwordConfirm"] = "";

        int departmentId = 1;
        QString department = userInfoMap["department"].toString().remove(QChar::Space);
        if (!department.isEmpty()) {
            departmentId = mysql->insertOrUpdateDept("", department);
        }
        userInfoMap["departmentId"] = departmentId;
        userInfoMap["F_WorkStationIp"] = getUserFormSystemIPV4;

        userInfoMapList.append(userInfoMap);
        existingUserNos.insert(userInfo.PolNo); // 新增 userNo 到集合中
    }

    if (userInfoMapList.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户已存在，无需导入！");
        return;
    }
    emit userListInsertToMSQ(userInfoMapList);
}



void UserManageSetForm::batchImportUserInfoResult(int count){

    emit userSelectt_userinfoALLTOMSQ(this->userId, this->roleId,"","");
    if(count > 0){
        QString text = QString("导入 %1 条数据！").arg(QString::number(count));
        QMessageBox::information(this,"提示",text);
    }
}


void UserManageSetForm::on_pushButtonOutport_clicked(){
     getUserInfoData;
    QString saveFileName = QFileDialog::getSaveFileName(
        this,
        tr("保存Excel文件模板"),
        "",
        tr("Excel Files (*.xlsx)")
    );

    if(!saveFileName.isEmpty()){
        QXlsx::Document xlsx;
        xlsx.write("A1", columnA1Name);
        xlsx.write("B1", columnB1Name);
        xlsx.write("C1", columnC1Name);
        xlsx.write("D1", columnD1Name);
        xlsx.write("E1", columnE1Name);
        xlsx.write("F1", columnF1Name);

        // 填充数据
        int row = 2; // 从第二行开始填充数据
        for (const NewUserInfo &userInfo : getUserInfoData) {
            xlsx.write(row, 1, userInfo.F_UserNo);
            xlsx.write(row, 2, userInfo.F_DeviceNo);
            xlsx.write(row, 3, userInfo.F_UserName);
            xlsx.write(row, 4, userInfo.departmentName);
            xlsx.write(row, 5, userInfo.F_Remark);
            xlsx.write(row, 6, userInfo.create_time);
            row++; // 移动到下一行
        }

        if(!saveFileName.endsWith("xlsx")){
            saveFileName += ".xlsx";
        }
        xlsx.saveAs(saveFileName);
    }
}



void UserManageSetForm::on_btnCheckBox_clicked(bool checked)
{

    if(checked){

        ui->tableWidget->selectAll();

        for(int i = 0; i < ui->tableWidget->rowCount() ; i++){

            //QCheckBox *pTmpCheckBox = getCellCheckBox(m);

            QWidget* pWidget = 0;
            pWidget = ui->tableWidget->cellWidget(i,0);

            QCheckBox *pTmpCheckBox = static_cast<QCheckBox *>(pWidget);

            if (pTmpCheckBox == nullptr)
            {
                continue;
            }

            pTmpCheckBox->setChecked(Qt::Checked);


        }




    }else{

        ui->tableWidget->clearSelection();

        for(int i = 0; i < ui->tableWidget->rowCount(); i++){

            QWidget* pWidget = 0;
            pWidget = ui->tableWidget->cellWidget(i,0);

            QCheckBox *pTmpCheckBox = static_cast<QCheckBox *>(pWidget);

            if (pTmpCheckBox == nullptr)
            {
                continue;
            }

            pTmpCheckBox->setChecked(Qt::Unchecked);


        }


    }
}


void UserManageSetForm::on_btntestpushButton_clicked()
{

    /*获取选中的列表里的所有条目*/
   QList<QTableWidgetItem*> list= ui->tableWidget->selectedItems();
   if(list.count() <= 0)
   {
       QMessageBox::warning(this, tr("删除作业要素列表"),
       tr("请选择需要删除的列表."),
       QMessageBox::Ok);
       return;
   }

   /*从列表中依次移除条目*/
   for(int i=0;i<list.count();i++)
   {
       int row = ui->tableWidget->row(list.at(i));
       if(row != 0)
       {
          // ui->tableWidget->removeRow(row);
       }

        ui->tableWidget->removeRow(row);
   }

}



void UserManageSetForm::onUserItemSelectionChanged(){

    /*获取选中的列表里的所有条目*/
   QList<QTableWidgetItem*> list = ui->tableWidget->selectedItems();


   // 存储行数
   QSet<int> _selectedRows;

   // 遍历选中的项目，获取它们的行数，并存储在selectedRows中
   for (QTableWidgetItem* item : list) {
       int row = item->row();
       _selectedRows.insert(row);
   }


   for (int _row : _selectedRows) {

       QWidget* pWidget = 0;
       pWidget = ui->tableWidget->cellWidget(_row,0);

       QCheckBox *pTmpCheckBox = static_cast<QCheckBox *>(pWidget);

       if (pTmpCheckBox == nullptr)
       {
           continue;
       }

       pTmpCheckBox->setChecked(Qt::Checked);

   }


   qDebug() << list.count() << " \r\n";




}


void UserManageSetForm::onUserItemClickChanged(QTableWidgetItem *item){


    int row = item->row(); // 获取行号
    int column = item->column(); // 获取列号
    QString itemText = item->text(); // 获取单元格文本

    QWidget* pWidget = 0;
    pWidget = ui->tableWidget->cellWidget(row,0);

    QCheckBox *pTmpCheckBox = static_cast<QCheckBox *>(pWidget);

    if (pTmpCheckBox == nullptr)
    {
        return;
    }


    int _stat = pTmpCheckBox->checkState() ;
    if( _stat == Qt::Unchecked){

        pTmpCheckBox->setChecked(Qt::Checked);

    }else if (_stat == Qt::Checked) {

        pTmpCheckBox->setChecked(Qt::Unchecked);
    }



}



void UserManageSetForm::on_pushButton_clicked()
{
    // ui->lineEdit_dev->text();
    // ui->lineEdit_user->text();
    QString userNumber = ui->lineEdit_user->text() ;
    QString devNumber = ui->lineEdit_dev->text() ;


    emit userSelectt_userinfoALLTOMSQ(this->userId, this->roleId, userNumber , devNumber );

}




void UserManageSetForm::on_pushButton_2_clicked()
{
     ui->lineEdit_dev->setText("");
     ui->lineEdit_user->setText("");

     emit userSelectt_userinfoALLTOMSQ(this->userId, this->roleId, ""  ,  "" );

}
