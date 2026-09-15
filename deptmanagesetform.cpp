#include "deptmanagesetform.h"
#include "ui_deptmanagesetform.h"
#include "uiprofile.h"
#include <qinputdialog.h>

DeptManageSetForm::DeptManageSetForm(QString userid,QString roleId,MySqlLite *sqltie,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeptManageSetForm)
{
    ui->setupUi(this);
    UiProfile::apply(this, "deptmanagesetform");
    if(roleId.toInt() != 1){
    ui->addpushButton->setVisible(false);
    ui->addpushButton->setEnabled(false);
    ui->updatepushButton->setVisible(false);
    ui->updatepushButton->setEnabled(false);
    ui->deletepushButton->setVisible(false);
    ui->deletepushButton->setEnabled(false);
    }
    mysql = sqltie;
    this->userId = userid;
    this->roleId = roleId;
    // 行为：点击树中的某个部门
    connect(ui->deptreeWidget, &QTreeWidget::itemClicked, this, &DeptManageSetForm::onItemClicked);
    connect(ui->addpushButton, &QPushButton::clicked, this, &DeptManageSetForm::onAddButtonClicked);
    connect(ui->updatepushButton, &QPushButton::clicked, this, &DeptManageSetForm::onUpdateSubDepartment);
    connect(this, &DeptManageSetForm::depSelectAllTOMSQ, mysql, &MySqlLite::MSQSelectAllDeptManageTODEP);
    connect(mysql,&MySqlLite::gett_department_success,this,&DeptManageSetForm::setupTreeView);
    connect(this,&DeptManageSetForm::depInsertt_departmentTOMSQ,mysql,&MySqlLite::MSQinsertDeptManageTODEP);
    connect(this,&DeptManageSetForm::depUpdatet_departmentTOMSQ,mysql,&MySqlLite::MSQUpdatet_departmentTODEP);
    connect(this,&DeptManageSetForm::depDeletet_departmentTOMSQ,mysql,&MySqlLite::MSQDeletet_departmentTODEP);
    connect(ui->deletepushButton, &QPushButton::clicked, this, &DeptManageSetForm::onDeleteSubDepartment);

    emit depSelectAllTOMSQ(userId,roleId);

}

DeptManageSetForm::~DeptManageSetForm()
{
    delete ui;
}

void DeptManageSetForm::onItemClicked(QTreeWidgetItem *item, int column) {
    // 记录当前选中部门
    selectedDepartmentId = item->data(0, Qt::UserRole).toString();
}

void DeptManageSetForm::onAddButtonClicked() {
    addSubDepartment(selectedDepartmentId);
}
void DeptManageSetForm::onUpdateSubDepartment() {
    if(!selectedDepartmentId.isEmpty()){
    updateSubDepartment(selectedDepartmentId);
    }else{
        QMessageBox::warning(this, "警告", "未选中部门！");
    }
}
void DeptManageSetForm::onDeleteSubDepartment(){
    if(!selectedDepartmentId.isEmpty()){
    deleteSubDepartment(selectedDepartmentId);
    }else{
        QMessageBox::warning(this, "警告", "未选中部门！");
    }



    emit depSelectAllTOMSQ(userId,roleId);


}

void DeptManageSetForm::setupTreeView(QList<DepartmentInfo> departmentInfos) {
    qDebug() << "DeptManageSetForm::setupTreeView Start";

    if (departmentInfos.isEmpty()) {
        qDebug() << "No departments to display.";
        return;
    }

    getDepartmentInfos = departmentInfos;
    ui->deptreeWidget->clear();
    QMap<QString, QTreeWidgetItem*> itemMap;


    for (const DepartmentInfo& dept : departmentInfos) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, dept.f_name);
        item->setData(0, Qt::UserRole, dept.id);
        itemMap.insert(dept.id, item);
//        qDebug() << "Created item:" << dept.f_name << " with id:" << dept.id<<"with parentId:"<<dept.f_parentId;
    }


    for (const DepartmentInfo& dept : departmentInfos) {
        QTreeWidgetItem *item = itemMap.value(dept.id);
//        qDebug() << "Processing item:" << dept.f_name << " with parentId:" << dept.f_parentId; // Debug info

        if (dept.f_parentId.toInt() == 0) {

            ui->deptreeWidget->addTopLevelItem(item);
        } else {

            QTreeWidgetItem *parentItem = itemMap.value(dept.f_parentId, nullptr);
            if (parentItem) {
                parentItem->addChild(item);
//                qDebug() << "Added child:" << dept.f_name << " to parent:" << parentItem->text(0);
            } else {
                qDebug() << "Parent item not found for child:" << dept.f_name;
            }
        }
    }


    ui->deptreeWidget->expandAll();
}

void DeptManageSetForm::addSubDepartment(QString parentId) {
    QDialog dialog(this);
    dialog.setWindowTitle("添加子级部门");
    dialog.setFixedSize(300, 200);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *parentLabel = new QLabel("父级部门名:");
    QLineEdit *parentNameEdit = new QLineEdit();
    parentNameEdit->setReadOnly(true);

    if (!parentId.isEmpty()) {
        for (const DepartmentInfo& dept : getDepartmentInfos) {
            if (dept.id == parentId) {
                parentNameEdit->setText(dept.f_name);
            }
        }
    } else {
        parentNameEdit->setText("");
    }

    QLabel *nameLabel = new QLabel("部门名称:");
    QLineEdit *nameEdit = new QLineEdit();

    layout->addWidget(parentLabel);
    layout->addWidget(parentNameEdit);
    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);


    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        QString newDepartmentName = nameEdit->text();
        if (!newDepartmentName.isEmpty()) {

            emit depInsertt_departmentTOMSQ(parentId,newDepartmentName);
            dialog.accept();
        } else {
            QMessageBox::warning(this, "警告", "部门名称不能为空！");
        }
    });


    QObject::connect(cancelButton, &QPushButton::clicked, [&]() {
        dialog.reject();
    });

    dialog.exec();


    emit depSelectAllTOMSQ(userId,roleId);




}

void DeptManageSetForm::updateSubDepartment(QString departmentId) {

    QDialog dialog(this);
    dialog.setWindowTitle("修改部门");
    dialog.setFixedSize(300, 200); // 设置弹窗宽高

    QVBoxLayout *layout = new QVBoxLayout(&dialog);


    QLabel *parentLabel = new QLabel("父级部门名:");
    QLineEdit *parentNameEdit = new QLineEdit();

    QLabel *nameLabel = new QLabel("部门名称:");
    QLineEdit *nameEdit = new QLineEdit(); // 可编辑的部门名称输入框


    if (!departmentId.isEmpty()) {
        loadSelectedDepartment(departmentId, parentNameEdit, nameEdit);
    } else {
        resetFields(parentNameEdit, nameEdit);
    }

    layout->addWidget(parentLabel);
    layout->addWidget(parentNameEdit);
    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    QPushButton *cancelButton = new QPushButton("取消");

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);


    connect(okButton, &QPushButton::clicked, [&]() { handleOkButtonClick(nameEdit, parentNameEdit, dialog); });
    connect(cancelButton, &QPushButton::clicked, [&]() { dialog.reject(); });

    dialog.exec();



    emit depSelectAllTOMSQ(userId,roleId);



}

void DeptManageSetForm::loadSelectedDepartment(const QString &departmentId, QLineEdit *parentNameEdit, QLineEdit *nameEdit) {
    for (const DepartmentInfo &dept : getDepartmentInfos) {
        if (dept.id == departmentId) {
            department = dept.f_name;
            nameEdit->setText(dept.f_name); // 设置部门名称

            // 查找并设置父级部门名称
            for (const DepartmentInfo &deptFather : getDepartmentInfos) {
                if (dept.f_parentId == deptFather.id) {
                    departmentParent = deptFather.f_name;
                    parentNameEdit->setText(deptFather.f_name);
                }
            }
            return; // 找到部门后，退出函数
        }
    }
}

void DeptManageSetForm::resetFields(QLineEdit *parentNameEdit, QLineEdit *nameEdit) {
    parentNameEdit->clear();
    parentNameEdit->setReadOnly(true);
    nameEdit->clear();
    nameEdit->setReadOnly(true);
}

void DeptManageSetForm::handleOkButtonClick(QLineEdit *nameEdit, QLineEdit *parentNameEdit, QDialog &dialog) {
    QString newDepartmentName = nameEdit->text();
    QString newParentDepartmentName = parentNameEdit->text();

    // 确认输入有效性
    if (newDepartmentName.isEmpty() || newParentDepartmentName.isEmpty()) {
        QMessageBox::warning(this, "警告", "部门名称不能为空！");
        return;
    }

    // 处理名称和父级修改
    if (newDepartmentName != department || newParentDepartmentName != departmentParent) {
        QString deptParentId = findParentDepartmentId(newParentDepartmentName);

        if (deptParentId.isEmpty()) {
            QMessageBox::warning(this, "警告", "父级部门不存在！");
            return;
        }


       emit depUpdatet_departmentTOMSQ(deptParentId, newDepartmentName,selectedDepartmentId);

    }

    dialog.accept(); // 关闭对话框
}

QString DeptManageSetForm::findParentDepartmentId(const QString &parentName) const {
    for (const DepartmentInfo &dept : getDepartmentInfos) {
        if (dept.f_name == parentName) {
            return dept.id;
        }
    }
    return QString(); // 如果没有找到，返回空字符串
}

void DeptManageSetForm::deleteSubDepartment(QString departmentId){
    if("1" == departmentId){
        qDebug() << "默认部门不能删除:" << departmentId;
        return;
    }
    for (const DepartmentInfo &deptSub : getDepartmentInfos) {
        if (deptSub.f_parentId == departmentId) {
            QMessageBox::warning(this, "警告", "存在子级部门不允许删除！");
            return;
        }
    }
    emit depDeletet_departmentTOMSQ(departmentId);
}
