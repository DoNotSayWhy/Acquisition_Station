#ifndef NETWORKUTILITY_H
#define NETWORKUTILITY_H

#include <QObject>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <config.h>
#include <audioplayer.h>

class NetworkUtility : public QObject
{
    Q_OBJECT
public:
    explicit NetworkUtility(QObject *parent = nullptr);
private:
    void sendNetworkRequest(const QUrl &url, const QJsonObject &jsonObject);
    void sendUpdateNetworkRequest(const QUrl &url, const QJsonObject &jsonObject);
    QString server_interface;
    void downloadZip(const QString &url);
    QNetworkAccessManager *manager;
    bool isVoicePlayback = false;
    int uploadNum = 3;
signals:
    void netdownloadStatustoMain(QString);
public slots:
    void netWorkClientAdd(const QVariantMap &copyTUserInfo);
    void netWorkClientEdit(const QVariantMap &updateUserInfo);
    void netWorkClientDelete(const QString& workstationIp, const QJsonArray& userList);
    void netWorkFileClientAdd(QList<QVariantMap> fileInfoList);
    void netWorkFileClientDelete(const QString& workstationIp, const QJsonArray& guidList);
    void netWorkClientUpdateModel(const QVariantMap &workStationInfo);
    void netWorkSendHeartbeat(double freedisksize,double totaldisksize,const QString &stationIp);
    void uploadUserInfoSyncData( const QString& workstationIp, const QJsonArray& userInfoList) ;
    void uploadFileSyncData( const QString& workstationIp, const QJsonArray& fileList);
    void netTOmain(QString version,QString workstationIp);

    void netWorkFileClientEdit(const QList<QMap<QString,QString>> &fileInfos);
};

#endif // NETWORKUTILITY_H
