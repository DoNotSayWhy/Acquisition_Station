#include "networkutility.h"
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>
#include <qjsonarray.h>
#include <QDateTime>
#include"config.h"
#include <QDesktopServices>
#include <qmessagebox.h>

NetworkUtility::NetworkUtility(QObject *parent) : QObject(parent), manager(new QNetworkAccessManager(this)) {

     server_interface =  Config::getInstance()->Get("runConfig", "serverUrl").toString();
     isVoicePlayback =  Config::getInstance()->Get("wsConfig", "isVoicePlayback").toBool();
     bool conversionOk;
     uploadNum =  Config::getInstance()->Get("wsConfig", "Upload").toInt(&conversionOk);

     // 根据 conversionOk 的值赋给 uploadNum
     if (!conversionOk) {
         uploadNum = 3; // 转换失败时赋值为 3
     }


}

void NetworkUtility::sendNetworkRequest(const QUrl &url, const QJsonObject &jsonObject)
{

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument jsonDocument(jsonObject);
    QByteArray jsonData = jsonDocument.toJson();

    QNetworkReply *reply = manager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [this, reply,url,jsonData]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();

            qDebug() << __FUNCTION__  << "Response from" << url << ":" << response << " , "  << QThread::currentThreadId();
        } else {

            qDebug() << __FUNCTION__  << url << "sendNetworkRequest Error:" <<  reply->errorString() <<  QString::fromUtf8(jsonData) << " "  << QThread::currentThreadId();

            if(isVoicePlayback){

                // QMessageBox::warning(nullptr, "Warning", "网络已断开。");
                QString audioFilePath = "./mp3/error.mp3"; // Change this to your audio file path

                // Create an instance of AudioPlayer
                AudioPlayer *audioPlayer = new AudioPlayer(audioFilePath, false); // Set loop to true if you want looping

                // Start the audio player thread
                audioPlayer->start();

                // Optionally, connect signals to handle completion or errors
                QObject::connect(audioPlayer, &QThread::finished, [&]() {
                    qDebug() << "  Audio playback finished.";

                    audioPlayer->deleteLater(); // Clean up the audio player
                    // audioPlayer->quit(); // Exit the application
                });

            }

        }
        reply->deleteLater();
    });

}

void NetworkUtility::netWorkClientAdd(const QVariantMap &userDataList)
{
//    qDebug() << "NetworkUtility::netWorkClientAdd start";

    QString url = server_interface + "/api/UserInfo/ClientAdd";
     qDebug() << "NetworkUtility::netWorkClientAdd start"<<url;

    QJsonObject jsonObject;
    jsonObject["F_UserNo"] = userDataList["F_UserNo"].toString();
    jsonObject["F_DeviceNo"] = userDataList["F_DeviceNo"].toString();
    jsonObject["F_UserName"] = userDataList["F_UserName"].toString();
    jsonObject["F_Department"] = userDataList["F_Department"].toString();
    jsonObject["F_WorkStationIp"] = userDataList["F_WorkStationIp"].toString();
    jsonObject["F_IsDelete"] = 0;
    jsonObject["F_Gender"] = "";
    jsonObject["F_Password"] = "";
    jsonObject["F_Phone"] = "";
    jsonObject["F_Remark"] = "";
    jsonObject["F_Status"] = 1;
    jsonObject["F_CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    jsonObject["F_UpdateDate"] =QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    sendNetworkRequest(QUrl(url), jsonObject);
}


void NetworkUtility::netWorkClientEdit(const QVariantMap &updateUserInfo)
{
//    qDebug() << "NetworkUtility::netWorkClientEdit start";
    QString url = server_interface + "/api/UserInfo/ClientEdit";

    QJsonObject jsonObject;
    jsonObject["Id"] = updateUserInfo.value("Id", 0).toInt();
    jsonObject["F_UserNo"] = updateUserInfo.value("F_UserNo", "").toString();
    jsonObject["F_DeviceNo"] = updateUserInfo.value("F_DeviceNo", "").toString();
    jsonObject["F_UserName"] = updateUserInfo.value("F_UserName", "").toString();
    jsonObject["F_Department"] = updateUserInfo.value("F_Department", "").toString();
    jsonObject["F_WorkStationIp"] = updateUserInfo.value("F_WorkStationIp", "").toString();
    jsonObject["F_IsDelete"] = updateUserInfo.value("F_IsDelete", 0).toInt();
    jsonObject["F_Gender"] = updateUserInfo.value("F_Gender", "").toString();
    jsonObject["F_Password"] = updateUserInfo.value("F_Password", "").toString();
    jsonObject["F_Phone"] = updateUserInfo.value("F_Phone", "").toString();
    jsonObject["F_Remark"] = updateUserInfo.value("F_Remark", "").toString();
    jsonObject["F_Status"] = updateUserInfo.value("F_Status", 1).toInt();
    jsonObject["F_CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    jsonObject["F_UpdateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    qDebug() << "NetworkUtility::netWorkClientEdit start :"<<jsonObject;
    sendNetworkRequest(QUrl(url), jsonObject);
}

void NetworkUtility::netWorkClientDelete(const QString& workstationIp, const QJsonArray& userList) {

    QString url = server_interface + "/api/UserInfo/ClientDelete";
    // 创建 JSON 数据
    QJsonObject payload;
    payload["workstationip"] = workstationIp;
    payload["list"] = userList;
//    qDebug()<<payload;

    sendNetworkRequest(QUrl(url), payload);
}


void NetworkUtility::netWorkFileClientAdd(QList<QVariantMap> fileInfoList)
{
//    qDebug()<< "netWorkFileClientAdd fileInfoList size:"<<fileInfoList.size();
    QString url = server_interface + "/api/File/ClientAdd";
    QSet<QString> seenFileNames; // 用于跟踪已处理的文件名
    QList<QVariantMap> uniqueFileInfoList; // 存储唯一的文件信息

    // 遍历原始列表，去重
    for (const QVariantMap &fileInfo : fileInfoList) {
        QString fileName = fileInfo.value("F_FileName", "").toString();

        // 检查文件名是否已经处理过
        if (!seenFileNames.contains(fileName)) {
            seenFileNames.insert(fileName); // 添加到已处理集合中
            uniqueFileInfoList.append(fileInfo); // 添加到唯一列表中
        }
    }

    // 发送网络请求
    for (const QVariantMap &fileInfo : uniqueFileInfoList) {
        QJsonObject jsonObject;
        jsonObject["F_Guid"] = fileInfo.value("F_Guid", "").toString();
        jsonObject["F_DeviceNo"] = fileInfo.value("F_DeviceNo", "").toString();
        jsonObject["F_UserNo"] = fileInfo.value("F_UserNo", "").toString();
        jsonObject["F_WorkStationIp"] = fileInfo.value("F_WorkStationIp", "").toString();
        jsonObject["F_FileName"] = fileInfo.value("F_FileName", "").toString();
        jsonObject["F_Type"] = fileInfo.value("F_Type", 0).toInt();
        jsonObject["F_FilePath"] = fileInfo.value("F_LocalStorageDriveLetter", "").toString();
        jsonObject["F_ServerStorageDriveLetter"] = fileInfo.value("F_ServerStorageDriveLetter", "").toString();
        jsonObject["F_LocalStorageDriveLetter"] = fileInfo.value("F_FilePath", "").toString();
        jsonObject["F_ShootTime"] = fileInfo.value("F_ShootTime", "").toString();
//        jsonObject["F_UploadTime"] = fileInfo.value("F_UploadTime", "").toString();
        jsonObject["F_UploadTime"] = (uploadNum == 3)
            ? fileInfo.value("F_UploadTime", "").toString()
            : QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        jsonObject["F_FileSize"] = fileInfo.value("F_FileSize", 0).toInt();
        jsonObject["F_FileTime"] = fileInfo.value("F_FileTime", 0).toInt();
        jsonObject["F_Thumbnail"] = fileInfo.value("F_Thumbnail", "").toString();
        jsonObject["F_IsImportant"] = fileInfo.value("F_IsImportant", 0).toInt();
        jsonObject["F_FtpUploadStatus"] = fileInfo.value("F_FtpUploadStatus", 0).toInt();
        jsonObject["F_IsEncrypt"] = fileInfo.value("F_IsEncrypt", 0).toInt();
        jsonObject["F_IsTranscode"] = fileInfo.value("F_IsTranscode", 0).toInt();
        jsonObject["F_CompStatus"] = fileInfo.value("F_CompStatus", 0).toInt();
        jsonObject["F_FileDataSyncStatus"] = fileInfo.value("F_FileDataSyncStatus", 0).toInt();
        jsonObject["F_Remark"] = fileInfo.value("F_Remark", "").toString();
        jsonObject["F_FileSource"] = fileInfo.value("F_FileSource", 0).toInt();
        jsonObject["F_IsDelete"] = fileInfo.value("F_IsDelete", 0).toInt();
        jsonObject["F_MarkId"] = fileInfo.value("F_MarkId", 0).toInt();

//        qDebug() << "NetworkUtility::netWorkFileClientAdd upload" << fileInfo.value("F_FileName", "").toString();
        sendNetworkRequest(QUrl(url), jsonObject);
    }
}

void NetworkUtility::netWorkFileClientEdit(const QList<QMap<QString, QString>> &fileInfos)
{
    QString url = server_interface + "/api/File/ClientEdit";

    // 遍历 fileInfos 集合
    for (const QMap<QString, QString> &fileData : fileInfos) {
        QJsonObject jsonObject;
        jsonObject["F_Guid"] = fileData.value("F_Guid", ""); // 文件MD5值，唯一标识
        jsonObject["F_WorkStationIp"] = fileData.value("F_WorkStationIp", ""); // 采集站IP
        jsonObject["F_MarkId"] = fileData.value("F_MarkId", "0").toInt(); // 手动标注ID，默认0
        jsonObject["F_Remark"] = fileData.value("F_Remark", ""); // 自定义备注

        // 确保 F_Guid 和 F_WorkStationIp 是不可空的
        if (jsonObject["F_Guid"].toString().isEmpty()) {
            qDebug() << "Error: F_Guid is required for one of the files.";
            continue; // 跳过当前文件，继续下一个
        }

        if (jsonObject["F_WorkStationIp"].toString().isEmpty()) {
            qDebug() << "Error: F_WorkStationIp is required for one of the files.";
            continue; // 跳过当前文件，继续下一个
        }

        // 调试输出 JSON 对象
//        qDebug() << "NetworkUtility::netWorkFileClientEdit sending for file:" << jsonObject;

        // 发送网络请求
        sendNetworkRequest(QUrl(url), jsonObject);
    }
}


void NetworkUtility::netWorkFileClientDelete(const QString& workstationIp, const QJsonArray& guidList) {

    QString url = server_interface + "/api/File/ClientDelete";
    QJsonObject payload;
    payload["workstationip"] = workstationIp;
    payload["list"] = guidList;

    sendNetworkRequest(QUrl(url), payload);
}

void NetworkUtility::netWorkClientUpdateModel(const QVariantMap &workStationInfo)
{
    //qDebug() << "NetworkUtility::netWorkClientUpdateModel start";
    QString url = server_interface + "/api/WorkStation/ClientUpdateModel";

    QJsonObject jsonObject;
    jsonObject["F_WorkStationNo"] = workStationInfo.value("F_WorkStationIp", "").toString();
    jsonObject["F_WorkStationName"] = workStationInfo.value("F_WorkStationIp", "").toString();
    jsonObject["F_WorkStationIp"] = workStationInfo.value("F_WorkStationIp", "").toString();
    jsonObject["F_DiskSize"] = workStationInfo.value("F_DiskSize", "").toInt();
    jsonObject["F_RemainSize"] = workStationInfo.value("F_RemainSize", "").toInt();

    sendNetworkRequest(QUrl(url), jsonObject);
}

void NetworkUtility::netWorkSendHeartbeat(double freedisksize,double totaldisksize,const QString &stationIp)
{

    QString url = server_interface + "/api/WorkStation/SendHeartbeat";
    // qDebug() << "NetworkUtility::netWorkSendHeartbeat start"<<stationIp<<" url:"<<url;

    QJsonObject jsonObject;
    jsonObject["F_WorkStationIp"] = stationIp;

    sendNetworkRequest(QUrl(url), jsonObject);
}


void NetworkUtility::uploadUserInfoSyncData( const QString& workstationIp, const QJsonArray& userInfoList) {

    QString url = server_interface + "/api/UserInfo/SyncData";
//    qDebug() << "NetworkUtility::netWorkSendHeartbeat start url:"<<url;


    QJsonObject payload;
    payload["workstationip"] = workstationIp;
    payload["userinfolist"] = userInfoList;

    sendNetworkRequest(QUrl(url), payload);
}


void NetworkUtility::uploadFileSyncData( const QString& workstationIp, const QJsonArray& fileList) {

    QString url = server_interface + "/api/File/SyncData";
//    qDebug() << "NetworkUtility::netWorkSendHeartbeat start url:"<<url;


    QJsonObject payload;
    payload["workstationip"] = workstationIp;
    payload["filelist"] = fileList;

    sendNetworkRequest(QUrl(url), payload);

}

void NetworkUtility::netTOmain(QString version,QString workstationIp)
{
    QString url = server_interface + "/api/CollectSoftVersion/GetCollectVersionPackage";
    qDebug()<<"NetworkUtility::netTOmain :"<<version;
    // Create JSON object
    QJsonObject json;
    json["F_VersionNo"] = version;
    json["F_WorkStationIp"] = workstationIp;


    sendUpdateNetworkRequest(QUrl(url), json);

}
void NetworkUtility::sendUpdateNetworkRequest(const QUrl &url, const QJsonObject &jsonObject)
{
//    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument jsonDocument(jsonObject);
    QByteArray jsonData = jsonDocument.toJson();

    QNetworkReply *reply = manager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [this,reply,url]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
            if (jsonResponse.isNull()) {
                qDebug() << "Failed to parse JSON:" << response;
            } else if (jsonResponse.isObject()) {
                QJsonObject jsonObject = jsonResponse.object(); // 转换为 QJsonObject
                // 访问 JSON 数据
                int code = jsonObject.value("code").toInt();
                if(code == 200){
                 QString  data = jsonObject.value("data").toString();
                 qDebug() << "data" << data;
//                 // 打开浏览器访问 data 链接
//                 QUrl downloadUrl(data);
//                 if (downloadUrl.isValid()) {
//                     QDesktopServices::openUrl(downloadUrl);
//                 } else {
//                     qDebug() << "Invalid URL:" << data;
//                 }
                 downloadZip(data);
                }else if(code == 500){
                 QString message = jsonObject.value("message").toString();
                 emit netdownloadStatustoMain("无需更新");
                 qDebug() << "message:" << message;
                }
            }
//            qDebug() <<url<<" : Response:" << response;
        } else {
           // qDebug()<<url << "sendNetworkRequest Error:" << reply->errorString();
        }
        reply->deleteLater();
//        manager->deleteLater();
    });
}
void NetworkUtility::downloadZip(const QString &url)
{
    QString wsConfig  = "/data/";
    QString HSASAppzip = wsConfig +"HSASAppzip";
    QDir destinationDir(HSASAppzip);
    // 如果目标目录不存在，则创建它
    if (!destinationDir.exists()) {
        if (!destinationDir.mkpath(".")) {
            qDebug() << "无法创建目标目录";
            return ;
        }
    }
    // 从 URL 获取文件名
    QString fileName = QFileInfo(QUrl(url).path()).fileName(); // 获取文件名
    QString savePath = HSASAppzip +"/"+ fileName; // 构建保存路径

    QUrl zipDownloadUrl(url);
    QNetworkRequest request(zipDownloadUrl);

    // 发起下载请求
    QNetworkReply *downloadReply = manager->get(request);
    emit netdownloadStatustoMain("正在下载");
    connect(downloadReply, &QNetworkReply::finished, this, [this, downloadReply, savePath]() {
        if (downloadReply->error() == QNetworkReply::NoError) {
            QByteArray zipData = downloadReply->readAll(); // 读取 ZIP 文件的数据
            QFile file(savePath); // 创建文件对象
            if (file.open(QIODevice::WriteOnly)) { // 打开文件以写入
                file.write(zipData); // 写入数据
                file.close(); // 关闭文件
                emit netdownloadStatustoMain("下载完成");
                qDebug() << "ZIP file downloaded successfully:" << savePath;
            } else {
                emit netdownloadStatustoMain("下载失败");
                qDebug() << "Failed to save ZIP file:" << file.errorString();
            }
        } else {
            qDebug() << "Download Error:" << downloadReply->errorString();
            emit netdownloadStatustoMain("下载失败");
        }
        downloadReply->deleteLater(); // 确保释放下载回复
    });
}
