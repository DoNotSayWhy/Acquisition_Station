#include "wsinterfacesetform.h"
#include "ui_wsinterfacesetform.h"
#include "config.h"
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFont>
#include <QLabel>
#include <QLineEdit>

namespace {

// Match the typography, input height and spacing used by station settings.
void applyPortraitInterfaceStyle(Ui::WsInterfaceSetForm *ui)
{
    const QFont labelFont("Sans Serif", 14, QFont::DemiBold);
    const QFont controlFont("Sans Serif", 14);

    ui->gridLayout->setContentsMargins(30, 24, 30, 24);
    ui->gridLayout->setHorizontalSpacing(14);
    ui->gridLayout->setVerticalSpacing(14);
    ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    ui->horizontalSpacer_2->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    const QList<QLabel *> labels = {
        ui->label_5, ui->label_6, ui->label_7, ui->label_8,
        ui->label_9, ui->label_10, ui->label_11
    };
    for (QLabel *label : labels) {
        label->setFont(labelFont);
        label->setMinimumWidth(140);
        label->setMinimumHeight(48);
    }

    const QList<QLineEdit *> fields = {
        ui->lineEditInterfaceUrl, ui->lineEditInterfaceKey, ui->lineEditFtpUrl,
        ui->lineEditFtpPort, ui->lineEditFtpUser, ui->lineEditFtpPw
    };
    for (QLineEdit *field : fields) {
        field->setFont(controlFont);
        field->setMinimumHeight(48);
    }

    ui->checkBoxConnectThirdServer->setFont(controlFont);
    ui->checkBoxConnectThirdServer->setMinimumHeight(48);
    ui->comboBoxFileTranslate->setFont(controlFont);
    ui->comboBoxFileTranslate->setMinimumHeight(48);
    ui->gridLayout->invalidate();
}

}

WsInterfaceSetForm::WsInterfaceSetForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WsInterfaceSetForm)
{
    ui->setupUi(this);
    pConfig = new Config();

    applyPortraitInterfaceStyle(ui);
    initForms();
}

WsInterfaceSetForm::~WsInterfaceSetForm()
{
    delete ui;
}

bool WsInterfaceSetForm::saveConfig(bool showSuccessMessage){
    Qt::CheckState checkStatus = ui->checkBoxConnectThirdServer->checkState();
    if(checkStatus == Qt::Checked){
        pConfig->Set("interfaceConfig","connectThird",1);
    } else {
        pConfig->Set("interfaceConfig","connectThird",0);
    }

    QString thirdUrl = ui->lineEditInterfaceUrl->text();
    QString thirdKey = ui->lineEditInterfaceKey->text();

    QString ftpUrl = ui->lineEditFtpUrl->text();
    QString ftpPort = ui->lineEditFtpPort->text();
    QString ftpUser = ui->lineEditFtpUser->text();
    QString ftpPw = ui->lineEditFtpPw->text();
    int index = ui->comboBoxFileTranslate->currentIndex();

    pConfig->Set("interfaceConfig","thirdUrl",thirdUrl);
    pConfig->Set("interfaceConfig","thirdKey",thirdKey);
    pConfig->Set("interfaceConfig","ftpUrl",ftpUrl);
    pConfig->Set("interfaceConfig","ftpPort",ftpPort);
    pConfig->Set("interfaceConfig","ftpUser",ftpUser);
    pConfig->Set("interfaceConfig","ftpPw",ftpPw);
    pConfig->Set("interfaceConfig","fileTranslate",index);

    if (showSuccessMessage) {
        QMessageBox::information(this, "提示", "接口设置保存成功！","完成");
    }
    return true;

}

void WsInterfaceSetForm::initForms(){

    QStringList translateFileConfig;
    translateFileConfig.append("不压缩转换");
    translateFileConfig.append("压缩转换");
    ui->comboBoxFileTranslate->addItems(translateFileConfig);

    QVariant fileTranslate = pConfig->Get("interfaceConfig","fileTranslate");
    if(fileTranslate.isNull()){
        ui->comboBoxFileTranslate->setCurrentIndex(0);
    }else{
        bool isOk = false;
        int index = fileTranslate.toInt(&isOk);
        if(isOk){
            ui->comboBoxFileTranslate->setCurrentIndex(index);
        }
    }

    QVariant connectThird = pConfig->Get("interfaceConfig","connectThird");
    if(connectThird.isNull()){
        ui->checkBoxConnectThirdServer->setChecked(false);
    }else{
        bool isOk = false;
        int con = connectThird.toInt(&isOk);
        if(isOk){
            if(1 == con){
                ui->checkBoxConnectThirdServer->setChecked(true);
            }else{
               ui->checkBoxConnectThirdServer->setChecked(false);
            }
        }
    }

     QVariant thirdUrl = pConfig->Get("interfaceConfig","thirdUrl");
     if(!thirdUrl.isNull()){
         ui->lineEditInterfaceUrl->setText(thirdUrl.toString());
     }
     QVariant thirdKey = pConfig->Get("interfaceConfig","thirdKey");
     if(!thirdKey.isNull()){
         ui->lineEditInterfaceKey->setText(thirdKey.toString());
     }

     QVariant ftpUrl = pConfig->Get("interfaceConfig","ftpUrl");
     if(!ftpUrl.isNull()){
         ui->lineEditFtpUrl->setText(ftpUrl.toString());
     }
     QVariant ftpPort = pConfig->Get("interfaceConfig","ftpPort");
     if(!ftpPort.isNull()){
         ui->lineEditFtpPort->setText(ftpPort.toString());
     }
     QVariant ftpUser = pConfig->Get("interfaceConfig","ftpUser");
     if(!ftpUser.isNull()){
         ui->lineEditFtpUser->setText(ftpUser.toString());
     }
     QVariant ftpPw = pConfig->Get("interfaceConfig","ftpPw");
     if(!ftpPw.isNull()){
         ui->lineEditFtpPw->setText(ftpPw.toString());
     }

}
