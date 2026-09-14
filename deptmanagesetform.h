#ifndef DEPTMANAGESETFORM_H
#define DEPTMANAGESETFORM_H

#include <QWidget>
#include <QApplication>
#include <QTreeWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QMessageBox>
#include "mysqllite.h"

namespace Ui {
class DeptManageSetForm;
}

class DeptManageSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit DeptManageSetForm(QString userid,QString roleId,MySqlLite *sqltie,QWidget *parent = 0);
    ~DeptManageSetForm();

private slots:

    void onItemClicked(QTreeWidgetItem *item, int column);

    void onAddButtonClicked();

    void onUpdateSubDepartment();

    void setupTreeView(QList<DepartmentInfo>);

    void onDeleteSubDepartment();

    void handleOkButtonClick(QLineEdit *nameEdit, QLineEdit *parentNameEdit, QDialog &dialog);

signals:

    void depSelectAllTOMSQ(QString userid,QString roleId);
    void depInsertt_departmentTOMSQ(QString parentId,QString newDepartmentName);
    void depUpdatet_departmentTOMSQ(QString parentId,QString newDepartmentName,QString departmentId);
    void depDeletet_departmentTOMSQ(QString departmentId);
private:
    Ui::DeptManageSetForm *ui;
    QString selectedDepartmentId = ""; // 默认无选中部门
    void addSubDepartment(QString parentId);
    MySqlLite *mysql;
    QString roleId;
    QString userId;
    QList<DepartmentInfo> getDepartmentInfos;
    void updateSubDepartment(QString departmentId);
    QString department;
    QString departmentParent;
    QString deptParentId;
    void loadSelectedDepartment(const QString &departmentId, QLineEdit *parentNameEdit, QLineEdit *nameEdit);
    void resetFields(QLineEdit *parentNameEdit, QLineEdit *nameEdit);
    QString findParentDepartmentId(const QString &parentName) const ;

    void deleteSubDepartment(QString departmentId);
};

#endif // DEPTMANAGESETFORM_H
