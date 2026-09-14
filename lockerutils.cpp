#include "lockerutils.h"
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <iostream>
/**
 * @brief lockerutils::lockerutils
 * @param parent
 * 蓝灯常亮，验证通过绿灯闪一次，验证失败红灯闪一下
 * 检查拨码0
 * 检查是usb就使用ttyUSB0
 * 检查是com2 -> ttyS1
 * 如果1号位，是锁板，只能供一次电，持续会烧掉
 * 6号红灯
 * 7号绿灯
 * 8号蓝灯
 */
lockerutils::lockerutils(QObject *parent) : QObject(parent)
{

}

/*
 unsigned short 转到两位16进制的字符串
*/
std::string lockerutils::intToTwoHexString(unsigned short num) {
    std::stringstream stream;
    stream << std::setw(2) << std::setfill('0') << std::hex << num;
    return stream.str();
}

/*
 两位16进制字符串转换为一个整数
*/
int lockerutils::twoHexStringToInt(std::string hexString){

    int intValue = std::stoi(hexString, 0, 16);
    return intValue;
}

/*
计算校验位
*/
std::string lockerutils::checkXor(std::string hexString) {
    int checkData = 0;
    for(int i = 0;i < hexString.size(); i = i + 2){
        std::string tmp = hexString.substr(i,2);
        int param = twoHexStringToInt(tmp);
        checkData = param ^ checkData;
    }
    return intToTwoHexString(checkData);
}


/*
 打开一把锁的命令,例如:574B4C590900820183
 表示打开0号板上的1号锁
 boardNum:板号
 keyId:锁号
*/
std::string lockerutils::openOneKeyCommand(unsigned short boardNum,unsigned short keyId) {
    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keyIdStr = intToTwoHexString(keyId);
    std::ostringstream tmpStr;
    tmpStr << "574B4C5909" << boardNumStr << "82" << keyIdStr;
    std::string checkXorData = checkXor(tmpStr.str());
    std::string result = tmpStr.str().append(checkXorData);
    return result;
}

/*
 同时打开多把锁的命令
 boardNum:板号
 keyIdArray:锁号数组
*/
std::string lockerutils::openManyKeyCommand(unsigned short boardNum,std::vector<unsigned short> keyIdArray){

    std::string keyArrayStr = "";
    int keyArraySize = keyIdArray.size();
    for(int i = 0;i < keyArraySize; ++i){
        keyArrayStr += intToTwoHexString(keyIdArray.at(i));
    }

    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keySizeStr = intToTwoHexString(keyArraySize);
    std::ostringstream tmpStr;
    tmpStr << "574B4C59XX" << boardNumStr << "93" << keySizeStr << keyArrayStr;
    //replace the "XX"
    std::string orginOrderStr = tmpStr.str();
    int cmdLeng = orginOrderStr.length()/2 + 1;
    std::string cmdLengHexStr = intToTwoHexString(cmdLeng);
    orginOrderStr.replace(orginOrderStr.find("XX"),2,cmdLengHexStr);


    std::string checkXorData = checkXor(orginOrderStr);
    std::string result = orginOrderStr.append(checkXorData);
    return result;

}

/*
 按顺序打开多把锁的命令
 boardNum:板号
 keyIdArray:锁号数组
*/
std::string lockerutils::openManyOrderKeyCommand(unsigned short boardNum,std::vector<unsigned short> keyIdArray){
    std::string keyArrayStr = "";
    int keyArraySize = keyIdArray.size();
    for(int i = 0;i < keyArraySize; ++i){
        keyArrayStr += intToTwoHexString(keyIdArray.at(i));
    }

    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keySizeStr = intToTwoHexString(keyArraySize);
    std::ostringstream tmpStr;
    tmpStr << "574B4C59XX" << boardNumStr << "87" << keySizeStr << keyArrayStr;
    //replace the "XX"
    std::string orginOrderStr = tmpStr.str();
    int cmdLeng = orginOrderStr.length()/2 + 1;
    std::string cmdLengHexStr = intToTwoHexString(cmdLeng);
    orginOrderStr.replace(orginOrderStr.find("XX"),2,cmdLengHexStr);


    std::string checkXorData = checkXor(orginOrderStr);
    std::string result = orginOrderStr.append(checkXorData);
    return result;

}


/*
 一个通道打开电源:
 表示打开0号板上的1号通道打开电源
 boardNum:板号
 channelId:通道号
 1lock
 234 deng
*/
std::string lockerutils::getTurnOnCommand(unsigned short boardNum,unsigned short channelId) {
    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keyIdStr = intToTwoHexString(channelId);
    std::ostringstream tmpStr;
    tmpStr << "574B4C5909" << boardNumStr << "88" << keyIdStr;
    std::string checkXorData = checkXor(tmpStr.str());
    std::string result = tmpStr.str().append(checkXorData);
    return result;
}

/*
 打开多个持续通电的端口,如同时打开多个灯
 boardNum:板号
 channelId:通道号数组
*/
std::string lockerutils::getTurnOnManyCommand(unsigned short boardNum,std::vector<unsigned short> keyIdArray){
    std::string keyArrayStr = "";
    int keyArraySize = keyIdArray.size();
    for(int i = 0;i < keyArraySize; ++i){
        keyArrayStr += intToTwoHexString(keyIdArray.at(i));
    }

    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keySizeStr = intToTwoHexString(keyArraySize);
    std::ostringstream tmpStr;
    tmpStr << "574B4C59XX" << boardNumStr << "9501" << keySizeStr << keyArrayStr;
    //replace the "XX"
    std::string orginOrderStr = tmpStr.str();
    int cmdLeng = orginOrderStr.length()/2 + 1;
    std::string cmdLengHexStr = intToTwoHexString(cmdLeng);
    orginOrderStr.replace(orginOrderStr.find("XX"),2,cmdLengHexStr);


    std::string checkXorData = checkXor(orginOrderStr);
    std::string result = orginOrderStr.append(checkXorData);
    return result;

}


/*
 一个通道关闭电源:
 表示关闭0号板上的1号通道电源
 boardNum:板号
 channelId:通道号
*/
std::string lockerutils::getTurnOffCommand(unsigned short boardNum,unsigned short channelId) {
    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keyIdStr = intToTwoHexString(channelId);
    std::ostringstream tmpStr;
    tmpStr << "574B4C5909" << boardNumStr << "89" << keyIdStr;
    std::string checkXorData = checkXor(tmpStr.str());
    std::string result = tmpStr.str().append(checkXorData);
    return result;
}

/*
关掉多个持续通电的端口,如同时关掉多个灯
*/
std::string lockerutils::getTurnOffManyCommand(unsigned short boardNum,std::vector<unsigned short> channelIdArray){
    std::string keyArrayStr = "";
    int keyArraySize = channelIdArray.size();
    for(int i = 0;i < keyArraySize; ++i){
        keyArrayStr += intToTwoHexString(channelIdArray.at(i));
    }

    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keySizeStr = intToTwoHexString(keyArraySize);
    std::ostringstream tmpStr;
    tmpStr << "574B4C59XX" << boardNumStr << "9500" << keySizeStr << keyArrayStr;
    //replace the "XX"
    std::string orginOrderStr = tmpStr.str();
    int cmdLeng = orginOrderStr.length()/2 + 1;
    std::string cmdLengHexStr = intToTwoHexString(cmdLeng);
    orginOrderStr.replace(orginOrderStr.find("XX"),2,cmdLengHexStr);


    std::string checkXorData = checkXor(orginOrderStr);
    std::string result = orginOrderStr.append(checkXorData);
    return result;
}


std::string lockerutils::getOneLockerStateCommand(unsigned short boardNum,unsigned short keyId){
    std::string boardNumStr = intToTwoHexString(boardNum);
    std::string keyIdStr = intToTwoHexString(keyId);
    std::ostringstream tmpStr;
    tmpStr << "574B4C5909" << boardNumStr << "83" << keyIdStr;
    std::string checkXorData = checkXor(tmpStr.str());
    std::string result = tmpStr.str().append(checkXorData);
    return result;
}

//574B4C591200840008010000000000000096
std::string lockerutils::getAllLockerStateCommand(unsigned short boardNum){
    std::string boardNumStr = intToTwoHexString(boardNum);
    std::ostringstream tmpStr;
    tmpStr << "574B4C5908" << boardNumStr << "84" ;
    std::string checkXorData = checkXor(tmpStr.str());
    std::string result = tmpStr.str().append(checkXorData);
    return result;
}

bool lockerutils::verifyKeyStatusData(std::string recvData){
    bool result = false;
    if(recvData.length() > 9){
        //574B4C591200840008010000000000000096
        string startStr = recvData.substr(0,8);
        if(startStr.compare("574B4C59") == 0){
            //this message is the locker board message data.
            if(recvData.length() >= 18){
                //must be 84
                string cmdStr = recvData.substr(12,2);
                if(cmdStr.compare("84") == 0){
                    result = true;
                }
            }
        }
    }
    return result;
}

int lockerutils::getBoardNumberFromRecvData(std::string recvData){
    int result = -1;
    if(recvData.length() > 9){
        //574B4C591200840008010000000000000096
        string startStr = recvData.substr(0,8);
        if(startStr.compare("574B4C59") == 0){
            //this message is the locker board message data.
            if(recvData.length() >= 18){
                //must be 84
                string cmdStr = recvData.substr(12,2);
                if(cmdStr.compare("84") == 0){
                    string boardNumStr = recvData.substr(10,2);
                    result = twoHexStringToInt(boardNumStr);
                }
            }
        }
    }
    return result;
}


/*


*/
//574B4C591200840008010000000000000096
//keyId: base 1
//return 0:close,1:open
int lockerutils::getOneKeyStatus(std::unordered_map<unsigned short,std::string> allStatus,unsigned short boardNum,unsigned short keyId){
    int result = -1;
    unordered_map<unsigned short,std::string>::iterator it = allStatus.begin();
    std::string curBoardState = "";
    while (it != allStatus.end())
    {
    cout << (*it).first << " " << (*it).second << endl;
    if((*it).first == boardNum){
        curBoardState = (*it).second;
        break;
        }
    it++;
    }

    if(curBoardState.length() > 9){
        //574B4C591200840008010000000000000096
        string startStr = curBoardState.substr(0,8);
        if(startStr.compare("574B4C59") == 0){
            //this message is the locker board message data.
            if(curBoardState.length() >= 18){
                //must be 84
                string cmdStr = curBoardState.substr(12,2);
                if(cmdStr.compare("84") == 0){

                    string boardNumStr = curBoardState.substr(10,2);
                    string countStr = curBoardState.substr(16,2);
                    string lockerStateStr = curBoardState.substr(18);
                    int lastLen = lockerStateStr.length();

                    int count = twoHexStringToInt(countStr);
                    if(count == (lastLen -2)/2
                            && 0 <= (keyId - 1)
                            && (keyId -1) < count){
                        //right: it says the data is correct.
                        int index = keyId -1;
                        string stateValue = lockerStateStr.substr(2*index,2);
                        result = twoHexStringToInt(stateValue);
                    }
                }
            }
        }
    }
    return result;
}

/*
获取板子上有多少个通道
*/
int lockerutils::getBoardChannelCount(std::unordered_map<unsigned short,std::string> allStatus,unsigned short boardNum){
    int result = -1;
    unordered_map<unsigned short,std::string>::iterator it = allStatus.begin();
    std::string curBoardState = "";
    while (it != allStatus.end())
    {
    cout << (*it).first << " " << (*it).second << endl;
    if((*it).first == boardNum){
        curBoardState = (*it).second;
        break;
        }
    it++;
    }

    if(curBoardState.length() > 9){
        //574B4C591200840008010000000000000096
        string startStr = curBoardState.substr(0,8);
        if(startStr.compare("574B4C59") == 0){
            //this message is the locker board message data.
            if(curBoardState.length() >= 18){
                //must be 84
                string cmdStr = curBoardState.substr(12,2);
                if(cmdStr.compare("84") == 0){

                    string boardNumStr = curBoardState.substr(10,2);
                    string countStr = curBoardState.substr(16,2);
                    string lockerStateStr = curBoardState.substr(18);

                    int count = twoHexStringToInt(countStr);
                    result = count;
                }
            }
        }
    }
    return result;
}
