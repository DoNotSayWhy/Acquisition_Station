#ifndef LAMPTHREAD_H
#define LAMPTHREAD_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QSerialPort>
#include <functional>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "lockerutils.h"
#include <QTimer>

#include<config.h>

struct Task {
    std::function<void()> func;
    int delay; // 延迟时间（毫秒）
};

class LampThread : public QThread
{
    Q_OBJECT
public:
    explicit LampThread(QObject *parent = nullptr);
    ~LampThread();
    void addTask(const std::function<void()> &task, int delay);
    void turnOnLamp(unsigned short keyId);
    void turnOffLamp(unsigned short keyId);
    void OpenOneKey();
    bool openPort();
    QString byteArrayToHexStr(const QByteArray &ba);

protected:
    void run() override;

private slots:
    void receiveInfo();
    void processTasks();

private:
    QSerialPort *m_serialPort;
    lockerutils *utils; // 假设 lockerutils 是一个自定义类
    QMutex mutex;
    QWaitCondition condition;
//    QList<std::function<void()>> taskQueue;
    QMap<unsigned short, std::string> lockStatusData; // 存储锁状态数据
    bool running; // 控制线程运行的标志
    QList<Task> taskQueue; // 修改为存储 Task 结构体的队列
    QTimer *taskTimer;



private:

    /*
         灯口定义 678 -RGB ,1 -锁
    */

    enum LampChannel {
        RedLampChannel = 6,
        GreenLampChannel = 7,
        BlueLampChannel = 8
    };

};

#endif // LAMPTHREAD_H
