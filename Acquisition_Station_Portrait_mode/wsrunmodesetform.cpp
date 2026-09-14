#include "wsrunmodesetform.h"
#include "ui_wsrunmodesetform.h"
#include <QHostInfo>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include "config.h"

namespace {

// Keep this page visually consistent with the main station-settings form.
void applyPortraitRunModeStyle(Ui::WsRunModeSetForm *ui)
{
    const QFont labelFont("Sans Serif", 14, QFont::DemiBold);
    const QFont controlFont("Sans Serif", 14);

    ui->gridLayout->setContentsMargins(30, 24, 30, 24);
    ui->gridLayout->setHorizontalSpacing(14);
    ui->gridLayout->setVerticalSpacing(18);
    ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    ui->horizontalSpacer_2->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    const QList<QLabel *> labels = {ui->label_2, ui->label_3};
    for (QLabel *label : labels) {
        label->setFont(labelFont);
        label->setMinimumWidth(140);
        label->setMinimumHeight(48);
    }

    const QList<QLineEdit *> fields = {ui->lineEditClientIpAddress, ui->lineEditServerBaseUrl};
    for (QLineEdit *field : fields) {
        field->setFont(controlFont);
        field->setMinimumHeight(48);
    }

    ui->groupBox->setMinimumHeight(56);
    ui->radioButtonSingle->setFont(controlFont);
    ui->radioButtonServer->setFont(controlFont);
    ui->radioButtonSingle->setMinimumHeight(48);
    ui->radioButtonServer->setMinimumHeight(48);
    ui->gridLayout->invalidate();
}

}

WsRunModeSetForm::WsRunModeSetForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WsRunModeSetForm)
{
    ui->setupUi(this);
    pConfig = new Config();

    applyPortraitRunModeStyle(ui);
    initForms();
}

WsRunModeSetForm::~WsRunModeSetForm()
{
    delete ui;
}

QString WsRunModeSetForm::getLocalIP() {
    QList<QNetworkInterface> listNiTmp = QNetworkInterface::allInterfaces();
    QStringList slAllActiveIp;
    foreach  (QNetworkInterface niTmp, listNiTmp)
    {
        if (niTmp.flags().testFlag(QNetworkInterface::IsRunning))
        {
            QList<QNetworkAddressEntry> listAddressEntry = niTmp.addressEntries();
            foreach (QNetworkAddressEntry addressentry, listAddressEntry)
            {
                if (addressentry.ip().protocol() == QAbstractSocket::IPv4Protocol
                        && addressentry.ip() != QHostAddress::LocalHost)
                {
                    slAllActiveIp.append(addressentry.ip().toString());
                    qDebug() << "ip:" << addressentry.ip().toString();
                }
            }
        }
    }
    qDebug() << "slAllActiveIp:" << slAllActiveIp;
    if(slAllActiveIp.size() >= 1){
        return slAllActiveIp.at(0);
    } else {
        return "";
    }
}

void WsRunModeSetForm::initForms(){
    QVariant localIp = pConfig->Get("runConfig","localIpAddress");
    if(localIp.isNull()){
        QString localIpTmp = getLocalIP();
        if(localIpTmp.length() == 0){
            ui->lineEditClientIpAddress->setText("127.0.0.1");
        }else{
            ui->lineEditClientIpAddress->setText(localIpTmp);
        }
    }else{
        QString ipConfig = localIp.toString();
        ui->lineEditClientIpAddress->setText(ipConfig);
    }

    QVariant serverUrl = pConfig->Get("runConfig","serverUrl");
    if(serverUrl.isNull()){
        ui->lineEditServerBaseUrl->setText("http://192.168.1.200:8090/api");
    } else {
        ui->lineEditServerBaseUrl->setText(serverUrl.toString());
    }

    QVariant runMode = pConfig->Get("runConfig","runMode");
    if(runMode.isNull()){
        ui->radioButtonSingle->setChecked(false);
        ui->radioButtonServer->setChecked(false);
    } else {
        bool isOk = false;
        int runModeConfig = runMode.toInt(&isOk);
        if(isOk){
            if(runModeConfig == 1){
                ui->radioButtonSingle->setChecked(true);
                ui->radioButtonServer->setChecked(false);
            }else if(runModeConfig == 2){
                ui->radioButtonSingle->setChecked(false);
                ui->radioButtonServer->setChecked(true);
            }
        }
    }

}

bool WsRunModeSetForm::saveConfig(bool showSuccessMessage){
    int runMode = 0;
    if(ui->radioButtonSingle->isChecked()){
        runMode = 1;
    }else if(ui->radioButtonServer->isChecked()){
        runMode = 2;
    }

    if(runMode == 0){
        if (showSuccessMessage) {
            QMessageBox::warning(this, "提示", "请选择单机运行模式或者服务器运行模式！");
        }
        return false;
    }

    QString localIp = ui->lineEditClientIpAddress->text();
    QString serverUrl = ui->lineEditServerBaseUrl->text();

    pConfig->Set("runConfig","localIpAddress",localIp);
    pConfig->Set("runConfig","serverUrl",serverUrl);
    pConfig->Set("runConfig","runMode",runMode);

    if (showSuccessMessage) {
        QMessageBox::information(this, "提示", "运行模式保存成功！","完成");
    }
    return true;

}
