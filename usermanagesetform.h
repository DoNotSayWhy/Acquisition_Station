#ifndef USERMANAGESETFORM_H
#define USERMANAGESETFORM_H

#include <QWidget>
#include "mysqllite.h"
#include "deptmanagesetform.h"
#include "networkutility.h"
#include <QDebug>
#include <QTableWidgetItem>

namespace Ui {
class UserManageSetForm;
}
struct UserInfoByUserManage {
    QString userNo;
    QString deviceNo;
    QString department;
    QString userName;
    QString statusValue;
    int roleValue;
    QString password;
    QString passwordConfirm;
    int departmentId;
};

struct ImportUserInfo{
    QString PolNo;
    QString DevNo;
    QString Name;
    QString DepName;
    QString Remark;
    QString CreateDate;

    ImportUserInfo()
            : PolNo(""), DevNo(""), Name("")
    , DepName(""), Remark(""), CreateDate(""){}

    ImportUserInfo(QString _PolNo,QString _DevNo,QString _Name,
                   QString _DepName,QString _Remark,QString _CreateDate)
            : PolNo(_PolNo), DevNo(_DevNo), Name(_Name)
    , DepName(_DepName), Remark(_Remark), CreateDate(_CreateDate){}

        ImportUserInfo(const ImportUserInfo &other)
            : PolNo(other.PolNo), DevNo(other.DevNo), Name(other.Name)
            , DepName(other.DepName), Remark(other.Remark), CreateDate(other.CreateDate) {}

        ImportUserInfo &operator=(const ImportUserInfo &other) {
            if (this != &other) {
                PolNo = other.PolNo;
                DevNo = other.DevNo;
                Name = other.Name;
                DepName = other.DepName;
                Remark = other.Remark;
                CreateDate = other.CreateDate;
            }
            return *this;
        }

};

class UserManageSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit UserManageSetForm(QString userid,QString roleId,MySqlLite *sqltie,NetworkUtility *net,QWidget *parent = 0);
    ~UserManageSetForm();

    void setUserFormSystemIPV4(QString systemIPV4);

signals:
    void userSelectt_userinfoALLTOMSQ(QString userid,QString roleId,QString userNumber,QString devNumber);
    void userSelectt_departmentAllTOMSQ(QString userid, QString roleId);
    void userGetDeptForMSQ(QString userid, QString roleId,int stauts);
    void userInsertToMSQ(const QMap<QString, QVariant>& userInfoMap);
    void userListInsertToMSQ(QList<QMap<QString, QVariant>> userInfoListMap);
    void userGetUpdateDeptForMSQ(QString userid, QString roleId,int stauts);
    void userupdatet_userInfoToMSQ(const QMap<QString, QVariant>& userInfoMap);
    void USERdelett_userinfo_byidtoMSQ(QString getUserFormSystemIPV4,QList<QMap<QString, QString>> userInfoList);

private slots:
    void showTableWidgetData(QList<NewUserInfo> userInfoData);

    void on_addpushButton_clicked();

    void gett_department(QList<DepartmentInfo> departments,int stauts);

    void on_updatepushButton_clicked();

    void on_deletepushButton_clicked();

    void on_btnBatchImport_clicked();

    void batchImportUserInfoResult(int count);

    void on_pushButtonOutport_clicked();
    void setUserDialog(bool isUpdate) ;



    void on_btnCheckBox_clicked(bool checked);

    void on_btntestpushButton_clicked();

    void onUserItemSelectionChanged();
    void onUserItemClickChanged(QTableWidgetItem *item);


    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::UserManageSetForm *ui;
    MySqlLite *mysql;
    NetworkUtility *mynet;
    QString roleId;
    QString userId;
    QList<NewUserInfo> getUserInfoData;
    QList<DepartmentInfo> getDepartmentData;
    void setUserInfoDialog();
    QVector<QCheckBox*> checkBoxes; // 存储复选框的指针
    void setUserUpdateDialog();
    QString getUserFormSystemIPV4;
    QString openExcelFileDialog(QWidget *parent);
    void setUserInfoList(QList<ImportUserInfo> userInfoList);
};

#endif // USERMANAGESETFORM_H
