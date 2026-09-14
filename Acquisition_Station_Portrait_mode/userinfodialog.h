#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <QDialog>
#include "mysqllite.h"
#include "faceformutility.h"
#include "ziangfingerutility.h"
#include "fingerprintform.h"
namespace Ui {
class UserInfoDialog;
}


class UserInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserInfoDialog(QString title,NewUserInfo user,QList<DepartmentInfo> departments,MySqlLite *sqltie,QWidget *parent = nullptr);
    ~UserInfoDialog();

    QString getUserNo() const;
    QString getDeviceNo() const;
    QString getDepartment() const;
    QString getUserName() const;
    QString getFaceStatus() const;
    QString getFingerprintStatus() const;
    QString getUserStatus() const;
    QString getRole() const;
    QString getPassWD() const ;
    QString getPassWDTwo() const;

private slots:
    void on_pushButtonok_clicked();

    void on_pushButtoncancel_clicked();

    void on_pushButtonfinger_clicked();

    void updatefingerLabel(const QString& message);

    void on_pushButtonface_clicked();

    void goBackUserInfoDialog();

    void fingerEnrollResult(QString userNo);

private:
    Ui::UserInfoDialog *ui;
    MySqlLite *mysql;
    QList<DepartmentInfo> getDepartments;
    void setUserInfoDialogData(NewUserInfo user);
    void  ifopen_face_finger(bool);
    FaceFormUtility *face;
    void initFingerStatus();
    FingerPrintForm *fingerForm;
    int indexFingerForm;
    int indexFaceForm;

};

#endif // USERINFODIALOG_H


