#ifndef CONFIG_H
#define CONFIG_H


#include <iostream>
#include <mutex>



#include <QVariant>
#include <QSettings>
#include <QDebug>
#include <QtCore/QtCore>
class Config
{
public:
    Config(QString qstrfilename = "");
    virtual ~Config(void);
    void Set(QString,QString,QVariant);


    void Sync();

    QVariant Get(QString,QString);


    static Config* getInstance() {
      if (_cfg_instance == nullptr) {
          std::lock_guard<std::mutex> lock(_cfg_mtx);  // 锁住临界区
          if (_cfg_instance == nullptr) {
              _cfg_instance = new Config();
          }
      }
      return _cfg_instance;
    }




private:
    QString m_qstrFileName;
    QSettings *m_psetting;
    void initializeConfig();
    QString findFFmpegPath();


    static Config* _cfg_instance;
    static std::mutex _cfg_mtx;

};


#endif // CONFIG_H
