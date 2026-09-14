#ifndef SYSTEMMANAGEWINDOW_H
#define SYSTEMMANAGEWINDOW_H

#include <QMainWindow>
#include<mysqllite.h>
#include "deptmanagesetform.h"
#include "networkutility.h"
#include "usermanagesetform.h"

namespace Ui {
class SystemManageWindow;
}
class SystemManageWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SystemManageWindow(QString userid,QString roleId,MySqlLite *sqltie,NetworkUtility *net,QString myipv4Address,QWidget *parent = nullptr);
    ~SystemManageWindow();


    void setSystemIPV4Label(QString systemIPV4);

private:
    Ui::SystemManageWindow *ui;
    MySqlLite *mysql;
    NetworkUtility *mynet;
    QString roleId;
    QString userId;
    QString getSystemIPV4Label;
    UserManageSetForm *userForm;

private:
    void initForms();
    void initConnectSignals();
    DeptManageSetForm *deptForm;

private slots:
    void pushButtonBackClick();
    void pushButtonRecorderSet();
    void pushButtonWorkstationSet();
    void pushButtonDeptSet();
    void pushButtonUserManage();
    void pushButtonLogQuery();

signals:

};

#endif // SYSTEMMANAGEWINDOW_H
