#include "writesettingdialog.h"
#include "ui_writesettingdialog.h"

writeSettingDialog::writeSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::writeSettingDialog)
{
    ui->setupUi(this);
    qDebug()<<"init writeSettingDialog";
    this->setWindowTitle("Wiriting");
    changetounfinsh();
    this->setWindowModality(Qt::ApplicationModal);
    setFixedSize(this->width(),this->height());
    ui->label->setWordWrap(true);
}

writeSettingDialog::~writeSettingDialog()
{
    delete ui;
}

void writeSettingDialog::writefinish()
{

    ui->btn_OK->setEnabled(true);
    ui->label->setText(tr("write setting finish,click ok button to exit the dialog!"));
}

void writeSettingDialog::writeonefinish(int num)
{

    //ui->btn_OK->setEnabled(true);
    ui->label->setText(tr("write device ") + QString::number(num+1) + tr(" success,not is writing next device"));
}

void writeSettingDialog::writeAllfinish()
{

    ui->btn_OK->setEnabled(true);
    ui->label->setText(tr("all devices write setting finish,click ok button to exit the dialog!"));
}
void writeSettingDialog::on_btn_OK_clicked()
{
    this->accept();
    changetounfinsh();
}
void writeSettingDialog::closeEvent(QCloseEvent *event)
{
    qDebug()<<"close event";
    event->ignore();
}

void writeSettingDialog::changetounfinsh()
{

    ui->btn_OK->setEnabled(false);
    ui->label->setText(tr("now is writing settings,please wait!"));
}
