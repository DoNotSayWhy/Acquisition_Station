#include "workstationsetform.h"
#include "ui_workstationsetform.h"
#include "wssetform.h"
#include "wsrunmodesetform.h"
#include "wsinterfacesetform.h"

#include "hsglobal.h"
#include <QFont>
#include <QResizeEvent>
#include <QTimer>
#include <QCoreApplication>

namespace {

void applyPortraitWorkstationLayout(Ui::WorkStationSetForm *ui, QWidget *page)
{
    const int width = page->width();
    const int height = page->height();
    const int margin = 20;
    const int gap = 16;
    const int buttonWidth = (width - margin * 2 - gap * 2) / 3;

    QPushButton *navButtons[] = {
        ui->pushButton_w_set, ui->pushButton_w_run_mode, ui->pushButton_w_interface_set
    };
    for (int i = 0; i < 3; ++i) {
        navButtons[i]->setGeometry(margin + i * (buttonWidth + gap), 16, buttonWidth, 54);
        navButtons[i]->setFont(QFont("Sans Serif", 14, QFont::DemiBold));
    }
    ui->stackedWidget->setGeometry(margin, 88, width - margin * 2, height - 180);
    ui->pushButton_w_save->setGeometry(margin, height - 74, 180, 54);
    ui->pushButton_w_save->setFont(QFont("Sans Serif", 14, QFont::DemiBold));
    ui->pushButton_w_reboot->setGeometry(width - margin - 180, height - 74, 180, 54);
}

}


int INDEX_SET_FORM = 0;
int INDEX_RUN_MODE_FORM = 1;
int INDEX_INTERFACE_FORM = 2;


WorkStationSetForm::WorkStationSetForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorkStationSetForm)
{
    ui->setupUi(this);
    applyPortraitWorkstationLayout(ui, this);

    initForms();
    initConnectSignals();
    ui->stackedWidget->setCurrentIndex(INDEX_SET_FORM);

    ui->pushButton_w_reboot->setVisible(true);
    ui->pushButton_w_reboot->setText(QString::fromUtf8("重启生效"));
    ui->pushButton_w_reboot->setFont(QFont("Sans Serif", 14, QFont::DemiBold));
}

WorkStationSetForm::~WorkStationSetForm()
{
    delete ui;
}

void WorkStationSetForm::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyPortraitWorkstationLayout(ui, this);
    ui->pushButton_w_save->raise();
    ui->pushButton_w_reboot->raise();
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
    if (restartInProgress) {
        return;
    }

    restartInProgress = true;
    ui->pushButton_w_reboot->setEnabled(false);

    // Save without a QMessageBox: a nested modal dialog can be hidden by the
    // full-screen shell and makes the restart look like a frozen application.
    QWidget *current = ui->stackedWidget->currentWidget();
    bool saved = false;
    if (WsSetForm *setForm = qobject_cast<WsSetForm *>(current)) {
        saved = setForm->saveConfig(false);
    } else if (WsRunModeSetForm *runModeForm = qobject_cast<WsRunModeSetForm *>(current)) {
        saved = runModeForm->saveConfig(false);
    } else if (WsInterfaceSetForm *interfaceForm = qobject_cast<WsInterfaceSetForm *>(current)) {
        saved = interfaceForm->saveConfig(false);
    }
    if (!saved) {
        restartInProgress = false;
        ui->pushButton_w_reboot->setEnabled(true);
        return;
    }

    // All worker loops observe this flag.  Post quit to the event loop so
    // queued worker signals cannot race with the settings button handler.
    is_mainwindow_exited = true;
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}
