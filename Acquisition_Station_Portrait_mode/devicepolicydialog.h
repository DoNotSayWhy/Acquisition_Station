#ifndef DEVICEPOLICYDIALOG_H
#define DEVICEPOLICYDIALOG_H


#include <QDialog>
#include "mysqllite.h"

namespace Ui {
class DevicePolicyDialog;
}

class DevicePolicyDialog :  public QDialog
{
    Q_OBJECT

public:
    explicit DevicePolicyDialog(MySqlLite* sqltie,QWidget *parent = nullptr);
    ~DevicePolicyDialog();
    QString getDevNo() const;
    QString getPolNo() const;  // Changed to return the selected UserNo

signals:
    void getUserNoList();

private slots:
    void searchUserNo();
    void accept() override;
    void reject() override;
    void setListWidgetData(const QStringList& userNos);

private:
    QString devNo;  // Stores devNo
    QString polNo;
    MySqlLite *mysqltie = nullptr;
    QStringList myuserNos;

private:
    Ui::DevicePolicyDialog *ui;
};

#endif // DEVICEPOLICYDIALOG_H
