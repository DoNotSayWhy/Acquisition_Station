#include "locwdget.h"

#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include "ui_locwdget.h"
#include "lockerutils.h"
#include <QSerialPort>
#include <QSerialPortInfo>


void charArrayToHex(unsigned char *array, int size, char *hex, int hexSize) {
    for (int i = 0; i < size; i++) {
        sprintf(hex + i * 2, "%02X", array[i]);
    }
}

locwdget::locwdget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::locwdget)
{
//    ui->setupUi(this);
      setWindowTitle("锁板控制-hskj");
      ui->textEdit->setText("574B4C590900820183");
      pQueryThread = new autoquerythread();
      utils = new lockerutils();
      //实例化一个串口对象
      m_serialPort = new QSerialPort();
      //获取可用的串口号
      foreach(const QSerialPortInfo info,QSerialPortInfo::availablePorts())
      {
          qDebug()<< "Port name:" << info.portName();
              ui->comboBoxSerialPort->addItem(info.portName());
      }

      QStringList boardList;
      boardList.append("0");
      boardList.append("1");
      boardList.append("2");
      boardList.append("3");
      boardList.append("4");
      ui->comboBoxBoardNum->addItems(boardList);
      ui->comboBoxBoardNum->setCurrentIndex(0);

      QStringList keyIdList;
      for(int i = 0;i < 30;++i){
          keyIdList.append(QString::number(i+1));
      }
      ui->comboBoxKeyId->addItems(keyIdList);
      ui->comboBoxKeyId->setCurrentIndex(0);

      connectOkOrNot(false);
  }

  locwdget::~locwdget()
  {
      delete ui;
      if(m_serialPort->isOpen()){
          m_serialPort->close();
      }
  }

  void locwdget::openSerialPort(){
      //ProcessSerialPort();
      openPort();
  }


  int locwdget::ProcessSerialPort() {

      return 0;
  }

  void locwdget::openPort(){
      //如果串口已经打开了先给他关闭了
      if(m_serialPort->isOpen())
      {
          m_serialPort->clear();
          m_serialPort->close();
      }

      //当前选择的串口名字ttyS1
      m_serialPort->setPortName(ui->comboBoxSerialPort->currentText());

      if(!m_serialPort->open(QIODevice::ReadWrite))//用ReadWrite的模式尝试打开串口
      {
          connectOkOrNot(false);
          qDebug()<<"打开失败!";
          return;
      }
      qDebug()<<"串口打开成功!";

      m_serialPort->setBaudRate(QSerialPort::Baud9600,QSerialPort::AllDirections);//设置波特率和读写方向
      m_serialPort->setDataBits(QSerialPort::Data8);//数据位为8位
      m_serialPort->setFlowControl(QSerialPort::NoFlowControl);//无流控制
      m_serialPort->setParity(QSerialPort::NoParity);//无校验位
      m_serialPort->setStopBits(QSerialPort::OneStop);//一位停止位

      //手动绑定槽函数
      connect(m_serialPort,SIGNAL(readyRead()),this,SLOT(receiveInfo()));

      connect(pQueryThread,SIGNAL(queryLockStatus()),this,SLOT(queryLockStatus()));

      pQueryThread->start();
      connectOkOrNot(true);

  }

  void locwdget::connectOkOrNot(bool isConnect){
      ui->pushButtonClear->setEnabled(true);
      ui->pushButtonOpenSerialPort->setEnabled(!isConnect);

      ui->pushButton->setEnabled(isConnect);
      ui->pushButtonOpenOne->setEnabled(isConnect);
      ui->pushButtonOpenOne_2->setEnabled(isConnect);
      ui->pushButtonOpenOne_3->setEnabled(isConnect);
      ui->pushButtonOpenOne_4->setEnabled(isConnect);
      ui->pushButtonOpenOne_5->setEnabled(isConnect);
      ui->pushButtonOpenOne_6->setEnabled(isConnect);
      ui->pushButtonOpenOne_7->setEnabled(isConnect);
      ui->pushButtonSend->setEnabled(isConnect);
  }

  QString locwdget::byteArrayToHexStr(const QByteArray &ba) {
      QString hexStr;
      hexStr.reserve(ba.size() * 2);
      for (const char &ch : ba) {
          hexStr += QString::number(static_cast<unsigned char>(ch), 16).toUpper().rightJustified(2, '0');
      }
      return hexStr;
  }

  void locwdget::queryLockStatus(){
      if(m_serialPort->isOpen()){
          std::string data8 = utils->getAllLockerStateCommand(0);

          QString sendData = QString::fromStdString(data8);

          QByteArray ba;
          ba = QByteArray::fromHex(sendData.toLatin1());
          m_serialPort->write(ba);
      }
  }

  //接收到单片机发送的数据进行解析
  void locwdget::receiveInfo()
  {    qDebug()<<"接收";
          QByteArray info = m_serialPort->readAll();
          QString result = byteArrayToHexStr(info);
          QString recvLockStatusData = byteArrayToHexStr(info);
          QString originText = ui->textEdit_2->toPlainText();
          result += "\n";
          result += originText;
          ui->textEdit_2->setText(result);

          if(utils->verifyKeyStatusData(recvLockStatusData.toStdString())){
              unsigned short boardNum = utils->getBoardNumberFromRecvData(recvLockStatusData.toStdString());
              std::string stateData = recvLockStatusData.toStdString();
              //lockStatusData.insert(std::make_pair(boardNum,stateData));
              lockStatusData[boardNum] = stateData;
          }

      //qDebug()<<"receive info:"<write("0x55");
      //m_serialPort->write("0xaa");
  }





  void locwdget::pushButtonSend(){
      /*
      //only a test.
      QString sendData = "";

      std::vector<unsigned short> keyArray;
      keyArray.push_back(1);
      keyArray.push_back(3);
      keyArray.push_back(6);
      keyArray.push_back(5);
      std::string data = utils->openOneKeyCommand(0,1);
      std::string data2 = utils->openManyOrderKeyCommand(0,keyArray);


      std::vector<unsigned short> channelIdArray;
      channelIdArray.push_back(4);
      channelIdArray.push_back(6);
      channelIdArray.push_back(5);

      std::string data3 = utils->getTurnOnCommand(0,3);
      std::string data4 = utils->getTurnOnManyCommand(0,channelIdArray);
      std::string data5 = utils->getTurnOffCommand(0,3);
      std::string data6 = utils->getTurnOffManyCommand(0,channelIdArray);


      std::string data7 = utils->getOneLockerStateCommand(0,1);
      std::string data8 = utils->getAllLockerStateCommand(0);

      sendData = QString::fromStdString(data2);

      QByteArray ba;
      ba = QByteArray::fromHex(sendData.toLatin1());
      m_serialPort->write(ba);
  */


      QString sss = "574B4C590900820183";
      QString text = ui->textEdit->toPlainText();
      if(text.isNull() || text.length() == 0){
          text = sss;
      }
      QByteArray ba;
      ba = QByteArray::fromHex(text.toLatin1());

      m_serialPort->write(ba);

  }

  void locwdget::pushButtonClear(){
      ui->textEdit_2->setText("");
      ui->labelSingleStatus->setText("");
  }


  void locwdget::pushButtonOpenOneKey(){

      QVariant boardNum = ui->comboBoxBoardNum->currentText();
      QVariant keyId = ui->comboBoxKeyId->currentText();

      unsigned short boardNo = 0;
      unsigned short keyNo = 1;
      if(boardNum.canConvert<unsigned short>()){
          boardNo = boardNum.value<unsigned short>();
      }
      if(keyId.canConvert<unsigned short>()){
          keyNo = keyId.value<unsigned short>();
      }

      std::string data = utils->openOneKeyCommand(boardNo,keyNo);

      QString sendData = QString::fromStdString(data);

      QByteArray ba;
      ba = QByteArray::fromHex(sendData.toLatin1());
      m_serialPort->write(ba);

  }
  void locwdget::pushButtonOpenOrderAllKey(){
      QVariant boardNum = ui->comboBoxBoardNum->currentText();
      QVariant keyId = ui->comboBoxKeyId->currentText();
      unsigned short boardNo = 0;
      unsigned short keyNo = 1;
      if(boardNum.canConvert<unsigned short>()){
          boardNo = boardNum.value<unsigned short>();
      }
      if(keyId.canConvert<unsigned short>()){
          keyNo = keyId.value<unsigned short>();
      }

      int channelCount = utils->getBoardChannelCount(lockStatusData,boardNo);
      std::vector<unsigned short> keyArray;
      for(int i = 0;i < channelCount;++i){
          keyArray.push_back(i+1);
      }

      std::string data = utils->openManyOrderKeyCommand(boardNo,keyArray);
      QString sendData = QString::fromStdString(data);
      QByteArray ba;
      ba = QByteArray::fromHex(sendData.toLatin1());
      m_serialPort->write(ba);

  }
  void locwdget::pushButtonOpenAllOnce(){
      QVariant boardNum = ui->comboBoxBoardNum->currentText();
      QVariant keyId = ui->comboBoxKeyId->currentText();
      unsigned short boardNo = 0;
      unsigned short keyNo = 1;
      if(boardNum.canConvert<unsigned short>()){
          boardNo = boardNum.value<unsigned short>();
      }
      if(keyId.canConvert<unsigned short>()){
          keyNo = keyId.value<unsigned short>();
      }
      int channelCount = utils->getBoardChannelCount(lockStatusData,boardNo);
      std::vector<unsigned short> keyArray;
      for(int i = 0;i < channelCount;++i){
          keyArray.push_back(i+1);
      }

      std::string data = utils->openManyKeyCommand(boardNo,keyArray);
      QString sendData = QString::fromStdString(data);
      QByteArray ba;
      ba = QByteArray::fromHex(sendData.toLatin1());
      m_serialPort->write(ba);

  }
  void locwdget::pushButtonOpenLamp(){
      QVariant boardNum = ui->comboBoxBoardNum->currentText();
      QVariant keyId = ui->comboBoxKeyId->currentText();
      unsigned short boardNo = 0;
      unsigned short keyNo = 1;
      if(boardNum.canConvert<unsigned short>()){
          boardNo = boardNum.value<unsigned short>();
      }
      if(keyId.canConvert<unsigned short>()){
          keyNo = keyId.value<unsigned short>();
      }

      int channelCount = utils->getBoardChannelCount(lockStatusData,boardNo);
      std::vector<unsigned short> keyArray;
      for(int i = 0;i < channelCount;++i){
          keyArray.push_back(i+1);
      }

      std::string data = utils->getTurnOnCommand(boardNo,keyNo);
      QString sendData = QString::fromStdString(data);
      QByteArray ba;
      ba = QByteArray::fromHex(sendData.toLatin1());
      m_serialPort->write(ba);

  }
  void locwdget::pushButtonCloseLamp(){
      QVariant boardNum = ui->comboBoxBoardNum->currentText();
      QVariant keyId = ui->comboBoxKeyId->currentText();
      unsigned short boardNo = 0;
      unsigned short keyNo = 1;
      if(boardNum.canConvert<unsigned short>()){
          boardNo = boardNum.value<unsigned short>();
      }
      if(keyId.canConvert<unsigned short>()){
          keyNo = keyId.value<unsigned short>();
      }
      int channelCount = utils->getBoardChannelCount(lockStatusData,boardNo);
      std::vector<unsigned short> keyArray;
      for(int i = 0;i < channelCount;++i){
          keyArray.push_back(i+1);
      }

      std::string data = utils->getTurnOffCommand(boardNo,keyNo);
      QString sendData = QString::fromStdString(data);
      QByteArray ba;
      ba = QByteArray::fromHex(sendData.toLatin1());
      m_serialPort->write(ba);

  }
  void locwdget::pushButtonQuerySingleKey(){
      QVariant boardNum = ui->comboBoxBoardNum->currentText();
      QVariant keyId = ui->comboBoxKeyId->currentText();
      unsigned short boardNo = 0;
      unsigned short keyNo = 1;
      if(boardNum.canConvert<unsigned short>()){
          boardNo = boardNum.value<unsigned short>();
      }
      if(keyId.canConvert<unsigned short>()){
          keyNo = keyId.value<unsigned short>();
      }
      int keyStatus = utils->getOneKeyStatus(lockStatusData,boardNo,keyNo);
      QString s;
      if(keyStatus == 0){
          s = "关";
      }else if(keyStatus == 1){
          s = "开";
      }else{
          s = "未知";
      }
      QString str;
      str.sprintf("板号：%d,锁号：%d,状态：%s", boardNo,keyNo, s.toStdString().c_str());
      ui->labelSingleStatus->setText(str);

  }
  void locwdget::pushButtonQueryAllKey(){

  }

  void locwdget::pushButtonDisconnect(){
      if(m_serialPort->isOpen()){
          m_serialPort->close();
      }
  }




