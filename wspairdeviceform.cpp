#include "wspairdeviceform.h"
#include "ui_wspairdeviceform.h"
#include <QMessageBox>
#include <QDebug>
#include <QAbstractItemView>
#include <QFont>
#include <QResizeEvent>
#include <QScrollBar>

WsPairDeviceForm::WsPairDeviceForm(int count,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WsPairDeviceForm),
    deviceCount(count)
{
    ui->setupUi(this);

    showFullScreen();
    checkConfigPortInfo();
    initForms();
    initConnects();
    updatePortraitLayout();
}

void WsPairDeviceForm::paintEvent(QPaintEvent *event){
        QPainter painter(this);
        painter.drawPixmap(rect(),QPixmap(":/image/Dataquery8303.png"),QRect());
}

WsPairDeviceForm::~WsPairDeviceForm()
{
    delete ui;
}


void WsPairDeviceForm::pushButtonClickBack(){
    qDebug() << __FUNCTION__  << " ====================== WsPairDeviceForm::pushButtonClickBack ====================== " ;
    this->close();
}

void WsPairDeviceForm::checkConfigPortInfo(){

    pConfig = new Config();


    QVariant port_num = pConfig->Get("portInfo","port_num");
    if(port_num.isNull()){
        pConfig->Set("portInfo","port_num",deviceCount);
        for(int i = 0;i < deviceCount;++i){
            QString port_no = "port" + QString::number(i);
            pConfig->Set("portInfo",port_no,"-1");
        }
    } else {
        int originCount = port_num.toInt();
        if(originCount < deviceCount){
            //rebuild port
            pConfig->Set("portInfo","port_num",deviceCount);
            for(int i = 0;i < deviceCount;++i){
                QString port_no = "port" + QString::number(i);
                QVariant originValue = pConfig->Get("portInfo",port_no);
                if(originValue.isNull()){
                    pConfig->Set("portInfo",port_no,"-1");
                }
            }
        } else {
            //check if all ports config.
            for(int i = 0;i < originCount;++i){
                QString port_no = "port" + QString::number(i);
                QVariant originValue = pConfig->Get("portInfo",port_no);
                if(originValue.isNull()){
                    pConfig->Set("portInfo",port_no,"-1");
                }
            }
        }
    }
}


void WsPairDeviceForm::initForms(){

    currentPairPort = "";
    QVariant port_num = pConfig->Get("portInfo","port_num");
    int configPortCount = 0;
    if(!port_num.isNull()){
        configPortCount = port_num.toInt();
    }

    if(deviceCount != configPortCount){
        return;
    }

    if(deviceCount > 0){
        deviceForm.resize(deviceCount);

        for(int i = 0;i < deviceCount;++i){
            QListWidgetItem *newItem = new QListWidgetItem();

            deviceForm[i] = new DevicePortForm(this);
            deviceForm[i]->setPortNum(QString::number(i+1));
            QString port_no = "port" + QString::number(i);
            QVariant originValue = pConfig->Get("portInfo",port_no);
            if("-1" == originValue.toString()){
                deviceForm[i]->setPairStatus(0);
            }else{
                deviceForm[i]->setPairStatus(1);
            }

            ui->listWidget->insertItem(i,newItem);

            ui->listWidget->setItemWidget(newItem, deviceForm[i]);

            connect(deviceForm[i],SIGNAL(pushButtonPairDevice(QString)),this,SLOT(pushButtonPairDevice(QString)),Qt::QueuedConnection);
        }
    }

    // Three responsive portrait columns; the scroll bar stays hidden but touch/
    // mouse-wheel scrolling remains available for ports below the first screen.
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setFlow(QListView::LeftToRight);
    ui->listWidget->setWrapping(true);
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setSpacing(12);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setStyleSheet(
        "QListWidget { background: transparent; border: none; padding: 0; }"
        "QListWidget::item { background: #f3fbff; border: 1px solid #75c9ef; border-radius: 4px; }"
        "QListWidget::item:selected { background: #e2f5ff; }");
}

void WsPairDeviceForm::updatePortraitLayout()
{
    const int pageWidth = qMax(width(), 480);
    const int pageHeight = qMax(height(), 680);
    const int sideMargin = 24;
    const int topOfList = 154;
    const int gap = 12;
    const int columnCount = 3;
    // Match the main page: three equal square port cards per row.
    const int cardWidth = qMax(150, (pageWidth - sideMargin * 2 - gap * (columnCount - 1)) / columnCount);
    const int cardHeight = cardWidth;

    ui->labelPairDeviceTitle->setGeometry((pageWidth - 180) / 2, 20, 180, 42);
    ui->labelPairDeviceTitle->setFont(QFont("Sans Serif", 18, QFont::DemiBold));
    ui->label->setGeometry(sideMargin, 100, 92, 36);
    ui->label->setFont(QFont("Sans Serif", 14, QFont::DemiBold));
    ui->labelUsbPortNum->setGeometry(120, 96, pageWidth - 144, 44);
    ui->labelUsbPortNum->setFont(QFont("Sans Serif", 14));
    ui->pushButtonBack->setGeometry(pageWidth - sideMargin - 96, 24, 96, 38);
    ui->pushButtonBack->setFont(QFont("Sans Serif", 13, QFont::DemiBold));
    ui->listWidget->setGeometry(sideMargin, topOfList, pageWidth - sideMargin * 2,
                                qMax(180, pageHeight - topOfList - 20));
    ui->listWidget->setGridSize(QSize(cardWidth, cardHeight));

    for (int index = 0; index < ui->listWidget->count(); ++index) {
        QListWidgetItem *item = ui->listWidget->item(index);
        item->setSizeHint(QSize(cardWidth, cardHeight));
        if (index < deviceCount && deviceForm[index]) {
            deviceForm[index]->setFixedSize(cardWidth, cardHeight);
        }
    }
}

void WsPairDeviceForm::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updatePortraitLayout();
}
void WsPairDeviceForm::initConnects(){
     deviceThread = new WsPairDeviceThread();

     deviceThread->start();

     qDebug()<<"deviceThread->start()";

     connect(deviceThread,SIGNAL(insertRecorder(QString)),this,SLOT(toInsertRecorder(QString)),Qt::QueuedConnection);
     connect(deviceThread,SIGNAL(deleteRecorder(QString)),this,SLOT(toDeleteRecorder(QString)),Qt::QueuedConnection);
}


void WsPairDeviceForm::toInsertRecorder(QString portNum){
     ui->labelUsbPortNum->setText(portNum);
     currentPairPort = portNum;
}
void WsPairDeviceForm::toDeleteRecorder(QString portNum){
    if(portNum == currentPairPort){
        currentPairPort = "";
        ui->labelUsbPortNum->setText("");
    }
}

void WsPairDeviceForm::pushButtonPairDevice(QString portNum){
    printf("pushButtonPairDevice portNum=%s\n",portNum.toStdString().c_str());
    if(currentPairPort.length() == 0){
        QMessageBox::warning(this, "提示", "请先插入记录仪!");
        return;
    }
    bool isOk = false;
    int num = portNum.toInt(&isOk);
    if(isOk){
        int realNum = num - 1;
        int originStatus = deviceForm[realNum]->getPairStatus();
        if(originStatus == 0){
            QString port_num = "port" + QString::number(realNum);
            pConfig->Set("portInfo",port_num,currentPairPort);
            deviceForm[realNum]->setPairStatus(1);
        } else {
            QMessageBox::StandardButton response
                    = QMessageBox::question(this, "温馨提示", "当前端口已经匹配，请确认是否取消匹配？",
                                                                           QMessageBox::Yes | QMessageBox::No);
               if (response == QMessageBox::Yes) {
                   // 用户选择了“是”
                   //todo show the select dialog.
                   QString port_num = "port" + QString::number(realNum);
                   pConfig->Set("portInfo",port_num,"-1");
                   deviceForm[realNum]->setPairStatus(0);
               } else {
                   // 用户选择了“否”
               }
        }
    }

}

















