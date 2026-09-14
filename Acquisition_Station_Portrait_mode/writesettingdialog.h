#ifndef WRITESETTINGDIALOG_H
#define WRITESETTINGDIALOG_H

#include <QDialog>
#include<QDebug>
#include<QMouseEvent>
namespace Ui {
class writeSettingDialog;
}

class writeSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit writeSettingDialog(QWidget *parent = nullptr);
    ~writeSettingDialog();
private slots:
    void writefinish();

    void on_btn_OK_clicked();

    void writeonefinish(int num);
    void writeAllfinish();
private:
    Ui::writeSettingDialog *ui;
    void changetounfinsh();
    void closeEvent(QCloseEvent *event);
};

#endif // WRITESETTINGDIALOG_H
