#ifndef MYSQLLITE_H
#define MYSQLLITE_H
#include <qdebug.h>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include<QSqlTableModel>
#include<QDateTime>
#include <QObject>
#include<zfycontrol.h>
#include<config.h>
#include <memory>
#include "IdFaceSdk.h"
#include <vector>


#include "hsglobal.h"

using namespace std;

struct FileInfo{
    QString file_uuid;
    QString F_Guid;
    QString file_name;
    QString file_date;
    QString file_upload_time;
    QString file_driverid;
    QString file_remark;
    QString file_userid;
    QString filepath;
    QString file_shortpath;
    QString file_est;
    // 文件类型
    QString F_Type ;
    //人员编号
    QString F_UserNo;
    //设备编号
    QString F_DeviceNo;
    //姓名
    QString F_UserName;
    //拍摄时间
    QString F_ShootTime;
    //上传时间
    QString F_UploadTime;
    //重要文件
    QString F_IsImportant;
    //播放
    //备注
    QString F_Remark;
    QString F_WorkStationIp;

    QString F_MarkId;

    QString F_LocalStorageDriveLetter;
    QString F_IMP_VALUES;
    QString F_MARKID_VALUES;

};
struct DepartmentInfo{
    QString id;
    QString f_name;
    QString f_parentId;
    QString f_code;
    QString f_remark;
    QString f_isDelete;
};
struct LogInfo {
    QString id;              // Id
    QString f_description;   // F_Description
    QString f_loginName;     // F_LoginName
    QString f_logType;       // F_LogType
    QString f_moduleName;    // F_ModuleName
    QString f_realName;      // F_RealName
    QString f_isDelete;      // F_IsDelete
    QString createdAt;       // CreatedAt
};
struct NewUserInfo{
    QString id;
    QString F_UserNo;
    QString F_DeviceNo;
    QString F_UserName;
    QString F_DepartmentId;
    QString f_name;
    QString F_WorkStationIp;
    QString F_WorkStationId;
    QString F_IsDelete;
    QString F_Gender;
    QString F_Password;
    QString F_Phone;
    QString F_Remark;
    QString F_Status;
    QString create_time;
    QString F_RoleId;
    QString departmentName;

};

struct FileAllInfo{
    QString file_uuid;
    QString file_name;
    QString user_id;
    QString driver_id;
    QString file_caseNo;
    QString fileshowpath;
    QString file_gps;
    QDateTime create_time;
    QDateTime upload_time;
    qint64 file_size;
    int file_type;
    int save_max_day;
    int is_important;

};
struct FaceFeature {
    QString userNo; // 用户编号
    QByteArray featureData; // 特征数据
};
Q_DECLARE_METATYPE(FileAllInfo);
struct UserInfo{
    int user_id;
    QString user_name;
    QString user_photo_url;
};

struct FingerFeatureData{
    QString userNo;
    QString userName;
    QByteArray featureData;
    QString ext1;
    QString ext2;
};

class MySqlLite :public QObject
{
    Q_OBJECT
public:
    explicit MySqlLite(QObject *parent = nullptr);
    ~MySqlLite();
    QSqlDatabase    sqliteDatabase;
    void openmysql();
    void opensqlite();


    int dbsType ;
    QTimer * keepTimer =nullptr;

    void getFiles(const QString &folderPath,QStringList &selectfiles);
    int insertOrUpdateDept(QString parentId, QString newDepartmentName);
    NewUserInfo getUserInfoByUserNo(QString userNo);
    bool isValidUserName(const QString &userName) ;

private:
    QString fileName ;
    QSqlQuery *sql_query =nullptr;
    void gettb_files_audio(int type_jur,QString userid);
    QString getFileTypeDescription(QString fileType);
    QString getIsImportantDescription(QString import);
    void gettb_files_onlyme(int type_jur,QString userid);
    void gettb_files_all(int type_jur,QString userid);
    bool isDeleteZFYData =false;
    void gett_department_all(QString userid);
    void getPermissiont_department(QString userid);
    void gett_department_onlyme(QString userid);
    void gett_userinfo_all(QString userid,QString userNumber,QString devNumber);

    void getPermissiont_userinfo(QString userid,QString userNumber,QString devNumber);

    void gett_userinfo_onlyme(QString userid,QString userNumber,QString devNumber);

    FileInfo fillFileInfo(QSqlQuery& query);

    DepartmentInfo fillDepartmentInfo(QSqlQuery& query);

    std::vector<FingerFeatureData> mAllFingerFeature;
    void createTableIfNotExists(const QString &tableName, const QString &query);
    void initTables();

    bool isUploadEnabled = true;
    QString getFMarkIdDescription(QString import);


    void get_realdb_files_onlyme(QString userid,QString filetype ,QString pernumber,QString pername, QString significance,
                                 QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend,QString  dateEditstart_create , QString  dateEditend_create,int maxRowsToDisplay,int page);
    void get_realdb_files_all(QString userid,QString filetype ,QString pernumber,QString pername, QString significance,
                              QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend,QString  dateEditstart_create , QString  dateEditend_create,int maxRowsToDisplay,int page);

    QString  makefiletype(QString value ){

        // 全部   视频    音频  图片  文本日志  其他
        QString lins = "";

        if (value == "视频") {
            lins = "1";
        }
        else if (value == "音频") {
           lins = "2";
        }
        else if (value == "图片") {
             lins = "3";
        }
        else if (value == "文本日志") {
            lins = "'4'";
        }
        else if (value == "其他") {
            lins = "5";
         }
        else {
            lins = "1,2,3,4,5";
        }

        return lins;
    }

    QString  makesignificance(QString value ){

        // 全部   视频    音频  图片  文本日志  其他
        QString lins = "";

        if (value == "标注不重要") {
            lins = "0";
        }
        else if (value == "标注重要") {
           lins = "1";
        } else {
            lins = "";
        }

        return lins;
    }



public:
    void insertFingerFeature(FingerFeatureData data);
    void deleteFingerFeature(QString userNo);
    void updateFingerFeature(QString userNo,FingerFeatureData data);
    void getFingerFeatureAll();
    std::vector<FingerFeatureData> getFingerFeatureList();
    FingerFeatureData queryFingerFeature(QString userNo);
    void deleteFingerFeatureList(QStringList userNoList);




public slots:
    void selectt_filedata(const QString &filePath,int winNum);
    void sloMQSelectUserNo(const QString &username,const QString &password);
    void getPermissionData(const QString &userDeviceNo);
    void slotDelet_tfilebyid(QString workstationIp,QList<QMap<QString,QString>> tfileIds);
    void insertT_UserInfoCTtoMSQ(const QVariantMap &copyTUserInfo);
    void doinsertfiles(const QList<QVariantMap>& fileList,bool sourceFilePathIsErr,const QString &sourceDirFather,int windowNum);

    void get_tb_files_type(int type_jur,QString userid,QString type);


    void get_realdb_files_type( QString roleId,QString usrNO ,QString filetype ,QString pernumber,QString pername, QString significance,
                            QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend,
                            QString  dateEditstart_create , QString  dateEditend_create,int maxRowsToDisplay , int page);

    void  get_realdb_PermissionData(  QString userid,QString filetype ,QString pernumber,QString pername, QString significance,
                                    QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend, QString  dateEditstart_create , QString  dateEditend_create,int maxRowsToDisplay,int page);


    void MSQSelectAllDeptManageTODEP(QString userid,QString roleId);
    void MSQinsertDeptManageTODEP(QString parentId,QString newDepartmentName);
    void MSQUpdatet_departmentTODEP(QString parentId,QString newDepartmentName,QString departmentId);
    void MSQDeletet_departmentTODEP(QString departmentId);
    void MSQSelectt_userinfoALLTOUSER(QString userid,QString roleId,QString userNumber,QString devNumber);
    void MSQSelectAllDeptManageTOUSER(QString userid, QString roleId,int statu);
    void MSQInsertt_userinfoToUSER(const QMap<QString, QVariant>& userInfoMap);
    void BatchInsertt_userinfoToUSER(QList<QMap<QString, QVariant>> userInfoList);
    void MSQupdatet_userInfoToUSER(const QMap<QString, QVariant>& userInfoMap);
    void MSQRdelett_userinfo_byidtoUSER(QString getUserFormSystemIPV4,QList<QMap<QString, QString>> userInfoList);
    void MSQSelectT_logAllTOLog(QString userid,QString roleId);
    void insertt_logformainF(QString userNo,int type);
    void MSQselectWeekUserInfoTOMainF(QString ipv4);
    void MSQselectWeekFileInfoTOMainF(QString ipv4);
    void saveFaceByUserNoForFaceForm(QString userNo,QByteArray face,int nFeatureSize);
    void fetchAllFaceFeaturesTOFaceFrom();
    void delete_by_saveday(int day);
    void slotImportant_tfilebyid(QString workstationIp,QList<QMap<QString,QString>> tfileIds);
    void getUserListCAR_DEPOT();
    void slotRemark_tfilebyid(const QMap<QString, QString>& file);
    void setPARAMRelevance(const QString &path,const QString &polNo);

    void doQueryFilesById(const QStringList& ids);


    void slotAutoUpload_tfilebyid(QString workstationIp,QList<QMap<QString,QString>> tfileIds);

    void UploadFilebyTime(QString uploadDateTime);


    void gett_gather_userinfo_all();


    void slotRename_tfilebyid(const QMap<QString, QString> &file);
private:
    void gett_logAll_all(QString userid,QString roleId);
    void getPermissiont_log(QString userid,QString roleId);

    void gett_log_onlyme(QString userid,QString roleId);
    const char* logTypeToString(int type);


signals:
    void gettb_files_success(QList<FileInfo>);
    void gettb_user_success(QList<UserInfo>);
    void insertsuccess(QVariant dataVar);
    void insertsuccesstest();
    void insertlogsuccess(int type,QVariantMap map);
    void sendConfirmtoSerVer(int status,QString Taskid);
    void sig_filenocopy(QFileInfoList filenocopy,int num);
    void readyToCopyFile(const QString &filePath,int winNum);
    void test();
    void deleteThreadMQ(int windowNum);
    void onCopyFinishedMQ(const QString &path,int windowNum);
    void waitUnplugUsbDevice(const QString &path,int windowNum);
    void sourceFileErrUnplugUsbDevice(const QString &path,int windowNum);
    void mysqlClientAdd(const QVariantMap &copyTUserInfo);
    void mysqlWorkFileClientAdd(QList<QVariantMap> fileInfoList);
    void MQSelectUserNoSuc(const QString &username,const QString &roleId);
    void gett_department_success(QList<DepartmentInfo>);
    void gett_userinfo_success(QList<NewUserInfo>);
    void department_successTOUSER(QList<DepartmentInfo>,int stauts);
    void departmentforupdateTOUSER(QList<DepartmentInfo>);
    void t_log_successTOLog(QList<LogInfo> log_alls);
    void mysqlUserInfo_ClientEditTONET(const QVariantMap &copyTUserInfo);
    void mysqlUserInfo_ClientDeleteTONET(const QString& workstationIp, const QJsonArray& userList);
    void MSQFile_ClientDelete_TONET(const QString& workstationIp, const QJsonArray& guidList);
    void MSQselectWeekUserInfoTONet( const QString& workstationIp, const QJsonArray& userInfoList);
    void MSQselectWeekFileInfoTONet(const QString& workstationIp, const QJsonArray& fileList);
    void batchInsertUserInfoListResult(int count);
    void allFaceFeaturesTOFaceR(std::vector<FaceFeature> faceArray);
    void UserListCAR_DEPOT(const QStringList &userList);
    void loginError();


    void realdb_files_success(QList<FileInfo>);

};

#endif // MYSQLLITE_H
