#include "workstationsetform.h"
#include "ui_workstationsetform.h"
#include "wssetform.h"
#include "wsrunmodesetform.h"
#include "wsinterfacesetform.h"

#include "hsglobal.h"


int INDEX_SET_FORM = 0;
int INDEX_RUN_MODE_FORM = 1;
int INDEX_INTERFACE_FORM = 2;


WorkStationSetForm::WorkStationSetForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorkStationSetForm)
{
    ui->setupUi(this);
    this->showMaximized();

    initForms();
    initConnectSignals();
    ui->stackedWidget->setCurrentIndex(INDEX_SET_FORM);

    ui->pushButton_w_reboot->setVisible(false);
}

WorkStationSetForm::~WorkStationSetForm()
{
    delete ui;
}


void WorkStationSetForm::initForms(){

    WsSetForm *setForm = new WsSetForm(this);
    ui->stackedWidget->insertWidget(INDEX_SET_FORM,setForm);

    WsRunModeSetForm *runModeForm = new WsRunModeSetForm(this);
    ui->stackedWidget->insertWidget(INDEX_RUN_MODE_FORM,runModeForm);

    WsInterfaceSetForm *interForm = new WsInterfaceSetForm(this);
    ui->stackedWidget->insertWidget(INDEX_INTERFACE_FORM,interForm);

}
void WorkStationSetForm::initConnectSignals(){



}


void WorkStationSetForm::pushButtonWsSet(){
    ui->stackedWidget->setCurrentIndex(INDEX_SET_FORM);
}
void WorkStationSetForm::pushButtonWsRunMode(){
    ui->stackedWidget->setCurrentIndex(INDEX_RUN_MODE_FORM);
}
void WorkStationSetForm::pushButtonWsInterface(){
    ui->stackedWidget->setCurrentIndex(INDEX_INTERFACE_FORM);
}


void WorkStationSetForm::pushButtonWsSaveConfig(){
    QWidget* pCurrentWidget = ui->stackedWidget->currentWidget();
    WsSetForm* setForm = qobject_cast<WsSetForm*>(pCurrentWidget);
    if(setForm){
        setForm->saveConfig();
    }
    WsRunModeSetForm* runModeForm = qobject_cast<WsRunModeSetForm*>(pCurrentWidget);
    if(runModeForm){
        runModeForm->saveConfig();
    }
    WsInterfaceSetForm* interfaceForm = qobject_cast<WsInterfaceSetForm*>(pCurrentWidget);
    if(interfaceForm){
        interfaceForm->saveConfig();
    }

}
void WorkStationSetForm::pushButtonWsReboot(){

}


void WorkStationSetForm::on_pushButton_w_reboot_clicked()
{

    is_mainwindow_exited = true;
    pushButtonWsSaveConfig();
    qApp->quit();
}
