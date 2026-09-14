#ifndef LOCWDGET_H
#define LOCWDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <unordered_map>
#include <string>
#include "lockerutils.h"
#include "autoquerythread.h"

namespace Ui {
class locwdget;
}

class locwdget : public QWidget
{
    Q_OBJECT

public:
    explicit locwdget(QWidget *parent = 0);
    ~locwdget();


    //定义曹函数
private slots:
    void receiveInfo();
    void queryLockStatus();
    void connectOkOrNot(bool isConnect);

public:
    std::unordered_map<unsigned short,std::string> lockStatusData;
    lockerutils *utils;
    //串口对象指针
    QSerialPort *m_serialPort;
    autoquerythread *pQueryThread;
    QString byteArrayToHexStr(const QByteArray &ba);

    int ProcessSerialPort();
    void openPort();

public slots:
    void openSerialPort();
    void pushButtonSend();
    void pushButtonClear();
    void pushButtonOpenOneKey();
    void pushButtonOpenOrderAllKey();
    void pushButtonOpenAllOnce();
    void pushButtonOpenLamp();
    void pushButtonCloseLamp();
    void pushButtonQuerySingleKey();
    void pushButtonQueryAllKey();
    void pushButtonDisconnect();

private:
    Ui::locwdget *ui;
};

#endif // LOCWDGET_H
