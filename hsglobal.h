#ifndef HSGLOBAL_H
#define HSGLOBAL_H

extern bool is_mainwindow_exited;

extern bool is_standard_sign;

#include <mutex>
#include <map>

#include <QList>

constexpr int kMaxWorkstationPorts = 999;

extern std::mutex hsGlobalMtx;


extern std::map<int, std::string> hsBusMap;
extern std::mutex hsBusMtx;

extern std::mutex hsDbGlobalMtx;

struct NewUserInfo ;
extern QList<NewUserInfo> global_userinfo_all;

#define GET_CLASS_NAME() (typeid(*this).name())

#endif // HSGLOBAL_H
