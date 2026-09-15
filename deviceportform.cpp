#include "deviceportform.h"
#include "ui_deviceportform.h"
#include "uiprofile.h"
#include <QFont>

DevicePortForm::DevicePortForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DevicePortForm)
{
    ui->setupUi(this);
    UiProfile::apply(this, "deviceportform");
    currentPortNum = "";
    ui->gridLayout->setContentsMargins(16, 12, 16, 12);
    ui->gridLayout->setHorizontalSpacing(8);
    ui->gridLayout->setVerticalSpacing(4);
    ui->labelPortNum->setFont(QFont("Sans Serif", 16, QFont::DemiBold));
    ui->pushButtonPairOrNot->setFont(QFont("Sans Serif", 14, QFont::DemiBold));
    ui->pushButtonPairOrNot->setMinimumSize(132, 48);
}

DevicePortForm::~DevicePortForm()
{
    delete ui;
}

void DevicePortForm::setPortNum(QString txt){
    ui->labelPortNum->setText(txt);
    currentPortNum = txt;
}

void DevicePortForm::pushButtonPairDeviceClick(){
    emit pushButtonPairDevice(currentPortNum);
}

void DevicePortForm::setPairStatus(int status){
    this->pairStatus = status;
    if(0 == status){
        ui->pushButtonPairOrNot->setStyleSheet("QPushButton{color: white; border: none;background-image: url(:/image/portmatching3868.png);}QPushButton:pressed{border-image: url(:/image/usermanagement4124.png); border: none;color: rgb(85, 255, 255);}");
        ui->pushButtonPairOrNot->setText("点击匹配");
    }else if(1 == status){
        ui->pushButtonPairOrNot->setStyleSheet("QPushButton{color: white; border: none;background-image: url(:/image/portmatching1.png);}QPushButton:pressed{border-image: url(:/image/usermanagement4124.png); border: none;color: rgb(85, 255, 255);}");
        ui->pushButtonPairOrNot->setText("已匹配");
    }
}
int DevicePortForm::getPairStatus(){
    return this->pairStatus;
}

















