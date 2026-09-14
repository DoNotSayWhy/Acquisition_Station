#include "lampthread.h"
#include <QDateTime>
#include <QTimer>
#include <QCoreApplication>


LampThread::LampThread(QObject *parent) : QThread(parent), m_serialPort(nullptr), taskTimer(new QTimer(this)) {
    connect(taskTimer, &QTimer::timeout, this, &LampThread::processTasks);
}

void LampThread::addTask(const std::function<void()> &task, int delay) {
    QMutexLocker locker(&mutex);
    taskQueue.append({task, delay}); // 存储任务和对应的延迟时间
    if (taskQueue.size() == 1) { // 如果是第一个任务，启动定时器
        taskTimer->start(delay);
    }
}

void LampThread::run() {
    m_serialPort = new QSerialPort();

    if (!openPort()) {
        qDebug() << "Failed to open serial port. Exiting thread.";
        return; // 如果串口打开失败，退出线程
    }

    // 连接信号和槽
    connect(m_serialPort, &QSerialPort::readyRead, this, &LampThread::receiveInfo);

    exec(); // 启动事件循环
}

bool LampThread::openPort() {
    if (m_serialPort) {
        if (m_serialPort->isOpen()) {
            m_serialPort->clear();
            m_serialPort->close();
        }

        QString comName = Config::getInstance()->Get("wsConfig","comName").toString();
        if(comName.isEmpty()){

            comName = "ttyUSB0";
        }

        m_serialPort->setPortName(comName);
        if (!m_serialPort->open(QIODevice::ReadWrite)) {
            qDebug() << "打开失败!";
            return false;
        }
        qDebug() << "串口打开成功!";

        m_serialPort->setBaudRate(QSerialPort::Baud9600);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setStopBits(QSerialPort::OneStop);

//        connect(m_serialPort, &QSerialPort::readyRead, this, &LampThread::receiveInfo);
    }
    return true;
}

void LampThread::processTasks() {
//    std::function<void()> task;

    Task task;

     {
         QMutexLocker locker(&mutex);
         if (!taskQueue.isEmpty()) {
             task = taskQueue.takeFirst(); // 取出队列中的第一个任务
         }
     }

     if (task.func) {
         task.func(); // 执行任务
     }

     // 如果还有其他任务，设置下一个任务的定时器
     if (!taskQueue.isEmpty()) {
         taskTimer->start(taskQueue.first().delay); // 设置下一个任务的延迟
     } else {
         taskTimer->stop(); // 如果没有任务，停止定时器
     }
}

void LampThread::turnOnLamp(unsigned short keyId) {
    if (m_serialPort && m_serialPort->isOpen()) {
        QString sendData;
        switch (keyId) {
            case 8: sendData = "574B4C590900880880"; break;
            case 6: sendData = "574B4C59090088068E"; break;
            case 7: sendData = "574B4C59090088078F"; break;
            default: return; // 如果没有匹配的 keyId，直接返回
        }

        QByteArray ba = QByteArray::fromHex(sendData.toLatin1());
        qint64 bytesWritten = m_serialPort->write(ba);

        usleep(500 * 1000);


        if (bytesWritten == -1) {
            qDebug() << "Failed to write to port:" << m_serialPort->errorString();
        } else {
            qDebug() << "Bytes written:" << bytesWritten;
        }

        QDateTime currentDateTime = QDateTime::currentDateTime();
        qDebug() << "Worker::turnOnLamp" << "keyId:" << keyId << "sendData:" << sendData
                 << currentDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
    } else {
        qDebug() << "串口未打开，无法执行 turnOnLamp!";
    }
}

void LampThread::turnOffLamp(unsigned short keyId) {
    if (m_serialPort && m_serialPort->isOpen()) {
        QString sendData;
        switch (keyId) {
            case 8: sendData = "574B4C590900890881"; break;
            case 6: sendData = "574B4C59090089068F"; break;
            case 7: sendData = "574B4C59090089078E"; break;
            default: return; // 如果没有匹配的 keyId，直接返回
        }

        QByteArray ba = QByteArray::fromHex(sendData.toLatin1());
        qint64 bytesWritten = m_serialPort->write(ba);

        usleep(500 * 1000);


        if (bytesWritten == -1) {
            qDebug() << "Failed to write to port:" << m_serialPort->errorString();
        } else {
            qDebug() << "Bytes written:" << bytesWritten;
        }

        QDateTime currentDateTime = QDateTime::currentDateTime();
        qDebug() << "Worker::turnOffLamp" << "keyId:" << keyId << "sendData:" << sendData
                 << currentDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
    } else {
        qDebug() << "串口未打开，无法执行 turnOffLamp!";
    }
}




void LampThread::OpenOneKey(){
    if(m_serialPort->isOpen()){
    //    std::string data = utils->openOneKeyCommand(0,1);

        QString sendData = "574B4C590900820183";

        QByteArray ba;
        ba = QByteArray::fromHex(sendData.toLatin1());
        m_serialPort->write(ba);

        usleep(550 * 1000);

    }
}

void LampThread::receiveInfo() {
    if (m_serialPort) {
//        QByteArray info = m_serialPort->readAll();
//        QString result = byteArrayToHexStr(info);
//        qDebug() << "MainForm::receiveInfo()" << result;

//        if (utils->verifyKeyStatusData(result.toStdString())) {
//            unsigned short boardNum = utils->getBoardNumberFromRecvData(result.toStdString());
//            std::string stateData = result.toStdString();
//            lockStatusData[boardNum] = stateData;
//        }

            qDebug()<<"接收";
                QByteArray info = m_serialPort->readAll();
                QString result = byteArrayToHexStr(info);
                QString recvLockStatusData = byteArrayToHexStr(info);
        //        QString originText = ui->textEdit_2->toPlainText();
                result += "\n";
        //        result += originText;
        //        ui->textEdit_2->setText(result);
                qDebug()<<"MainForm::receiveInfo()"<<result;

                if(utils->verifyKeyStatusData(recvLockStatusData.toStdString())){
                    unsigned short boardNum = utils->getBoardNumberFromRecvData(recvLockStatusData.toStdString());
                    std::string stateData = recvLockStatusData.toStdString();
                    //lockStatusData.insert(std::make_pair(boardNum,stateData));
                    lockStatusData[boardNum] = stateData;
                }
    }
}

QString LampThread::byteArrayToHexStr(const QByteArray &ba) {
    QString hexStr;
    hexStr.reserve(ba.size() * 2);
    for (const char &ch : ba) {
        hexStr += QString::number(static_cast<unsigned char>(ch), 16).toUpper().rightJustified(2, '0');
    }
    return hexStr;
}

LampThread::~LampThread() {
    running = false; // 设置运行标志为 false
    if (m_serialPort) {
        m_serialPort->close(); // 关闭串口
        delete m_serialPort; // 释放串口资源
    }
    wait(); // 等待线程结束
}
