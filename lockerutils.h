#ifndef LOCKERUTILS_H
#define LOCKERUTILS_H

#include <QObject>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class lockerutils : public QObject
{
    Q_OBJECT
public:
    explicit lockerutils(QObject *parent = nullptr);

signals:

public slots:

private:
    std::string intToTwoHexString(unsigned short num);
    int twoHexStringToInt(std::string hexString);
    std::string checkXor(std::string hexString);

public:
    //开锁命令
    std::string openOneKeyCommand(unsigned short boardNum,unsigned short keyId);
    std::string openManyKeyCommand(unsigned short boardNum,std::vector<unsigned short> keyIdArray);
    std::string openManyOrderKeyCommand(unsigned short boardNum,std::vector<unsigned short> keyIdArray);

    //查询锁状态命令
    std::string getOneLockerStateCommand(unsigned short boardNum,unsigned short keyId);
    std::string getAllLockerStateCommand(unsigned short boardNum);

    //开灯命令
    std::string getTurnOnCommand(unsigned short boardNum,unsigned short channelId);
    std::string getTurnOnManyCommand(unsigned short boardNum,std::vector<unsigned short> channelIdArray);

    //关灯命令
    std::string getTurnOffCommand(unsigned short boardNum,unsigned short channelId);
    std::string getTurnOffManyCommand(unsigned short boardNum,std::vector<unsigned short> channelIdArray);

    int getBoardNumberFromRecvData(std::string recvData);
    bool verifyKeyStatusData(std::string recvData);
    int getOneKeyStatus(std::unordered_map<unsigned short,std::string> allStatus,unsigned short boardNum,unsigned short keyId);
    int getBoardChannelCount(std::unordered_map<unsigned short,std::string> allStatus,unsigned short boardNum);
};

#endif // LOCKERUTILS_H
