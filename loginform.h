#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QDialog>
//#include<mainwindow.h>
#include <QtNetwork>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
#include<QNetworkRequest>
#include<QMessageBox>
#include<config.h>
#include<QRegExpValidator>
#include<qlineedit.h>
#include "faceformutility.h"
#include "mysqllite.h"
#include "fingerprintform.h"
namespace Ui {
class LoginForm;
}

class LoginForm : public QDialog
{
    Q_OBJECT

public:
    explicit LoginForm(int type,MySqlLite *sqltie,QWidget *parent = nullptr);

    ~LoginForm();
public:
    enum LoginFormType {
        DataQuery,       // 默认值为 0
        SystemSettings,  // 默认值为 1
        Unlock,          // 默认值为 2
        Update,          // 默认值为 3
        Exit,            // 默认值为 4
        PersonalCenter,   // 默认值为 5
        DevicePair          // 默认值为 6 设备关联
    };
signals:
    void loginsuccess(int type,QString login_UserName,QStringList jurstrlist);
    void sigLogSelectUserNo(const QString &username,const QString &password);
    void loginsuccessRoleId(int type,const QString &username,const QString &roleId);
private slots:
    void on_pushButton_clicked();
    void appendToInput(const QString &digit);

    void requestFinished(QNetworkReply *reply);
    bool eventFilter(QObject *obj, QEvent *event);
    void on_pushButtonfinger_clicked();
    void on_pushButtonface_clicked();

public slots:
    void LogSelectUserNoSuc(const QString &username,const QString &roleId);
    void goBackLogDialog();
    void showForFaceR(QString);
    void fingerRecognizeResult(QString userNo,QString userName);


private:
    Ui::LoginForm *ui;

    QNetworkRequest request;
    QNetworkAccessManager* naManager;
    int type = -1;
    QString login_UserName;
    QLineEdit *currentFocus = nullptr;
    void ifopen_face_finger(bool enableFace, bool enableFinger);
    FaceFormUtility *face = nullptr;
    MySqlLite *mysql = nullptr;
    FingerPrintForm *mFingerForm = nullptr;
    int indexFace;
    int indexFinger;

};

#endif // LOGINFORM_H
