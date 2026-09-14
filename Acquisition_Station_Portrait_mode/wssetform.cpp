#include "wssetform.h"
#include "ui_wssetform.h"
#include "wspairdeviceform.h"
#include "config.h"
#include <QMessageBox>
#include <QFont>
#include <opencv2/opencv.hpp>

namespace {

void applyPortraitWsSetLayout(Ui::WsSetForm *ui, QWidget *page)
{
    const int width = page->width();
    const int margin = 65;
    const int labelWidth = 180;
    const int fieldWidth = 150;
    const int rightX = 450;
    const int rightLabelWidth = 80;
    const int rightFieldWidth = 150;
    const int height = 48;
    const QFont labelFont("Sans Serif", 14, QFont::DemiBold);
    const QFont fieldFont("Sans Serif", 14);

    auto place = [&](QLabel *label, QWidget *field, int y) {
        label->setGeometry(margin, y, labelWidth, height);
        field->setGeometry(margin + labelWidth + 12, y, fieldWidth, height);
        label->setFont(labelFont);
        field->setFont(fieldFont);
    };
    auto placeRight = [&](QLabel *label, QWidget *field, int y) {
        label->setGeometry(rightX, y, rightLabelWidth, height);
        field->setGeometry(rightX + rightLabelWidth + 10, y, rightFieldWidth, height);
        label->setFont(labelFont);
        field->setFont(fieldFont);
    };

    place(ui->label_10, ui->comboBoxResolution, 24);
    placeRight(ui->label_UPLOAD, ui->comboBoxUPLOAD, 24);
    place(ui->label_9, ui->spinBoxPortCount, 88);
    placeRight(ui->label_UPLOAD_time, ui->timeEditUPLOAD, 88);
    place(ui->label_8, ui->spinBoxRowPortCount, 152);
    place(ui->label_7, ui->comboBoxSavePath, 216);
    place(ui->label_6, ui->lineEditSaveDir, 280);
    ui->lineEditSaveDir->setGeometry(margin + labelWidth + 12, 280, fieldWidth * 2, height);
    place(ui->label_5, ui->spinBoxSaveDays, 344);
    place(ui->label_4, ui->comboBoxPairDevice, 408);
    ui->pushButtonPairDevice->setGeometry(margin + labelWidth + fieldWidth + 24, 408, 140, height);
    ui->pushButtonPairDevice->setFont(fieldFont);
    place(ui->label_3, ui->comboBoxDeleteOrNot, 472);
    place(ui->labelVoice, ui->comboBoxVoice, 536);
    place(ui->label, ui->currversion, 600);
    ui->radioButton->hide();
    ui->radioButton_2->hide();
    ui->labelcamera->hide();
    ui->comboBoxcamera->hide();
}

}

WsSetForm::WsSetForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WsSetForm)
{
    ui->setupUi(this);
    pConfig = new Config();

    initForms();
    applyPortraitWsSetLayout(ui, this);
}

WsSetForm::~WsSetForm()
{
    delete ui;
}

void WsSetForm::pushButtonPairDeviceClick(){
    int portCount = ui->spinBoxPortCount->value();
    WsPairDeviceForm *form = new WsPairDeviceForm(portCount);
    form->show();
}

QList<QPair<QString, QString>> WsSetForm::getMountPoints() {
    QList<QPair<QString, QString>> mountPoints;
    QFile file("/proc/mounts");
    if (file.open(QIODevice::ReadOnly)) {
        QStringList lines = QString(file.readAll()).split("\n");
        for (const QString &line : lines) {
            QStringList parts = line.split(" ");
            if (parts.size() >= 3) {
                QString devPath = parts[0];
                if(devPath.startsWith("/dev")){
                     mountPoints.append({parts[0], parts[1]});
                }
            }
        }
    }
    return mountPoints;
}

QStringList WsSetForm::getMediaMountPoints() {
    QStringList mountPoints;

    // 创建 QProcess 对象
    QProcess process;

    process.start("/bin/bash", QStringList() << "-c" << "/bin/lsblk -o NAME,TYPE,SIZE,VENDOR,MODEL,MOUNTPOINT | grep -i part | grep '/media'");

    process.waitForFinished();

    // 获取命令输出
    QString output = process.readAllStandardOutput();
   // qDebug() << "Command Output:" << output; // 调试输出

    // 按行分割输出
    QStringList lines = output.split('\n', QString::SkipEmptyParts);

    // 提取挂载点
    for (const QString &line : lines) {
        QStringList columns = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        //qDebug() << "Columns:" << columns; // 调试输出
        if (columns.size() == 4) { // 确保列的数量为4
            QString mountPoint = columns.last().trimmed(); // 去除两端空白字符和换行符
            mountPoints.append(mountPoint);
        }
    }

    // 输出挂载点
    qDebug() << "Mount Points in /media:";
    for (const QString &mountPoint : mountPoints) {
        qDebug() << mountPoint; // 应该会正确打印挂载点
    }

    return mountPoints;
}

void WsSetForm::comboBoxCurrentIndexChanged(int index){

    qDebug() << "comboBoxCurrentIndexChanged:" << index << "comboBoxCurrentIndexChanged end\n";
    if(index == 1){
        ui->pushButtonPairDevice->show();
    }else{
        ui->pushButtonPairDevice->hide();
    }
}

void WsSetForm::initForms(){
    QStringList reslutionList;

    reslutionList.append("1920 * 1080");


    QStringList setPairDeviceList;
    setPairDeviceList.append("不设置");
    setPairDeviceList.append("设置");

    QStringList setDeleteFileList;
    setDeleteFileList.append("不删除");
    setDeleteFileList.append("删除");


    ui->comboBoxResolution->addItems(reslutionList);
    QVariant reslution = pConfig->Get("wsConfig","reslution");
    if(reslution.isNull()){
        ui->comboBoxResolution->setCurrentIndex(0);
    }else{
        QString resConfig = reslution.toString();
        int index = 0;
        for(int i = 0; i < reslutionList.size(); i++) {
            qDebug() << reslutionList.at(i);
            if(reslutionList.at(i) == resConfig){
                index = i;
                break;
            }
        }
        ui->comboBoxResolution->setCurrentIndex(index);
    }

    // The port-card collection is dynamic.  Keep a non-zero, documented
    // range while allowing large workstations.
    ui->spinBoxPortCount->setRange(1,999);
    ui->spinBoxPortCount->setValue(30);
    QVariant portCountStr = pConfig->Get("wsConfig","portCount");
    if(!portCountStr.isNull()){
        bool isOk = false;
        int portCount = portCountStr.toInt(&isOk);
        if(isOk){
            ui->spinBoxPortCount->setValue(portCount);
        }
    }

    ui->spinBoxRowPortCount->setRange(4,8);
    ui->spinBoxRowPortCount->setValue(4);
    QVariant portRowCountStr = pConfig->Get("wsConfig","portRowCount");
    if(!portRowCountStr.isNull()){
        bool isOk = false;
        int portRowCount = portRowCountStr.toInt(&isOk);
        if(isOk){
            ui->spinBoxRowPortCount->setValue(portRowCount);
        }
    }

//    QList<QPair<QString, QString>> mounts = getMountPoints();
    QStringList mounts = getMediaMountPoints();
    if(mounts.isEmpty()){
        mounts.append("/data");
    }else{

        #ifdef USE_DATA_DISK

        mounts.append("/data");

        #endif
    }


    for (const auto &mount : mounts) {
//        qDebug() << "Device:" << mount.first << "Mount Point:" << mount.second;
        ui->comboBoxSavePath->addItem(mount);
    }
    QVariant saveDirStr = pConfig->Get("wsConfig","saveDir");
    if(!saveDirStr.isNull()){
        int index = 0;
        for (const auto &mount : mounts) {
//            qDebug() << "Device:" << mount.first << "Mount Point:" << mount.second;
            if(mount == saveDirStr.toString()){
                break;
            }
            index ++;
        }
        ui->comboBoxSavePath->setCurrentIndex(index);
    }

    ui->lineEditSaveDir->setText("wsData");
    QVariant saveFilePathStr = pConfig->Get("wsConfig","saveFilePath");
    if(!saveFilePathStr.isNull()){
        QString saveFilePath = saveFilePathStr.toString();
        ui->lineEditSaveDir->setText(saveFilePath);
    }

    ui->spinBoxSaveDays->setRange(-1,1800);
    ui->spinBoxSaveDays->setValue(-1);
    QVariant saveDaysStr = pConfig->Get("wsConfig","saveDays");
    if(!saveDaysStr.isNull()){
        bool isOk = false;
        int saveDays = saveDaysStr.toInt(&isOk);
        if(isOk){
            ui->spinBoxSaveDays->setValue(saveDays);
        }
    }


    ui->comboBoxPairDevice->addItems(setPairDeviceList);


    QVariant pairDeviceStr = pConfig->Get("wsConfig","pairDevice");
    if(!pairDeviceStr.isNull()){
        bool isOk = false;
        int pairDevieIndex = pairDeviceStr.toInt(&isOk);
        if(isOk){
            ui->comboBoxPairDevice->setCurrentIndex(pairDevieIndex);
        }
    }

    ui->comboBoxDeleteOrNot->addItems(setDeleteFileList);
    ui->comboBoxDeleteOrNot->setEditable(false);
    QVariant deleteFileStr = pConfig->Get("wsConfig","deleteFile");
    if(!deleteFileStr.isNull()){
        bool isOk = false;
        int deleteFileIndex = deleteFileStr.toInt(&isOk);
        if(isOk){
            ui->comboBoxDeleteOrNot->setCurrentIndex(deleteFileIndex);
        }else{
            //posible the value is ture or false.
            bool value = deleteFileStr.toBool();
            if(value){
                //true  to delete.
                ui->comboBoxDeleteOrNot->setCurrentIndex(1);
            }else{
                //false.  no delete
                ui->comboBoxDeleteOrNot->setCurrentIndex(0);
            }
        }
    }
#ifdef USE_FACE_FINGER
        std::vector<int> cameras = getAvailableCameras();
        for (int cameraIndex : cameras) {
            ui->comboBoxcamera->addItem("Camera " + QString::number(cameraIndex), cameraIndex);
        }
        int savedCameraIndex = pConfig->Get("wsConfig", "cameraIndex").toInt();
        int count = ui->comboBoxcamera->count();
        for (int i = 0; i < count; ++i) {
            if (ui->comboBoxcamera->itemData(i).toInt() == savedCameraIndex) {
                ui->comboBoxcamera->setCurrentIndex(i); // 设置当前选中项
                break;
            }
        }
#endif
        ui->radioButton->setText("执法音视频数据采集系统");
        ui->radioButton_2->setText("作业音视频数据采集系统");

        ui->radioButton->setVisible(false);
        ui->radioButton_2->setVisible(false);

        QStringList voicePlayback;
        voicePlayback.append("不播放");
        voicePlayback.append("播放");

        ui->comboBoxVoice->addItems(voicePlayback);
        QVariant voicePlaybackStr = pConfig->Get("wsConfig","isVoicePlayback");
        bool value = voicePlaybackStr.toBool();
        if(value){
            //true  to delete.
            ui->comboBoxVoice->setCurrentIndex(1);
        }else{
            //false.  no delete
            ui->comboBoxVoice->setCurrentIndex(0);
        }

        QStringList Upload;
        Upload.append("手动上传");
        Upload.append("自动上传");
        Upload.append("不设置");

        ui->comboBoxUPLOAD->addItems(Upload);
        QVariant UploadStr = pConfig->Get("wsConfig","Upload");
        // 尝试将 UploadStr 转换为整数
        bool conversionOk;
        int uploadNum = UploadStr.toInt(&conversionOk);

        // 检查转换是否成功，并且 uploadNum 是否在有效范围内
        if (conversionOk && (uploadNum == 0 || uploadNum == 1 || uploadNum == 2)) {
            // 如果转换成功且 uploadNum 是 0 或 1，设置当前索引
            ui->comboBoxUPLOAD->setCurrentIndex(uploadNum);
        } else {
            // 转换失败或 uploadNum 不在有效范围，默认选择第一个项
            ui->comboBoxUPLOAD->setCurrentIndex(0);
        }


        // 获取当前时间
        QTime currentTime = QTime::currentTime();

        // 设置 QTimeEdit 的最小时间为当前时间
        ui->timeEditUPLOAD->setMinimumTime(currentTime);

        // 设置 QTimeEdit 的最大时间为 23:59
        QTime maxTime(23, 59);
        ui->timeEditUPLOAD->setMaximumTime(maxTime);

        // 设置 QTimeEdit 的显示格式为 24 小时制，不显示秒
        ui->timeEditUPLOAD->setDisplayFormat("HH:mm"); // 只显示小时和分钟

        // 从配置中读取时间字符串
        QString savedTimeString = pConfig->Get("wsConfig", "UploadDateTime").toString(); // 从配置中读取时间字符串
        QTime savedTime = QTime::fromString(savedTimeString, "HH:mm:ss"); // 将字符串转换为 QTime

        if (savedTime.isValid()) {
            ui->timeEditUPLOAD->setTime(savedTime); // 设置时间到 QTimeEdit
        } else {
            ui->timeEditUPLOAD->setTime(currentTime); // 如果无效，则设置为当前时间
        }


        QString currversion =  pConfig->Get("wsConfig", "currversion").toString();
        if(currversion.isEmpty()){
            ui->currversion->setText("V1.0");
        }else{
            ui->currversion->setText(currversion);
        }



}


std::vector<int> WsSetForm::getAvailableCameras() {
    std::vector<int> availableCameras;
    for (int i = 0; i < 5; ++i) {
        cv::VideoCapture tempCamera;
        if (tempCamera.open(i)) {
            availableCameras.push_back(i);
            tempCamera.release(); // 释放摄像头
        }
    }
    return availableCameras;
}


bool WsSetForm::saveConfig(bool showSuccessMessage){

    const int previousPortCount = pConfig->Get("portInfo", "port_num").toInt();

    hsGlobalMtx.lock();

    QString reslution = ui->comboBoxResolution->currentText();
    int portCount = qBound(1, ui->spinBoxPortCount->value(), 999);
    int portRowCount = ui->spinBoxRowPortCount->value();
    QString saveDir = ui->comboBoxSavePath->currentText();
    QString saveFilePath = ui->lineEditSaveDir->text();
    QString selectedOption = "";
    if (ui->radioButton->isChecked()) {
//        selectedOption = ui->radioButton->text(); // 获取选中的文本
           selectedOption = "./6608.png";
    } else if (ui->radioButton_2->isChecked()) {
//        selectedOption = ui->radioButton_2->text(); // 获取选中的文本
        selectedOption = "./6609.png";

    }
    int saveDays = ui->spinBoxSaveDays->value();
    int pairDevice = ui->comboBoxPairDevice->currentIndex();
    int deleteFile = ui->comboBoxDeleteOrNot->currentIndex();
    int isVoicePlayback = ui->comboBoxVoice->currentIndex();
    int uploadNum = ui->comboBoxUPLOAD->currentIndex();
    QTime UploadDateTime = ui->timeEditUPLOAD->time(); // 获取时间


#ifdef USE_FACE_FINGER
    int camera = ui->comboBoxcamera->currentIndex();
    // 获取选中的摄像头的真实索引
    int cameraIndex = ui->comboBoxcamera->itemData(camera).toInt();

    // 保存真实的摄像头索引到配置
    pConfig->Set("wsConfig", "cameraIndex", cameraIndex);
#endif
    pConfig->Set("wsConfig","reslution",reslution);
    pConfig->Set("wsConfig","portCount",portCount);
    pConfig->Set("wsConfig","portRowCount",portRowCount);
    pConfig->Set("wsConfig","saveDir",saveDir);
    pConfig->Set("wsConfig","saveFilePath",saveFilePath);
    pConfig->Set("wsConfig","saveDays",saveDays);
    pConfig->Set("wsConfig","dataPath",saveDir + "/" + saveFilePath);
    pConfig->Set("wsConfig","pairDevice",pairDevice);
    pConfig->Set("wsConfig","deleteFile",deleteFile==0?false:true);
    pConfig->Set("path","autodeletecopyfile",deleteFile==0?false:true);
    pConfig->Set("wsConfig","logo",selectedOption);
    pConfig->Set("wsConfig","isVoicePlayback",isVoicePlayback==0?false:true);
    pConfig->Set("wsConfig","Upload",uploadNum);

    pConfig->Set("wsConfig", "UploadDateTime", UploadDateTime.toString());

    pConfig->Set("portInfo","port_num",portCount);

    QString currversion = ui->currversion->text();
    pConfig->Set("wsConfig", "currversion",currversion);

    pConfig->Sync();
    hsGlobalMtx.unlock();

    if (showSuccessMessage && previousPortCount != portCount) {
        QMessageBox::information(this, "提示", "端口数量已保存，请点击“重启生效”后重新启动软件加载新的端口数量。", "完成");
        return true;
    }

    if (showSuccessMessage) {
        QMessageBox::information(this, "提示", "设置保存成功！","完成");
    }

    return true;
}
