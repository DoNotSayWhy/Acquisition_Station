#include "recordersettingform.h"
#include "ui_recordersetting.h"
#include "uiprofile.h"
#include <QCheckBox>
#include <sstream>
#include <QMessageBox>
#include <QSettings>

RecorderSettingForm::RecorderSettingForm(QWidget *parent) : 
    QWidget(parent),
    ui(new Ui::RecorderSettingForm)
{
    ui->setupUi(this);
    UiProfile::apply(this, "recordersetting");
//    this->showMaximized();


}

//执行一个shell命令，输出结果逐行存储在resvec中，并返回行数
int32_t RecorderSettingForm::myexec(const char *cmd, vector<string> &resvec)
{
    resvec.clear();
    FILE *pp = popen(cmd, "r"); //建立管道
    if (!pp)
    {
        return -1;
    }
    char tmp[1024]; //设置一个合适的长度，以存储每一行输出
    while (fgets(tmp, sizeof(tmp), pp) != NULL)
    {
        if (tmp[strlen(tmp) - 1] == '\n')
        {
            tmp[strlen(tmp) - 1] = '\0'; //去除换行符
        }
        resvec.push_back(tmp);
    }
    pclose(pp); //关闭管道
    for (int i = 0; i < resvec.size(); i++)
    {
        cout << resvec.at(i) << endl;
        cout << "-----------------------------" << endl;
    }
    return resvec.size();
}
QStringList  RecorderSettingForm::getUsbPaths() {
    printf("%s------------%d\n", __FUNCTION__, __LINE__);
    DIR *dir_ptr;
    QStringList path_list;
    if ((dir_ptr = opendir("/sys/block")) == NULL) {
        printf("No USB storage detected.\n");
        return path_list;
    }

    const char *sdx[] = {"sda", "sdb", "sdc", "sdd", "sde", "sdf"};
    for (int i = 0; i < 6; i++) {
        char open_path[64] = {0};
        sprintf(open_path, "/sys/block/%s/removable", sdx[i]);
        printf("Checking path: [%s]\n", open_path);

        int fd = open(open_path, O_RDONLY);
        if (fd == -1) {
            printf("Failed to open [%s]\n", open_path);
            continue;
        }

        char buf[32] = {0};
        if (read(fd, buf, sizeof(buf)) > 0 && buf[0] == '1') {
            printf("USB found at [%s]\n", sdx[i]);
            vector<string> resvec;
            char df_path[64] = {0};
            sprintf(df_path, "df -h | grep /dev/%s", sdx[i]);
            myexec(df_path, resvec);
            if (resvec.size() == 0) {
                printf("No USB found in df output, manual mount may be required.\n");
            } else {
                cout << resvec.back() << endl;
                string df_output = resvec.back();
                size_t last_space = df_output.find_last_of(" \t");
                if (last_space != string::npos) {
                    string mount_path = df_output.substr(last_space + 1);
                    path_list.append(QString::fromStdString(mount_path));
                    printf("Mount path: [%s]\n", mount_path.c_str());
                }
            }
        }

    }

    return path_list;
}

RecorderSettingForm::~RecorderSettingForm()
{
    delete ui;
}

void RecorderSettingForm::paintEvent(QPaintEvent *event){

}


bool isAlphaNumeric(const QString &str) {
    for (QChar c : str) {
        if (!c.isDigit() && !c.isLetter()) {
            return false;
        }
    }
    return true;
}

void RecorderSettingForm::pushButtonClickConfirmModify(){
    QStringList usbPaths = getUsbPaths();
    if(usbPaths.size() == 0 || usbPaths.size() > 1){
        QMessageBox::information(this, "提示", "请确保只连接一个记录仪！","确认");
        return;
    }
    QString path = usbPaths.at(0);
    QString userNo = ui->lineEditUserNo->text();
    QString deviceNo = ui->lineEditDeviceNo->text();
    if(userNo.length() == 0 || deviceNo.length() == 0){
        QMessageBox::information(this, "提示", "请填写用户编号和设备编号！","确认");
        return;
    }

    if(!isAlphaNumeric(userNo)){
        ui->lineEditUserNo->focusPolicy();
        QMessageBox::information(this, "提示", "只请允许输入数字或者英文！","确认");
        return;
    }

    if(!isAlphaNumeric(deviceNo)){
        ui->lineEditDeviceNo->focusPolicy();
        QMessageBox::information(this, "提示", "只请允许输入数字或者英文！","确认");
        return;
    }

    QSettings setting(path+"/Param.ini", QSettings::IniFormat);
    setting.beginGroup("ZFY");
    //QString devNo = setting.value("DevNo").toString();
    //QString polNo = setting.value("PolNo").toString();
    setting.setValue("DevNo",deviceNo);
    setting.setValue("PolNo",userNo);
    setting.endGroup();

    QMessageBox::information(this, "提示", "修改成功！","完成");

}
void RecorderSettingForm::pushButtonClickReadParam(){
    ui->lineEditUserNo->setText("");
    ui->lineEditDeviceNo->setText("");
    QStringList usbPaths = getUsbPaths();
    if(usbPaths.size() == 0 || usbPaths.size() > 1){
        QMessageBox::information(this, "提示", "请确保只连接一个记录仪！","确认");
        return;
    }
    QString path = usbPaths.at(0);
    QSettings setting(path+"/Param.ini", QSettings::IniFormat);
    setting.beginGroup("ZFY");
    QString devNo = setting.value("DevNo").toString();
    QString polNo = setting.value("PolNo").toString();
    setting.endGroup();

    if(polNo.length() == 0){
        QMessageBox::information(this, "提示", "未配置数据，请先配置数据。","确认");
        return;
    }

    ui->lineEditUserNo->setText(polNo);
    ui->lineEditDeviceNo->setText(devNo);

}
