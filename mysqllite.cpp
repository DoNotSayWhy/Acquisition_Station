#include "mysqllite.h"
#include<QMessageBox>
#include <QRegularExpression>

MySqlLite::MySqlLite(QObject *parent) : QObject(parent)
{

     fileName =  Config::getInstance()->Get("path", "HSdbpath").toString();
     isDeleteZFYData =  Config::getInstance()->Get("wsConfig","deleteFile").toBool();


     bool conversionOk;
     int uploadNum =  Config::getInstance()->Get("wsConfig", "Upload").toInt(&conversionOk);

     // 定义布尔变量
     // 根据 uploadNum 的值赋值给布尔变量
     if (!conversionOk || uploadNum == 0 || uploadNum == 1) {
         // 如果转换失败或 uploadNum 是 0，设置为 false
         isUploadEnabled = false;
     } else if(uploadNum == 2){
         // 如果 uploadNum 是 1 或其他值，设置为 true
         isUploadEnabled = true;
     }
//     isDeleteZFYData = false;

     int dataBasesType = 0;
     dataBasesType =  Config::getInstance()->Get("wsConfig", "dataType").toInt();

     // dataBasesType = 1;
     dbsType = dataBasesType;

     if(dataBasesType == 0 ){

         opensqlite();

     }else{
         openmysql();
     }

     initTables();
}


MySqlLite::~MySqlLite() {
    if (sqliteDatabase.isOpen()) {
        sqliteDatabase.close();
    }
    if (sql_query) {
        delete sql_query;
    }
}

void MySqlLite::openmysql(){



    // 创建数据库连接
    //QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    // 设置连接参数
    //db.setHostName("localhost");     // 数据库主机
    //db.setDatabaseName("your_database_name");  // 数据库名
    //db.setUserName("your_username");  // 数据库用户名
    //db.setPassword("your_password");  // 数据库密码

    // qDebug()<< __FUNCTION__ << " =========== MySql QMYSQL ============ 所在线程："<< QThread::currentThreadId();

    sqliteDatabase = QSqlDatabase::addDatabase("QMYSQL");

    // 数据库主机
    QString  HostName =   Config::getInstance()->Get("wsConfig","HostName").toString();
    HostName = HostName.isEmpty() ? "127.0.0.1" : HostName;

    // 数据库名
    QString  DatabaseName =   Config::getInstance()->Get("wsConfig","DatabaseName").toString();
    DatabaseName = DatabaseName.isEmpty() ? "hszhifayi" : DatabaseName;

    // 数据库用户名
    QString  UserName =   Config::getInstance()->Get("wsConfig","UserName").toString();
    UserName = UserName.isEmpty() ? "root" : UserName;

    // 数据库密码
    QString  Password =   Config::getInstance()->Get("wsConfig","Password").toString();

     // 设置连接参数
     sqliteDatabase.setHostName(HostName);     // 数据库主机
     sqliteDatabase.setDatabaseName(DatabaseName);  // 数据库名
     sqliteDatabase.setUserName(UserName);  // 数据库用户名
     sqliteDatabase.setPassword(Password);  // 数据库密码


    try{

        bool openFlag = sqliteDatabase.open();
        if (!openFlag) {
            qDebug() << "open database file err";
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::openmysql 捕获到异常: ============================ " << e.what() << std::endl;
    }

    if (sql_query) {
        delete sql_query;
    }
    sql_query = new QSqlQuery(sqliteDatabase);



}



void MySqlLite::opensqlite(){


    sqliteDatabase = QSqlDatabase::addDatabase("QSQLITE");
    sqliteDatabase.setDatabaseName( Config::getInstance()->Get("path", "HSdbpath").toString());

    try{

        bool openFlag = sqliteDatabase.open();
        if (!openFlag) {
            qDebug() << "open database file err";
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::openmysql 捕获到异常: ============================ " << e.what() << std::endl;
    }

    if (sql_query) {
        delete sql_query;
    }
    sql_query = new QSqlQuery(sqliteDatabase);



}




void MySqlLite::gettb_files_onlyme(int type_jur, QString userid) {


    QList<FileInfo> file_all;
    QString execText = R"(
        SELECT
            f.Id,
            f.F_Guid,
            f.F_DeviceNo,
            f.F_UserNo,
            f.F_WorkStationIp,
            f.F_FileName,
            f.F_Type,
            f.F_LocalStorageDriveLetter,
            f.F_ServerStorageDriveLetter,
            f.F_FilePath,
            f.F_ShootTime,
            f.F_UploadTime,
            f.F_FileSize,
            f.F_FileTime,
            f.F_Thumbnail,
            f.F_IsImportant,
            f.F_FtpUploadStatus,
            f.F_IsEncrypt,
            f.F_IsTranscode,
            f.F_CompStatus,
            f.F_FileDataSyncStatus,
            f.F_Remark,
            f.F_FileSource,
            f.F_IsDelete,
            u.F_UserName,
            f.F_MarkId
        FROM
              t_file f
        LEFT JOIN
            t_userinfo   u ON f.F_UserNo = u.F_UserNo
        WHERE
            f.F_TYPE IN ('1', '2', '3','4') AND f.F_UserNo = :userid

            order by  f.id  desc

        LIMIT  8000
    )";

//    qDebug() << "gettb_files_onlyme execText:" << execText;

    sql_query->prepare(execText);
    sql_query->bindValue(":userid", userid);

    try{
        if (!sql_query->exec()) {
            qDebug() << "SQL error:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gettb_files_onlyme 捕获到异常: ============================ " << e.what() << std::endl;
    }


    while (sql_query->next()) {
        FileInfo fileinfo = fillFileInfo(*sql_query);
        file_all.append(fileinfo);
    }

    if (!file_all.isEmpty()) {
        qDebug() << "MySqlLite::gettb_files_onlyme gettb_files_success";
        emit gettb_files_success(file_all);
    } else {
        qDebug() << "No files found for user:" << userid;
    }
}


void MySqlLite::gettb_files_all(int type_jur, QString userid) {


    QList<FileInfo> file_all;
    QString execText = R"(
        SELECT
            f.Id,
            f.F_Guid,
            f.F_DeviceNo,
            f.F_UserNo,
            f.F_WorkStationIp,
            f.F_FileName,
            f.F_Type,
            f.F_LocalStorageDriveLetter,
            f.F_ServerStorageDriveLetter,
            f.F_FilePath,
            f.F_ShootTime,
            f.F_UploadTime,
            f.F_FileSize,
            f.F_FileTime,
            f.F_Thumbnail,
            f.F_IsImportant,
            f.F_FtpUploadStatus,
            f.F_IsEncrypt,
            f.F_IsTranscode,
            f.F_CompStatus,
            f.F_FileDataSyncStatus,
            f.F_Remark,
            f.F_FileSource,
            f.F_IsDelete,
            u.F_UserName,
            f.F_MarkId
        FROM
            t_file f
        LEFT JOIN
             t_userinfo  u ON f.F_UserNo = u.F_UserNo
        WHERE
            f.F_TYPE IN ('1', '2', '3','4')
            order by  f.id  desc

        LIMIT 4000

    )";

//    qDebug() << "gettb_files_all execText:" << execText;

    try{
        if (!sql_query->exec(execText)) {
            qDebug() << "SQL error:" << sql_query->lastError().text();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gettb_files_all 捕获到异常: ============================ " << e.what() << std::endl;
    }


    while (sql_query->next()) {
        FileInfo fileinfo = fillFileInfo(*sql_query);
        file_all.append(fileinfo);
    }

    if (!file_all.isEmpty()) {
        qDebug() << "MySqlLite::gettb_files_all gettb_files_success";
        emit gettb_files_success(file_all);
    } else {
        qDebug() << "No files found in the database.";
    }
}

FileInfo MySqlLite::fillFileInfo(QSqlQuery& query) {


    FileInfo fileinfo;
    fileinfo.F_Guid = query.value(1).toString();
    fileinfo.F_Type = getFileTypeDescription(query.value(6).toString());
    fileinfo.F_UserNo = query.value(3).toString();
    fileinfo.F_DeviceNo = query.value(2).toString();
    fileinfo.F_ShootTime = query.value(10).toString();
    fileinfo.F_UploadTime = query.value(11).toString();
    fileinfo.F_IsImportant = getIsImportantDescription(query.value(15).toString());
    fileinfo.F_Remark = query.value(21).toString();
    fileinfo.file_uuid = query.value(0).toString();
    fileinfo.file_name = query.value(5).toString();
    fileinfo.filepath = query.value(7).toString();
    fileinfo.F_WorkStationIp = query.value(4).toString();
    fileinfo.F_UserName = query.value(24).toString();
    fileinfo.F_MarkId   = getFMarkIdDescription(query.value(25).toString());

    fileinfo.F_LocalStorageDriveLetter   =  query.value(7).toString();
    fileinfo.F_IMP_VALUES   =  query.value(15).toString();
    fileinfo.F_MARKID_VALUES   =  query.value(25).toString();

    return fileinfo;
}

void MySqlLite::gett_department_all(QString userid) {


    QList<DepartmentInfo> department_all;

    QString execText = "SELECT Id, F_Name, F_ParentId, F_Code, F_Remark, F_IsDelete FROM t_department WHERE F_IsDelete = 0";

    try{

        if (!sql_query->exec(execText)) {
            qDebug() << "SQL error:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gett_department_all 捕获到异常: ============================ " << e.what() << std::endl;
    }


    while (sql_query->next()) {
        DepartmentInfo departmentInfo = fillDepartmentInfo(*sql_query);
        department_all.append(departmentInfo);
    }

    if (!department_all.isEmpty()) {
        qDebug() << "MySqlLite::gett_department_all gett_department_success";
        emit gett_department_success(department_all);
    } else {
        qDebug() << "No departments found.";
    }
}

DepartmentInfo MySqlLite::fillDepartmentInfo(QSqlQuery& query) {


    DepartmentInfo departmentInfo;
    departmentInfo.id = query.value(0).toString();
    departmentInfo.f_name = query.value(1).toString();
    departmentInfo.f_parentId = query.value(2).toString();
    departmentInfo.f_code = query.value(3).toString();
    departmentInfo.f_remark = query.value(4).toString();
    departmentInfo.f_isDelete = query.value(5).toString();
    return departmentInfo;
}


void MySqlLite::MSQSelectAllDeptManageTOUSER(QString userid, QString roleId, int statu) {


    QList<DepartmentInfo> department_all;

    QString execText = "SELECT Id, F_Name, F_ParentId, F_Code, F_Remark, F_IsDelete FROM t_department WHERE F_IsDelete = 0";

    try{

        if (!sql_query->exec(execText)) {
            qDebug() << "SQL error:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::MSQSelectAllDeptManageTOUSER 捕获到异常: ============================ " << e.what() << std::endl;
    }

    while (sql_query->next()) {
        DepartmentInfo departmentInfo = fillDepartmentInfo(*sql_query);
        department_all.append(departmentInfo);
    }


    if (!department_all.isEmpty()) {
        qDebug() << "MySqlLite::MSQSelectAllDeptManageTOUSER gettb_files_success statu:" << statu;
        emit department_successTOUSER(department_all,statu);
    } else {
        qDebug() << "No departments found.";
    }
}

void MySqlLite::MSQInsertt_userinfoToUSER(const QMap<QString, QVariant>& userInfoMap)
{

    QVariantMap copyTUserInfo;
//    QString MyHostIPV4Address = getHostIPV4Address().toString();

    // Validate userName
    QString userNo = userInfoMap["userNo"].toString();
    if (!isValidUserName(userNo)) {
        qDebug() << "Invalid userNo ERROR: " << userNo;
        return;
    }

     sql_query->prepare("INSERT INTO t_userinfo (F_UserNo, F_DeviceNo, F_UserName, F_DepartmentId, F_Status, F_RoleId, F_Password, F_CreateDate, F_UpdateDate) "
                   "VALUES (:userNo, :deviceNo, :userName, :departmentId, :status, :roleId, :password, :createDate, :updateDate)");


     sql_query->bindValue(":userNo", userInfoMap["userNo"].toString());
     sql_query->bindValue(":deviceNo", userInfoMap["deviceNo"].toString());
     sql_query->bindValue(":userName", userInfoMap["userName"].toString());
     sql_query->bindValue(":departmentId", userInfoMap["departmentId"].toInt());
     sql_query->bindValue(":status", userInfoMap["statusValue"].toString());
     sql_query->bindValue(":roleId", userInfoMap["roleValue"].toInt());
     sql_query->bindValue(":password", userInfoMap["password"].toString());
     sql_query->bindValue(":createDate", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
     sql_query->bindValue(":updateDate", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    try{

        if (!sql_query->exec()) {
            qDebug() << "MySqlLite::MSQInsertt_userinfoToUSER err" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }

//     copyTUserInfo["F_UserNo"] = userInfoMap["userNo"].toString();
//     copyTUserInfo["F_DeviceNo"] = userInfoMap["deviceNo"].toString();
//     copyTUserInfo["F_WorkStationIp"] = userInfoMap["F_WorkStationIp"].toString();
     copyTUserInfo["F_UserNo"] = userInfoMap["userNo"].toString();
     copyTUserInfo["F_DeviceNo"] = userInfoMap["deviceNo"].toString();
     copyTUserInfo["F_UserName"] = userInfoMap["userName"].toString();
     copyTUserInfo["F_DepartmentId"] = userInfoMap["departmentId"].toInt();
     copyTUserInfo["F_Status"] = userInfoMap["statusValue"].toString();
     copyTUserInfo["F_RoleId"] = userInfoMap["roleValue"].toInt();
     copyTUserInfo["F_Password"] = userInfoMap["password"].toString();
     copyTUserInfo["F_CreateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
     copyTUserInfo["F_UpdateDate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
     copyTUserInfo["F_WorkStationIp"] = userInfoMap["F_WorkStationIp"].toString();
     copyTUserInfo["F_Department"] = userInfoMap["department"].toString();

//     emit mysqlUserInfo_ClientEditTONET(copyTUserInfo);


     emit mysqlClientAdd(copyTUserInfo);
}

void MySqlLite::MSQupdatet_userInfoToUSER(const QMap<QString, QVariant> &userInfoMap)
{

    QVariantMap copyTUserInfo;
    QString userNo = userInfoMap["userNo"].toString();
    if (!isValidUserName(userNo)) {
        qDebug() << "Invalid userNo ERROR: " << userNo;
        return;
    }

    QString updateQueryString = "UPDATE t_userinfo "
                                 "SET F_UserNo = ?, "
                                 "F_DeviceNo = ?, "
                                 "F_UserName = ?, "
                                 "F_DepartmentId = ?, "
                                 "F_WorkStationIp = ?, "
                                 "F_WorkStationId = ?, "
                                 "F_Status = ?, "
                                 "F_RoleId = ?, "
                                 "F_Password = ? "
                                 "WHERE Id = ? AND F_IsDelete = 0;";


    if (!sql_query->prepare(updateQueryString)) {
        qDebug() << "MySqlLite::MSQupdatet_userInfoToUSER err" << sql_query->lastError().text();
        return;
    }


    sql_query->bindValue(0, userInfoMap["userNo"]);
    sql_query->bindValue(1, userInfoMap["deviceNo"]);
    sql_query->bindValue(2, userInfoMap["userName"]);

    sql_query->bindValue(3, userInfoMap["departmentId"]);

    sql_query->bindValue(4, userInfoMap["F_WorkStationIp"]);   // F_WorkStationIp
    sql_query->bindValue(5, 666);   // F_WorkStationId
    sql_query->bindValue(6, userInfoMap["statusValue"]);
    sql_query->bindValue(7, userInfoMap["roleValue"]);
    sql_query->bindValue(8, userInfoMap["password"]);

    sql_query->bindValue(9, userInfoMap["userId"]);


    try{
        if (!sql_query->exec()) {
            qDebug() << "MySqlLite::MSQupdatet_userInfoToUSER err2" << sql_query->lastError().text();
        } else {
            qDebug() << "MySqlLite::MSQupdatet_userInfoToUSER succ";
            copyTUserInfo["F_UserNo"] = userInfoMap["userNo"];
            copyTUserInfo["F_DeviceNo"] = userInfoMap["deviceNo"];
            copyTUserInfo["F_UserName"] = userInfoMap["userName"];
            copyTUserInfo["F_DepartmentId"] = userInfoMap["departmentId"];
            copyTUserInfo["F_WorkStationIp"] = userInfoMap["F_WorkStationIp"];
            copyTUserInfo["F_Status"] = userInfoMap["statusValue"];
            copyTUserInfo["F_RoleId"] = userInfoMap["roleValue"];
            copyTUserInfo["F_Department"] = userInfoMap["department"];
    //        copyTUserInfo["F_Password"] = userInfoMap["password"];
    //        copyTUserInfo["F_UserId"] = userInfoMap["userId"];
            emit mysqlUserInfo_ClientEditTONET(copyTUserInfo);
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }

}

/**
 * @brief MySqlLite::MSQRdelett_userinfo_byidtoUSER
 * @param getUserFormSystemIPV4
 * @param userInfoList
 * UPDATE t_userinfo
SET
    F_IsDelete = 1,
    F_UserNo = CASE Id
        WHEN 1 THEN 'userA_2023-10-05 12:34:56'
        WHEN 2 THEN 'userB_2023-10-05 12:34:56'
        WHEN 3 THEN 'userC_2023-10-05 12:34:56'
    END
WHERE Id IN (1, 2, 3);
 */

void MySqlLite::MSQRdelett_userinfo_byidtoUSER(QString getUserFormSystemIPV4, QList<QMap<QString, QString>> userInfoList)
{


    QJsonArray userList;
    QStringList userids;
    QStringList userNos; // 用于存储 F_UserNo
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"); // 获取当前时间

    for (QMap<QString, QString> userInfo : userInfoList) {
        userids.append(userInfo["id"]);
        userNos.append(userInfo["F_UserNo"]);
        QJsonObject user;
        user["F_UserNo"] = userInfo["F_UserNo"];
        userList.append(user);
    }
    QString updateQueryString = "UPDATE t_userinfo SET F_IsDelete = 1, F_UserNo = CASE Id ";

    for (int i = 0; i < userids.size(); ++i) {
        updateQueryString += QString("WHEN ? THEN ? "); // 使用 CASE 语句更新 F_UserNo
    }


    updateQueryString += "END WHERE Id IN (";

    for (int i = 0; i < userids.size(); ++i) {
        updateQueryString += "?";

        if (i < userids.size() - 1) {
            updateQueryString += ", ";
        }
    }
    updateQueryString += ");";

//    qDebug() << updateQueryString;

    if (!sql_query->prepare(updateQueryString)) {
        qDebug() << "MySqlLite::MSQRdelett_userinfo_byidtoUSER err" << sql_query->lastError().text();
        return;
    }


    // 绑定 Id 和 F_UserNo
    for (int i = 0; i < userids.size(); ++i) {
        sql_query->bindValue(i * 2, userids[i]); // 绑定 Id
        sql_query->bindValue(i * 2 + 1, userNos[i] + "_" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")); // 绑定 F_UserNo
    }

    // 绑定 Id 列表
    for (int i = 0; i < userids.size(); ++i) {
        sql_query->bindValue(userids.size() * 2 + i, userids[i]); // 绑定 Id
    }

    try{
        if (!sql_query->exec()) {
            qDebug() << "MySqlLite::MSQRdelett_userinfo_byidtoUSER err" << sql_query->lastError().text();
        } else {
            qDebug() << "MySqlLite::MSQRdelett_userinfo_byidtoUSER succ";
            #ifdef USE_FACE_FINGER
            //delete finger feature data.
            deleteFingerFeatureList(userNos);

            for (const QString &userNO : userNos) {
                // 准备更新语句
                sql_query->prepare("UPDATE t_face SET F_UserNo = F_UserNo || ' ' || ? WHERE F_UserNo = ?");

                // 绑定参数
                sql_query->bindValue(0, currentTime); // 追加当前时间
                sql_query->bindValue(1, userNO); // 目标 userNO

                // 执行更新操作
                if (!sql_query->exec()) {
                    qDebug() << "Update execution error for userNo:" << userNO << " - " << sql_query->lastError().text();
                } else {
                    if (sql_query->numRowsAffected() > 0) {
                        qDebug() << "Updated F_UserNo for userNo:" << userNO;
                    } else {
                        qDebug() << "No record found for userNo:" << userNO << ", skipping update.";
                    }
                }
            }
            #endif
            emit mysqlUserInfo_ClientDeleteTONET(getUserFormSystemIPV4, userList);
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }



}

void MySqlLite::MSQSelectT_logAllTOLog(QString userid, QString roleId)
{
    if(roleId == "1"){
       gett_logAll_all(userid,roleId);
    }else if(roleId == "2"){
       getPermissiont_log(userid,roleId);
    }else if(roleId == "3"){
       gett_log_onlyme(userid,roleId);
    }
}

void MySqlLite::insertt_logformainF(QString userNo,int type) {

    sql_query->prepare("INSERT INTO t_log (F_Description, F_LoginName, F_LogType, F_ModuleName, F_RealName) "
                       "VALUES (:description, :loginName, :logType, :moduleName, :realName)");


    sql_query->bindValue(":description", "登录成功");
    sql_query->bindValue(":loginName", userNo);
    sql_query->bindValue(":logType", logTypeToString(type));
    sql_query->bindValue(":moduleName", "登录成功");
    sql_query->bindValue(":realName", "登录成功");


    try{
        if (!sql_query->exec()) {
            qDebug() << "Failed to insert log:" << sql_query->lastError().text();
        } else {
            qDebug() << "Log insert successful.";
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }



}
const char* MySqlLite::logTypeToString(int type) {

    switch (type) {
        case 0: return "数据查询";        // DataQuery
        case 1: return "系统设置";        // SystemSettings
        case 2: return "解锁";            // Unlock
        case 3: return "更新";            // Update
        case 4: return "关机";            // Exit
        case 5: return "个人中心";        // PersonalCenter
        case 6: return "执法记录仪接入";
        case 7: return "执法记录仪移出";
        case 8: return "开机";
        default: return "未知类型";       // 处理无效类型
    }
}

void MySqlLite::MSQselectWeekUserInfoTOMainF(QString ipv4)
{


    QJsonArray userInfoList;


//    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
//    QString oneWeekAgo = QDateTime::currentDateTime().addDays(-7).toString("yyyy-MM-dd HH:mm:ss");



//    sql_query->prepare("SELECT * FROM t_userinfo WHERE F_CreateDate >= :oneWeekAgo");


    // 修改 SQL 查询以使用 JOIN 语句
    sql_query->prepare(R"(
        SELECT u.*, d.F_Name AS DepartmentName
        FROM t_userinfo u
        JOIN t_department d ON u.F_DepartmentId = d.Id
        WHERE u.F_Status = '1' AND u.F_IsDelete = 0
    )");



//    sql_query->bindValue(":oneWeekAgo", oneWeekAgo);

    try{
        if (!sql_query->exec()) {
            qDebug() << "MSQselectWeekUserInfoTOMainF error:" << sql_query->lastError().text();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::MSQselectWeekUserInfoTOMainF 捕获到异常: ============================ " << e.what() << std::endl;
    }

//    while (sql_query->next()) {
//            QJsonObject user;
//            user["F_UserNo"] = sql_query->value("F_UserNo").toString();
//            user["F_DeviceNo"] = sql_query->value("F_DeviceNo").toString();
//            user["F_UserName"] = sql_query->value("F_UserName").toString();
//            user["F_Department"] = sql_query->value("F_DepartmentId").toString();
//            user["F_WorkStationIp"] = ipv4;
//            user["F_IsDelete"] = 0;
//            user["F_Gender"] = sql_query->value("F_Gender").toString();
//            user["F_Password"] = sql_query->value("F_Password").toString();
//            user["F_Phone"] = sql_query->value("F_Phone").toString();
//            user["F_Remark"] = sql_query->value("F_Remark").toString();
//            user["F_Status"] = sql_query->value("F_Status").toString();
//            user["F_CreateDate"] = sql_query->value("F_CreateDate").toString();
//            user["F_UpdateDate"] = sql_query->value("F_UpdateDate").toString();

//            userInfoList.append(user);
//    }

    while (sql_query->next()) {
        QJsonObject user;
        user["F_UserNo"] = sql_query->value("F_UserNo").toString();
        user["F_DeviceNo"] = sql_query->value("F_DeviceNo").toString();
        user["F_UserName"] = sql_query->value("F_UserName").toString();
        user["F_Department"] = sql_query->value("DepartmentName").toString(); // 获取部门名称
        user["F_WorkStationIp"] = ipv4;
        user["F_IsDelete"] = 0;
        user["F_Gender"] = sql_query->value("F_Gender").toString();
        user["F_Password"] = sql_query->value("F_Password").toString();
        user["F_Phone"] = sql_query->value("F_Phone").toString();
        user["F_Remark"] = sql_query->value("F_Remark").toString();
        user["F_Status"] = sql_query->value("F_Status").toString();
        user["F_CreateDate"] = sql_query->value("F_CreateDate").toString();
        user["F_UpdateDate"] = sql_query->value("F_UpdateDate").toString();

        userInfoList.append(user);
    }


    emit MSQselectWeekUserInfoTONet(ipv4,userInfoList);
}

void MySqlLite::MSQselectWeekFileInfoTOMainF(QString ipv4)
{
    QJsonArray fileList;

//    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString oneWeekAgo = QDateTime::currentDateTime().addDays(-7).toString();



    sql_query->prepare("SELECT * FROM t_file WHERE F_UploadTime >= :oneWeekAgo");


    sql_query->bindValue(":oneWeekAgo", oneWeekAgo);

    try{

        if (!sql_query->exec()) {
            qDebug() << "Query error:" << sql_query->lastError().text();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::MSQselectWeekFileInfoTOMainF 捕获到异常: ============================ " << e.what() << std::endl;
    }


    while (sql_query->next()) {
        QJsonObject file1;
//        file1["Id"] = 0;
        file1["F_Guid"] = sql_query->value("F_Guid").toString();
        file1["F_DeviceNo"] = sql_query->value("F_DeviceNo").toString();
        file1["F_UserNo"] = sql_query->value("F_UserNo").toString();
        file1["F_WorkStationIp"] = sql_query->value("F_WorkStationIp").toString();
        file1["F_FileName"] = sql_query->value("F_FileName").toString();
        file1["F_Type"] = sql_query->value("F_Type").toInt();
        file1["F_LocalStorageDriveLetter"] = sql_query->value("F_LocalStorageDriveLetter").toString();
        file1["F_ServerStorageDriveLetter"] = sql_query->value("F_ServerStorageDriveLetter").toString();
        file1["F_FilePath"] = sql_query->value("F_FilePath").toString();
        file1["F_ShootTime"] = sql_query->value("F_ShootTime").toString();
        file1["F_UploadTime"] = sql_query->value("F_UploadTime").toString();
        file1["F_FileSize"] = sql_query->value("F_FileSize").toInt();
        file1["F_FileTime"] = sql_query->value("F_FileTime").toInt();
        file1["F_Thumbnail"] = sql_query->value("F_Thumbnail").toString();
        file1["F_IsImportant"] = sql_query->value("F_IsImportant").toInt();
        file1["F_FtpUploadStatus"] = sql_query->value("F_FtpUploadStatus").toInt();
        file1["F_IsEncrypt"] = sql_query->value("F_IsEncrypt").toInt();
        file1["F_IsTranscode"] = sql_query->value("F_IsTranscode").toInt();
        file1["F_CompStatus"] = sql_query->value("F_CompStatus").toInt();
        file1["F_FileDataSyncStatus"] = sql_query->value("F_FileDataSyncStatus").toInt();
        file1["F_Remark"] = sql_query->value("F_Remark").toString();
        file1["F_FileSource"] = sql_query->value("F_FileSource").toInt();
        file1["F_IsDelete"] = sql_query->value("F_IsDelete").toInt();
        file1["F_MarkId"] = sql_query->value("F_MarkId").toInt();


        fileList.append(file1);
    }
    emit MSQselectWeekFileInfoTONet(ipv4,fileList);

}

void MySqlLite::saveFaceByUserNoForFaceForm(QString userNO, QByteArray face,int nFeatureSize)
{


    sql_query->prepare("UPDATE t_face SET F_Feature = ? WHERE F_UserNo = ?");
    sql_query->bindValue(0, face);
    sql_query->bindValue(1, userNO);

    try{

        if (!sql_query->exec()) {
            qDebug() << "Update execution error:" << sql_query->lastError().text();
        } else {
            int updatedRows = sql_query->numRowsAffected();

            if (updatedRows == 0) {
                // 如果没有行被更新，执行插入
                int userInfoId = 1;
                int code = 1;

                qDebug() << "No rows updated, attempting to insert.";

                sql_query->prepare("INSERT INTO t_face (F_UserInfoId, F_Code, F_Feature, F_UserNo) VALUES (?, ?, ?, ?)");
                sql_query->bindValue(0, userInfoId);
                sql_query->bindValue(1, code);
                sql_query->bindValue(2, face);
                sql_query->bindValue(3, userNO);

                if (!sql_query->exec()) {
                    qDebug() << "Insert execution error:" << sql_query->lastError().text();
                } else {
                    qDebug() << "Face data saved successfully for userNo:" << userNO;
                }
            } else {
                qDebug() << "Face data updated successfully for userNo:" << userNO;
            }
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }



}

void MySqlLite::fetchAllFaceFeaturesTOFaceFrom() {


    std::vector<FaceFeature> faceFeatures;
    faceFeatures.clear();
//    std::vector<QByteArray> faceArray;

    sql_query->prepare("SELECT F_UserNo, F_Feature FROM t_face");

    try{

        if (!sql_query->exec()) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::fetchAllFaceFeaturesTOFaceFrom 捕获到异常: ============================ " << e.what() << std::endl;
    }

    while (sql_query->next()) {
        FaceFeature faceFeature;
//        QByteArray featureData = sql_query->value(1).toByteArray();

        faceFeature.userNo = sql_query->value(0).toString(); // 获取用户编号
        faceFeature.featureData = sql_query->value(1).toByteArray(); // 获取特征数据

        if (!faceFeature.featureData.isEmpty()) { // 确保特征数据不为空
            faceFeatures.push_back(faceFeature); // 将结构体实例添加到向量中
        } else {
            qDebug() << "Received empty feature data for UserNo:" << faceFeature.userNo; // 输出警告信息
        }
    }

    emit allFaceFeaturesTOFaceR(faceFeatures);

}

void MySqlLite::delete_by_saveday(int day)
{

    if(day == -1  || day == 0){

        return;
    }


        // 计算截止日期
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QDateTime cutoffDateTime = currentDateTime.addDays(-day);
        QString cutoffDateStr = cutoffDateTime.toString("yyyy-MM-ddTHH:mm:ss");

        // 查询要删除的不重要记录
        QString selectSql = "SELECT Id, F_LocalStorageDriveLetter FROM t_file WHERE F_UploadTime < :cutoffDate AND F_IsImportant = 0 AND F_MarkId = 0";


        sql_query->prepare(selectSql);
        sql_query->bindValue(":cutoffDate", cutoffDateStr);



        qDebug() << "Last sql :" << sql_query->lastQuery();

        try{
            if (!sql_query->exec()) {
                qDebug() << "Error selecting records:" << sql_query->lastError().text();
                qDebug() << "Last exec sql :" << sql_query->lastQuery();

                insertt_logformainF("888888",8);
                return;
            }

        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================ MySqlLite::delete_by_saveday 捕获到异常: ============================ " << e.what() << std::endl;
        }


        QStringList idsToDelete;
        while (sql_query->next()) {
            int id = sql_query->value("Id").toInt();
            QString filePath = sql_query->value("F_LocalStorageDriveLetter").toString();

            // 尝试删除本地文件
            if (!filePath.isEmpty() && QFile::exists(filePath)) {

                qDebug() << "File   found  :" << filePath;

                if (QFile::remove(filePath)) {
                    qDebug() << "File deleted successfully:" << filePath;
                } else {
                    qDebug() << "Error deleting file:" << filePath;
                }

                qDebug() << "File opt end  :" << filePath;
            }else{

                //qDebug() << "File path not found  :" << filePath;
            }

            idsToDelete.append(QString::number(id));
        }

        // 删除数据库记录
        if (!idsToDelete.isEmpty()) {
            QString deleteSql = "DELETE FROM t_file WHERE Id IN (";
            QStringList placeholders;

            // 为每个 ID 添加占位符
            for (int i = 0; i < idsToDelete.size(); ++i) {
                placeholders.append("?");
            }
            deleteSql.append(placeholders.join(","));
            deleteSql.append(")");

            sql_query->prepare(deleteSql);

            // 绑定参数
            for (int i = 0; i < idsToDelete.size(); ++i) {
                sql_query->bindValue(i, idsToDelete[i]);
            }

            try{

                if (sql_query->exec()) {
                    qDebug() << "Records deleted successfully.";
                } else {
                    qDebug() << "Error deleting records:" << sql_query->lastError().text();
                }


            }catch(const std::exception& e){

                // 处理异常
                 std::cerr << " ============================ MySqlLite::delete_by_saveday  DELETE FROM t_file WHERE 捕获到异常: ============================ " << e.what() << std::endl;
            }




        } else {
            qDebug() << "No records to delete.";
        }

        insertt_logformainF("888888",8);
}

void MySqlLite::slotImportant_tfilebyid(QString workstationIp, QList<QMap<QString, QString> > tfileIds)
{

    QStringList fileIds;

    for (const QMap<QString, QString>& map : tfileIds) {
        QString fileId = map.value("tfile_id");

        if (!fileId.isEmpty()) {
            fileIds.append(fileId);
        }
    }

    if (fileIds.isEmpty()) {
        qDebug() << "No IDs to update.";
        return;
    }

    // 创建 SQL 更新语句
//    QString sqlUpdate = "UPDATE t_file SET F_IsImportant = CASE "
//                        "WHEN F_IsImportant = 0 THEN 1 "
//                        "WHEN F_IsImportant = 1 THEN 0 "
//                        "ELSE F_IsImportant END "
//                        "WHERE Id IN (";

    QString sqlUpdate = "UPDATE t_file SET F_MarkId = CASE "
                        "WHEN F_MarkId = 0 THEN 1 "
                        "WHEN F_MarkId = 1 THEN 0 "
                        "END "
                        "WHERE Id IN (";

    // 添加占位符
    QStringList placeholders;
    for (int i = 0; i < fileIds.size(); ++i) {
        placeholders.append("?");
    }
    sqlUpdate.append(placeholders.join(","));
    sqlUpdate.append(")");

    // 准备 SQL 查询
    sql_query->prepare(sqlUpdate);

    // 绑定参数
    for (int i = 0; i < fileIds.size(); ++i) {
        sql_query->bindValue(i, fileIds[i]);
    }

    try{
        // 执行 SQL 查询
        if (sql_query->exec()) {
            qDebug() << "Records updated successfully.";
        } else {
            qDebug() << "Error updating records:" << sql_query->lastError().text();
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }


}

void MySqlLite::getUserListCAR_DEPOT()
{

    QStringList userList;
    QString sql = "SELECT F_UserNo FROM t_userinfo WHERE F_IsDelete = 0 AND F_Status = '1'";

    try{

        if (!sql_query->exec(sql)) {
            qDebug() << "Error executing query:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::getUserListCAR_DEPOT  捕获到异常: ============================ " << e.what() << std::endl;
    }

    // 遍历结果
    while (sql_query->next()) {
        QString userNo = sql_query->value(0).toString(); // 获取 F_UserNo 的值
        userList.append(userNo);
        emit UserListCAR_DEPOT(userList);
//        qDebug() << "F_UserNo:" << userNo;
    }
}

void MySqlLite::slotRemark_tfilebyid(const QMap<QString, QString> &file)
{
    // 从 QMap 中获取数据
    QString tfileId = file["tfile_id"];
    QString remark = file["F_Remark"];



    sql_query->prepare("UPDATE t_file SET F_Remark = :remark WHERE Id = :id");

    // 绑定参数
    sql_query->bindValue(":remark", remark);
    sql_query->bindValue(":id", tfileId.toInt()); // 将 tfileId 转换为整型

    try{
        // 执行查询
        if (!sql_query->exec()) {
            qDebug() << "Failed to update remark:" << sql_query->lastError().text();
        } else {
            qDebug() << "Remark updated successfully for Id:" << tfileId;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }

}


void MySqlLite::slotRename_tfilebyid(const QMap<QString, QString> &file)
{
    // 从 QMap 中获取数据
    QString tfileId = file["tfile_id"];

    QString F_LocalStorageDriveLetter = file["F_LocalStorageDriveLetter"];
    QString F_ServerStorageDriveLetter = file["F_LocalStorageDriveLetter"];

    QStringList l_remarks = F_LocalStorageDriveLetter.split('/');
    QString F_Remark = l_remarks.last();

    sql_query->prepare("UPDATE t_file SET F_LocalStorageDriveLetter = :F_LocalStorageDriveLetter  , F_ServerStorageDriveLetter = :F_ServerStorageDriveLetter ,F_Remark = :F_Remark  WHERE Id = :id");

    // 绑定参数
    sql_query->bindValue(":F_LocalStorageDriveLetter", F_LocalStorageDriveLetter);
    sql_query->bindValue(":F_ServerStorageDriveLetter", F_ServerStorageDriveLetter);

     sql_query->bindValue(":F_Remark", F_Remark);


    sql_query->bindValue(":id", tfileId.toInt()); // 将 tfileId 转换为整型

    try{
        // 执行查询
        if (!sql_query->exec()) {
            qDebug() << "Failed to update F_LocalStorageDriveLetter  --   F_ServerStorageDriveLetter :" << sql_query->lastError().text();
        } else {
            qDebug() << "slotRename_tfilebyid  updated successfully for Id:" << tfileId  << " --  "<< F_LocalStorageDriveLetter;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }

}



void MySqlLite::setPARAMRelevance(const QString &path,const QString &polNo)
{

    //qDebug()<< __FUNCTION__ << " =========== MySqlLite ============ 所在线程："<< QThread::currentThreadId();

#ifdef RELEVANCE
    QSettings setting(path + "/Param.ini", QSettings::IniFormat);
    setting.beginGroup("ZFY");
//    QString devNo = setting.value("DevNo").toString();
//    QString polNo = setting.value("PolNo").toString();

    sql_query->prepare("SELECT F_Status FROM t_userinfo WHERE F_UserNo = :userNo");
    sql_query->bindValue(":userNo", polNo);

    // Execute the query
    if (!sql_query->exec()) {
        qDebug() << "Query execution error:" << sql_query->lastError().text();
        return ; // Return an empty string on error
    }

    // Fetch the result
    if (sql_query->next()) {
        QString status = sql_query->value(0).toString(); // Get the F_Status value
        // 0 不启用
        if("0" == status){
           setting.setValue("uncorrelated", "true");
        }else{
            setting.setValue("uncorrelated", "false");
        }

    } else {
        setting.setValue("uncorrelated", "true");
        qDebug() << "No user found with F_UserNo:" << polNo;
    }
    setting.endGroup();

#endif
}

void MySqlLite::gett_logAll_all(QString userid,QString roleId) {

    sql_query->prepare("SELECT * FROM t_log WHERE F_IsDelete = :isDelete limit  30 ");
    sql_query->bindValue(":isDelete", 0);

    try{

        if (!sql_query->exec()) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gett_logAll_all  捕获到异常: ============================ " << e.what() << std::endl;
    }

    QList<LogInfo> log_all;


    while (sql_query->next()) {
        LogInfo log;

        log.id = sql_query->value(0).toString();
        log.f_description = sql_query->value(1).toString();
        log.f_loginName = sql_query->value(2).toString();
        log.f_logType = sql_query->value(3).toString();
        log.f_moduleName = sql_query->value(4).toString();
        log.f_realName = sql_query->value(5).toString();
        log.f_isDelete = sql_query->value(6).toString();
        log.createdAt = sql_query->value(7).toString();

        log_all.append(log);
    }


    if (!log_all.isEmpty()) {
//        qDebug() << "MySqlLite::gett_logAll_all gettb_files_success: Retrived" << log_all.size() << "logs.";
        emit t_log_successTOLog(log_all);
    } else {
        qDebug() << "No logs found.";
    }
}

void MySqlLite::getPermissiont_log(QString userid, QString roleId)
{

}

void MySqlLite::gett_log_onlyme(QString userid, QString roleId)
{

}


void MySqlLite::getPermissiont_department(QString userid)
{

}

void MySqlLite::gett_department_onlyme(QString userid)
{

}


void MySqlLite::gett_userinfo_all(QString userid,QString userNumber,QString devNumber) {

    QList<NewUserInfo> userinfo_all;

    QString queryString = "SELECT user.Id, user.F_UserNo, user.F_DeviceNo, user.F_UserName, "
                          "user.F_DepartmentId, dep.F_Name, user.F_WorkStationIp, "
                          "user.F_WorkStationId, user.F_IsDelete, user.F_Gender, "
                          "user.F_Password, user.F_Phone, user.F_Remark, "
                          "user.F_Status, user.F_RoleId, user.F_CreateDate, "
                          "user.F_UpdateDate, user.CreatedAt, user.UpdatedAt "
                          "FROM t_userinfo user "
                          "INNER JOIN t_department dep ON user.F_DepartmentId = dep.Id "
                          "WHERE user.F_IsDelete = 0 AND dep.F_IsDelete = 0  ";

    if(!userNumber.isEmpty()){
        queryString += " and  user.F_UserNo = '" +  userNumber + "' ";

    }



    if(!devNumber.isEmpty()){
        queryString += " and  user.F_DeviceNo like '" +  devNumber + "%' ";

    }



    try{
        if (!sql_query->exec(queryString)) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gett_userinfo_all  捕获到异常: ============================ " << e.what() << std::endl;
    }


    while (sql_query->next()) {
        NewUserInfo userInfoInfo;

        userInfoInfo.id = sql_query->value(0).toString();
        userInfoInfo.F_UserNo = sql_query->value(1).toString();
        userInfoInfo.F_DeviceNo = sql_query->value(2).toString();
        userInfoInfo.F_UserName = sql_query->value(3).toString();
        userInfoInfo.F_DepartmentId = sql_query->value(4).toString();
        userInfoInfo.f_name = sql_query->value(5).toString();
        userInfoInfo.F_WorkStationIp = sql_query->value(6).toString();
        userInfoInfo.F_WorkStationId = sql_query->value(7).toString();
        userInfoInfo.F_IsDelete = sql_query->value(8).toString();
        userInfoInfo.F_Gender = sql_query->value(9).toString();
        userInfoInfo.F_Password = sql_query->value(10).toString();
        userInfoInfo.F_Phone = sql_query->value(11).toString();
        userInfoInfo.F_Remark = sql_query->value(12).toString();
#ifdef RELEVANCE
        userInfoInfo.F_Status = (sql_query->value(13).toString() == "1") ? "关联" : "未关联";

#else
        userInfoInfo.F_Status = (sql_query->value(13).toString() == "1") ? "启用" : "禁用";

#endif


        const QString roleId = sql_query->value(14).toString();
        static const QMap<QString, QString> roleMap = {
            {"1", "超级管理员"},
            {"2", "普通管理员"},
            {"3", "普通用户"}
        };
        userInfoInfo.F_RoleId = roleMap.value(roleId, "未知角色");

        userInfoInfo.create_time = sql_query->value(15).toString();

        userinfo_all.append(userInfoInfo);
    }

    if (!userinfo_all.isEmpty()) {
        qDebug() << "MySqlLite::gett_userinfo_all gettb_files_success, retrieved:" << userinfo_all.size() << "users.";
        emit gett_userinfo_success(userinfo_all);
    } else {
        qDebug() << "No user information found.";
        emit gett_userinfo_success(userinfo_all);
    }
}



void MySqlLite::gett_gather_userinfo_all() {


    QList<NewUserInfo> userinfo_all;

    QString queryString = "SELECT user.Id, user.F_UserNo, user.F_DeviceNo, user.F_UserName, "
                          "user.F_DepartmentId, dep.F_Name, user.F_WorkStationIp, "
                          "user.F_WorkStationId, user.F_IsDelete, user.F_Gender, "
                          "user.F_Password, user.F_Phone, user.F_Remark, "
                          "user.F_Status, user.F_RoleId, user.F_CreateDate, "
                          "user.F_UpdateDate, user.CreatedAt, user.UpdatedAt "
                          "FROM t_userinfo user "
                          "INNER JOIN t_department dep ON user.F_DepartmentId = dep.Id "
                          "WHERE user.F_IsDelete = 0 AND dep.F_IsDelete = 0 AND user.Id > 1 ; ";

    try{
        if (!sql_query->exec(queryString)) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return  ;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gett_userinfo_all  捕获到异常: ============================ " << e.what() << std::endl;
    }


    while (sql_query->next()) {

        QCoreApplication::processEvents();

        NewUserInfo userInfoInfo;

        userInfoInfo.id = sql_query->value(0).toString();
        userInfoInfo.F_UserNo = sql_query->value(1).toString();
        userInfoInfo.F_DeviceNo = sql_query->value(2).toString();
        userInfoInfo.F_UserName = sql_query->value(3).toString();
        userInfoInfo.F_DepartmentId = sql_query->value(4).toString();
        userInfoInfo.f_name = sql_query->value(5).toString();
        userInfoInfo.F_WorkStationIp = sql_query->value(6).toString();
        userInfoInfo.F_WorkStationId = sql_query->value(7).toString();
        userInfoInfo.F_IsDelete = sql_query->value(8).toString();
        userInfoInfo.F_Gender = sql_query->value(9).toString();
        userInfoInfo.F_Password = sql_query->value(10).toString();
        userInfoInfo.F_Phone = sql_query->value(11).toString();
        userInfoInfo.F_Remark = sql_query->value(12).toString();
#ifdef RELEVANCE
        userInfoInfo.F_Status = (sql_query->value(13).toString() == "1") ? "关联" : "未关联";

#else
        userInfoInfo.F_Status = (sql_query->value(13).toString() == "1") ? "启用" : "禁用";

#endif


        const QString roleId = sql_query->value(14).toString();
        static const QMap<QString, QString> roleMap = {
            {"1", "超级管理员"},
            {"2", "普通管理员"},
            {"3", "普通用户"}
        };
        userInfoInfo.F_RoleId = roleMap.value(roleId, "未知角色");

        userInfoInfo.create_time = sql_query->value(15).toString();

        userinfo_all.append(userInfoInfo);
    }


    if (!userinfo_all.isEmpty()) {
        qDebug() << "MySqlLite::gett_userinfo_all gettb_files_success, retrieved:" << userinfo_all.size() << "users.";

        // emit gett_userinfo_success(userinfo_all);
        global_userinfo_all = userinfo_all;


    } else {
        qDebug() << "No user information found.";
    }



}


void MySqlLite::getPermissiont_userinfo(QString userid,QString userNumber,QString devNumber)
{

}

void MySqlLite::gett_userinfo_onlyme(QString userid,QString userNumber,QString devNumber)
{

}
QString MySqlLite::getFileTypeDescription(QString fileType) {

    int type = fileType.toInt();
    switch (type) {
        case 1:
            return "视频";
        case 2:
            return "音频";
        case 3:
            return "图片";
        case 4:
            return "文本日志";
        case 5:
            return "其他";
        default:
            return "未知类型";
    }
}
QString MySqlLite::getIsImportantDescription(QString import) {

    int imp = import.toInt();
    switch (imp) {
        case 0:
            return "IMP不重要";
        case 1:
            return "重要";
        case 2:
            return "IMP重要";
        default:
            return "未知类型";
    }
}
QString MySqlLite::getFMarkIdDescription(QString import) {

    int imp = import.toInt();
    switch (imp) {
        case 0:
            return "标注不重要";
        case 1:
            return "标注重要";
        default:
            return "未知类型";
    }
}
void MySqlLite::get_tb_files_type(int type_jur,QString roleId,QString usrNO)
{
    // 根据权限查数据//角色 1,超级管理员（看所有） 2，普通管理员（看部门以下） 3，普通用户（只能看自己）
    qDebug()<<"MySqlLite::get_tb_files_type type_jur"<<type_jur<<" usrId:"<<usrNO;
    if(roleId == "1"){
       gettb_files_all(type_jur,usrNO);
    }else if(roleId == "2"){
       getPermissionData(usrNO);
    }else if(roleId == "3"){
       gettb_files_onlyme(type_jur,usrNO);
    }


}




void MySqlLite::doinsertfiles(const QList<QVariantMap>& fileList, bool sourceFilePathIsErr, const QString &sourceDirFather, int windowNum) {


    QList<QVariantMap> successfulInsertions;
    successfulInsertions.clear();
    QSet<QString> existingGuids;


    sql_query->prepare("INSERT INTO t_file (F_Guid, F_DeviceNo, F_UserNo, F_WorkStationIp, F_FileName, F_Type, "
                       "F_LocalStorageDriveLetter, F_ServerStorageDriveLetter, F_FilePath, F_ShootTime, "
                       "F_UploadTime, F_FileSize, F_Thumbnail, F_IsImportant, F_FtpUploadStatus, "
                       "F_IsEncrypt, F_IsTranscode, F_CompStatus, F_FileDataSyncStatus, F_Remark, "
                       "F_FileSource, F_IsDelete ,F_FileTime,F_IsImportant,F_MarkId) "
                       "VALUES (:guid, :deviceNo, :userNo, :workStationIp, :fileName, :type, "
                       ":localStorageDriveLetter, :serverStorageDriveLetter, :filePath, :shootTime, "
                       ":uploadTime, :fileSize, :thumbnail, :isImportant, :ftpUploadStatus, "
                       ":isEncrypt, :isTranscode, :compStatus, :fileDataSyncStatus, :remark, "
                       ":fileSource, :isDelete, :fileTime , :isImportant , :markId)");



    for (const QVariantMap& copyfilemap : fileList) {

        QCoreApplication::processEvents();

        QString guid = copyfilemap["F_FileName"].toString();

        // 检查 GUID 是否已存在
        if (existingGuids.contains(guid)) {
//            qDebug() << "Duplicate entry found for F_Guid:" << guid;
            continue; // 跳过重复项
        }

        // 将 GUID 添加到集合中
        existingGuids.insert(guid);

        sql_query->bindValue(":guid", copyfilemap["F_Guid"].toString());
        sql_query->bindValue(":deviceNo", copyfilemap["F_DeviceNo"].toString());
        sql_query->bindValue(":userNo", copyfilemap["F_UserNo"].toString());
        sql_query->bindValue(":workStationIp", copyfilemap["F_WorkStationIp"].toString());
        sql_query->bindValue(":fileName", copyfilemap["F_FileName"].toString());
        sql_query->bindValue(":type", copyfilemap["F_Type"].toInt());
        sql_query->bindValue(":localStorageDriveLetter", copyfilemap["F_LocalStorageDriveLetter"].toString());
        sql_query->bindValue(":serverStorageDriveLetter", copyfilemap["F_ServerStorageDriveLetter"].toString());
        sql_query->bindValue(":filePath", copyfilemap["F_FilePath"].toString());
        sql_query->bindValue(":shootTime", copyfilemap["F_ShootTime"].toString());
        sql_query->bindValue(":uploadTime", copyfilemap["F_UploadTime"].toString());
        sql_query->bindValue(":fileSize", copyfilemap["F_FileSize"].toLongLong());
        sql_query->bindValue(":thumbnail", copyfilemap["F_Thumbnail"].toString());
        sql_query->bindValue(":isImportant", copyfilemap["F_IsImportant"].toInt());
        sql_query->bindValue(":ftpUploadStatus", copyfilemap["F_FtpUploadStatus"].toInt());
        sql_query->bindValue(":isEncrypt", copyfilemap["F_IsEncrypt"].toInt());
        sql_query->bindValue(":isTranscode", copyfilemap["F_IsTranscode"].toInt());
        sql_query->bindValue(":compStatus", copyfilemap["F_CompStatus"].toInt());
        sql_query->bindValue(":fileDataSyncStatus", copyfilemap["F_FileDataSyncStatus"].toInt());
        sql_query->bindValue(":remark", copyfilemap["F_Remark"].toString());
        sql_query->bindValue(":fileSource", copyfilemap["F_FileSource"].toInt());
        sql_query->bindValue(":isDelete", copyfilemap["F_IsDelete"].toInt());
        sql_query->bindValue(":fileTime", copyfilemap["F_FileTime"].toInt());
        // 检查 F_FileName 是否包含 "IMP"（不区分大小写）
        QString fileName = copyfilemap["F_FileName"].toString();
        if (fileName.contains("IMP", Qt::CaseInsensitive)) {
            sql_query->bindValue(":isImportant", 2); // 设置为 1
        } else {
            sql_query->bindValue(":isImportant", 0); // 设置为 0
        }
        sql_query->bindValue(":markId", 0);


        // 2024.12.13
        try{

            if (!sql_query->exec()) {
                qDebug() << "Failed to execute query doinsertfiles:" << sql_query->lastError().text();
                qDebug() << "Failed SQL Statement:" << sql_query->lastQuery();
                // 获取执行的 SQL 查询语句
                qDebug() << "Executed SQL:" << sql_query->executedQuery();
                qDebug() << "File Name:" << copyfilemap["F_FileName"].toString();
                continue;
            }

        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
        }

        if (isDeleteZFYData) {
            qDebug() << "QFile::remove F_FilePath:" << copyfilemap["F_FilePath"].toString();
            QFile::remove(copyfilemap["F_FilePath"].toString());
        }



        // 全部拷贝完成start
        successfulInsertions.append(copyfilemap);
        // 全部拷贝完成end
    }

//    qDebug() << "MySqlLite::doinsertfiles start insert t_file END:" << sourceFilePathIsErr;
//    emit mysqlWorkFileClientAdd(successfulInsertions);全部拷贝完成
    if(isUploadEnabled){

        emit mysqlWorkFileClientAdd(successfulInsertions);

    }



//    if (!sourceFilePathIsErr) {
//        emit sourceFileErrUnplugUsbDevice(sourceDirFather, windowNum);
//    } else {
//        emit waitUnplugUsbDevice(sourceDirFather, windowNum);
//    }
}




void MySqlLite::doQueryFilesById(const QStringList& ids) {

    QList<QVariantMap> queriedFiles;

    // 检查 ids 是否为空
    if (ids.isEmpty()) {
        qDebug() << "No IDs provided.";
        return; // 直接返回
    }
    // 动态构建 SQL 查询语句
    QString sql = "SELECT F_Guid, F_DeviceNo, F_UserNo, F_WorkStationIp, F_FileName, F_Type, "
                  "F_LocalStorageDriveLetter, F_ServerStorageDriveLetter, F_FilePath, F_ShootTime, "
                  "F_UploadTime, F_FileSize, F_Thumbnail, F_IsImportant, F_FtpUploadStatus, "
                  "F_IsEncrypt, F_IsTranscode, F_CompStatus, F_FileDataSyncStatus, F_Remark, "
                  "F_FileSource, F_IsDelete, F_FileTime FROM t_file WHERE id IN (";

    QStringList placeholders;
    for (int i = 0; i < ids.size(); ++i) {
        placeholders << "?"; // 添加占位符
    }
    sql += placeholders.join(", ") + ")";

    // 准备查询
    if (!sql_query->prepare(sql)) {
        qDebug() << "Failed to prepare query:" << sql_query->lastError().text();
        return;
    }

    // Debugging: check counts
//    qDebug() << "Number of placeholders:" << placeholders.size();
//    qDebug() << "Number of IDs:" << ids.size();

    // 逐个添加绑定值
    for (const QString& id : ids) {
        QString trimmedId = id.trimmed(); // 去除空格
        qDebug() << "Binding ID:" << trimmedId;
        sql_query->addBindValue(trimmedId); // 使用 addBindValue 添加绑定值
    }


   try{
        // 执行查询
        if (!sql_query->exec()) {
            qDebug() << "Failed to execute query doQueryFilesById:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doQueryFilesById  捕获到异常: ============================ " << e.what() << std::endl;
    }


    // 处理结果集
    while (sql_query->next()) {
        QVariantMap fileData;
        fileData["F_Guid"] = sql_query->value("F_Guid").toString();
        fileData["F_DeviceNo"] = sql_query->value("F_DeviceNo").toString();
        fileData["F_UserNo"] = sql_query->value("F_UserNo").toString();
        fileData["F_WorkStationIp"] = sql_query->value("F_WorkStationIp").toString();
        fileData["F_FileName"] = sql_query->value("F_FileName").toString();
        fileData["F_Type"] = sql_query->value("F_Type").toInt();
        fileData["F_LocalStorageDriveLetter"] = sql_query->value("F_LocalStorageDriveLetter").toString();
        fileData["F_ServerStorageDriveLetter"] = sql_query->value("F_ServerStorageDriveLetter").toString();
        fileData["F_FilePath"] = sql_query->value("F_FilePath").toString();
        fileData["F_ShootTime"] = sql_query->value("F_ShootTime").toString();
        fileData["F_UploadTime"] = sql_query->value("F_UploadTime").toString();
        fileData["F_FileSize"] = sql_query->value("F_FileSize").toLongLong();
        fileData["F_Thumbnail"] = sql_query->value("F_Thumbnail").toString();
        fileData["F_IsImportant"] = sql_query->value("F_IsImportant").toInt();
        fileData["F_FtpUploadStatus"] = sql_query->value("F_FtpUploadStatus").toInt();
        fileData["F_IsEncrypt"] = sql_query->value("F_IsEncrypt").toInt();
        fileData["F_IsTranscode"] = sql_query->value("F_IsTranscode").toInt();
        fileData["F_CompStatus"] = sql_query->value("F_CompStatus").toInt();
        fileData["F_FileDataSyncStatus"] = sql_query->value("F_FileDataSyncStatus").toInt();
        fileData["F_Remark"] = sql_query->value("F_Remark").toString();
        fileData["F_FileSource"] = sql_query->value("F_FileSource").toInt();
        fileData["F_IsDelete"] = sql_query->value("F_IsDelete").toInt();
        fileData["F_FileTime"] = sql_query->value("F_FileTime").toInt();
        fileData["F_MarkId"] = sql_query->value("F_MarkId").toInt();


        queriedFiles.append(fileData);
    }

    // 如果 isUploadEnabled 为 true，则发送信号
//    if (isUploadEnabled) {
        qDebug() << "ok to execute query doQueryFilesById:";
        emit mysqlWorkFileClientAdd(queriedFiles);
        //    }
}



void MySqlLite::slotAutoUpload_tfilebyid(QString workstationIp, QList<QMap<QString, QString> > tfileIds)
{
    //qDebug()<< __FUNCTION__ << " =========== MySqlLite ============ 所在线程："<< QThread::currentThreadId();

    QStringList fileIds;

    for (const QMap<QString, QString>& map : tfileIds) {
        QString fileId = map.value("tfile_id");

        if (!fileId.isEmpty()) {
            fileIds.append(fileId);
        }
    }

    if (fileIds.isEmpty()) {
        qDebug() << "No IDs to update.";
        return;
    }

    // 创建 SQL 更新语句
//    QString sqlUpdate = "UPDATE t_file SET F_IsImportant = CASE "
//                        "WHEN F_IsImportant = 0 THEN 1 "
//                        "WHEN F_IsImportant = 1 THEN 0 "
//                        "ELSE F_IsImportant END "
//                        "WHERE Id IN (";

    QString sqlUpdate = "UPDATE t_file SET  F_FileDataSyncStatus =  1  WHERE Id IN (";

    // 添加占位符
    QStringList placeholders;
    for (int i = 0; i < fileIds.size(); ++i) {
        placeholders.append("?");
    }
    sqlUpdate.append(placeholders.join(","));
    sqlUpdate.append(")");

    // 准备 SQL 查询
    sql_query->prepare(sqlUpdate);

    // 绑定参数
    for (int i = 0; i < fileIds.size(); ++i) {
        sql_query->bindValue(i, fileIds[i]);
    }

    try{
        // 执行 SQL 查询
        if (sql_query->exec()) {
            qDebug() << "Records updated successfully.";
        } else {
            qDebug() << "Error updating records:" << sql_query->lastError().text();
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }
}




void MySqlLite::UploadFilebyTime(QString uploadDateTime)
{


    QList<QVariantMap> queriedFiles;

    // 获取当前日期并格式化为字符串
    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    // 准备 SQL 查询语句，查找 F_UploadTime 为当前日期的数据
    sql_query->prepare("SELECT F_Guid, F_DeviceNo, F_UserNo, F_WorkStationIp, F_FileName, F_Type, "
                       "F_LocalStorageDriveLetter, F_ServerStorageDriveLetter, F_FilePath, F_ShootTime, "
                       "F_UploadTime, F_FileSize, F_Thumbnail, F_IsImportant, F_FtpUploadStatus, "
                       "F_IsEncrypt, F_IsTranscode, F_CompStatus, F_FileDataSyncStatus, F_Remark, "
                       "F_FileSource, F_IsDelete, F_FileTime "
                       "FROM t_file "
                       "WHERE DATE(F_UploadTime) = :currentDate");

    // 绑定当前日期
    sql_query->bindValue(":currentDate", currentDate);

    try{
        // 执行查询
        if (!sql_query->exec()) {
            qDebug() << "Failed to execute query doQueryFilesById:" << sql_query->lastError().text();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gett_userinfo_all  捕获到异常: ============================ " << e.what() << std::endl;
    }

    // 处理结果集
    while (sql_query->next()) {
        QVariantMap fileData;
        fileData["F_Guid"] = sql_query->value("F_Guid").toString();
        fileData["F_DeviceNo"] = sql_query->value("F_DeviceNo").toString();
        fileData["F_UserNo"] = sql_query->value("F_UserNo").toString();
        fileData["F_WorkStationIp"] = sql_query->value("F_WorkStationIp").toString();
        fileData["F_FileName"] = sql_query->value("F_FileName").toString();
        fileData["F_Type"] = sql_query->value("F_Type").toInt();
        fileData["F_LocalStorageDriveLetter"] = sql_query->value("F_LocalStorageDriveLetter").toString();
        fileData["F_ServerStorageDriveLetter"] = sql_query->value("F_ServerStorageDriveLetter").toString();
        fileData["F_FilePath"] = sql_query->value("F_FilePath").toString();
        fileData["F_ShootTime"] = sql_query->value("F_ShootTime").toString();

        // 获取 F_UploadTime 并格式化为所需的字符串格式
        QDateTime uploadTime = sql_query->value("F_UploadTime").toDateTime();
        fileData["F_UploadTime"] = uploadTime.toString("yyyy-MM-ddTHH:mm:ss"); // 格式化为 ISO 8601 格式

        fileData["F_FileSize"] = sql_query->value("F_FileSize").toLongLong();
        fileData["F_Thumbnail"] = sql_query->value("F_Thumbnail").toString();
        fileData["F_IsImportant"] = sql_query->value("F_IsImportant").toInt();
        fileData["F_FtpUploadStatus"] = sql_query->value("F_FtpUploadStatus").toInt();
        fileData["F_IsEncrypt"] = sql_query->value("F_IsEncrypt").toInt();
        fileData["F_IsTranscode"] = sql_query->value("F_IsTranscode").toInt();
        fileData["F_CompStatus"] = sql_query->value("F_CompStatus").toInt();
        fileData["F_FileDataSyncStatus"] = sql_query->value("F_FileDataSyncStatus").toInt();
        fileData["F_Remark"] = sql_query->value("F_Remark").toString();
        fileData["F_FileSource"] = sql_query->value("F_FileSource").toInt();
        fileData["F_IsDelete"] = sql_query->value("F_IsDelete").toInt();
        fileData["F_FileTime"] = sql_query->value("F_FileTime").toInt();
        fileData["F_MarkId"] = sql_query->value("F_MarkId").toInt();

        queriedFiles.append(fileData);
    }

    // 发送信号
    qDebug() << "ok to execute query UploadFilebyTime:";
    emit mysqlWorkFileClientAdd(queriedFiles);
}

void MySqlLite::insertT_UserInfoCTtoMSQ(const QVariantMap &copyTUserInfo)
{


    sql_query->prepare("INSERT INTO t_userinfo    (F_UserNo, F_DeviceNo ,F_DepartmentId, F_WorkStationIp, "
                      " F_IsDelete, F_Remark, F_Status ,"
                      " F_CreateDate, F_UpdateDate) "
                      "VALUES (:F_UserNo, :F_DeviceNo,:F_DepartmentId, :F_WorkStationIp, "
                      ":F_IsDelete, :F_Remark,:F_Status, "
                      ":F_CreateDate, :F_UpdateDate)");

    sql_query->bindValue(":F_UserNo", copyTUserInfo["F_UserNo"].toString());
    sql_query->bindValue(":F_DeviceNo", copyTUserInfo["F_DeviceNo"].toString());
//    sql_query->bindValue(":F_UserName", ""); // 静态用户名
    sql_query->bindValue(":F_DepartmentId", 1);
    sql_query->bindValue(":F_WorkStationIp", copyTUserInfo["F_WorkStationIp"].toString());
//    sql_query->bindValue(":F_WorkStationId", 1);
    sql_query->bindValue(":F_IsDelete", 0);
//    sql_query->bindValue(":F_Gender", "Male");
//    sql_query->bindValue(":F_Password", "888888");
//    sql_query->bindValue(":F_Phone", "");
    sql_query->bindValue(":F_Remark", "get by recorder");
#ifdef RELEVANCE
    sql_query->bindValue(":F_Status", 0);
#else
    sql_query->bindValue(":F_Status", 1);
#endif
//    sql_query->bindValue(":F_RoleId", 0);
    sql_query->bindValue(":F_CreateDate", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    sql_query->bindValue(":F_UpdateDate", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));


    try{

        if (!sql_query->exec()) {
            qDebug() << "Error inserting data:" << sql_query->lastError().text();
            insertt_logformainF(copyTUserInfo["F_UserNo"].toString(),6);
        } else {
            qDebug() << "Data inserted successfully!";
    //        copyTUserInfo["F_Department"] = "中国";
            emit mysqlClientAdd(copyTUserInfo);
            insertt_logformainF(copyTUserInfo["F_UserNo"].toString(),6);
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }

}

void MySqlLite::MSQSelectAllDeptManageTODEP(QString userid, QString roleId)
{


    if(roleId == "1"){
       gett_department_all(userid);
    }else if(roleId == "2"){
       getPermissiont_department(userid);
    }else if(roleId == "3"){
       gett_department_onlyme(userid);
    }

}

void MySqlLite::MSQinsertDeptManageTODEP(QString parentId, QString newDepartmentName)
{


    int parentIdValue = parentId.isEmpty() ? 0 : parentId.toInt();

    sql_query->prepare("INSERT INTO t_department (F_Name, F_ParentId) VALUES (?, ?)");
    sql_query->addBindValue(newDepartmentName);
    sql_query->addBindValue(parentIdValue);

    try{
        // 执行插入语句并检查结果
        if (sql_query->exec()) {
            qDebug() << "Inserted new department:" << newDepartmentName << "with parentId:" << parentIdValue;
        } else {
            qDebug() << "Failed to insert department:" << sql_query->lastError().text();
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }


}

void MySqlLite::MSQUpdatet_departmentTODEP(QString parentId, QString newDepartmentName,QString departmentId)
{

    QString updateQuery = "UPDATE t_department "
                          "SET F_Name = :newName, F_ParentId = :parentId, UpdatedAt = CURRENT_TIMESTAMP "
                          "WHERE Id = :departmentId";


    sql_query->prepare(updateQuery);


    sql_query->bindValue(":newName", newDepartmentName);
    sql_query->bindValue(":parentId", parentId.toInt());
    sql_query->bindValue(":departmentId", departmentId.toInt());


    try{
        if (!sql_query->exec()) {
            qDebug() << "MSQUpdatet_departmentTODEP Error updating department:" << sql_query->lastError().text();
        } else {
            qDebug() << "Department updated successfully.";
        }

        }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }

}

void MySqlLite::MSQDeletet_departmentTODEP(QString departmentId)
{

    QString deleteQuery = "UPDATE t_department SET F_IsDelete = 1, UpdatedAt = CURRENT_TIMESTAMP WHERE Id = :departmentId";


    sql_query->prepare(deleteQuery);
    sql_query->bindValue(":departmentId", departmentId.toInt());


    try{
        if (!sql_query->exec()) {
            qDebug() << "Error logically deleting department:" << sql_query->lastError().text();
        } else {
            qDebug() << "MSQDeletet_departmentTODEP Department logically deleted successfully.";
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }


}

void MySqlLite::MSQSelectt_userinfoALLTOUSER(QString userNO, QString roleId,QString userNumber,QString devNumber)
{

//    qDebug()<<"MySqlLite::MSQSelectt_userinfoALLTOUSER START userid:"<<userNO<<" roleId:"<<roleId;
    if(roleId == "1"){
       gett_userinfo_all(userNO,userNumber,devNumber);
    }else if(roleId == "2"){
       getPermissiont_userinfo(userNO,userNumber,devNumber);
    }else if(roleId == "3"){
       gett_userinfo_onlyme(userNO,userNumber,devNumber);
    }
}



void MySqlLite::selectt_filedata(const QString &filePath, int winNum) {


    emit readyToCopyFile(filePath, winNum);
}

void MySqlLite::sloMQSelectUserNo(const QString &username, const QString &password)
{

    QString queryStr;

    // 检查是否需要使用密码
    if (password.isEmpty()) {
        // 如果没有提供密码，构建不包含密码的查询
        queryStr = "SELECT t.F_RoleId FROM t_userinfo t WHERE t.F_UserNo = :username AND t.F_Status = '1' AND t.F_IsDelete = 0";
    } else {
        // 如果提供了密码，构建包含密码的查询
        queryStr = "SELECT t.F_RoleId FROM t_userinfo t WHERE t.F_UserNo = :username AND t.F_Password = :password AND t.F_Status = '1' AND t.F_IsDelete = 0";
    }

    // qDebug() << "Executing query:" << queryStr;

    // 登录查询使用独立QSqlQuery，避免其他定时任务复用成员sql_query时
    // 改写其prepare/bind状态并触发“Parameter count mismatch”。
    QSqlQuery loginQuery(sqliteDatabase);
    if (!loginQuery.prepare(queryStr)) {
        qWarning() << "Failed to prepare login query:" << loginQuery.lastError().text();
        return;
    }
    loginQuery.bindValue(":username", username);

    // 如果有密码，则绑定密码
    if (!password.isEmpty()) {
        loginQuery.bindValue(":password", password);
    }

    try{
        if (!loginQuery.exec()) {
            qDebug() << "Failed to execute query sloMQSelectUserNo:" << loginQuery.lastError().text();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::selectt_filedata  捕获到异常: ============================ " << e.what() << std::endl;
    }


    if (loginQuery.next()) {
        QString roleId = loginQuery.value(0).toString(); // 获取第一列数据 (F_RoleId)
        // qDebug() << "Role ID: " << roleId;


        emit MQSelectUserNoSuc(username, roleId);
    } else {
#ifdef LOCK_LAMP
        emit loginError();
#endif
        qDebug() << "No role found for user: " << username;
        QMessageBox::warning(NULL, tr("Login Fail"), tr("No role found for user: ") + username, QMessageBox::Ok);
    }

    // 检查执行查询后的错误
    if (loginQuery.lastError().isValid()) {
        qDebug() << "Error occurred: " << loginQuery.lastError().text();
    }

}


void MySqlLite::getFiles(const QString &folderPath,QStringList &selectfiles)
{


    QDir directory(folderPath);
    QStringList filters;
    filters << "*.mpg" << "*.avi" << "*.mpv" << "*.m1v" << "*.m2v" << "*.wmv" << "*.asf" << "*.mov" << "*.mp4"
            << "*.mkv" << "*.nmea" << "*.wav" << "*.wma" << "*.mp3" << "*.mp2" << "*.aac" << "*.3gpp"
            << "*.tif" << "*.gif" << "*.jpg" << "*.pcx" << "*.bmp" << "*.png" << "*.tga" << "*.jp2" << "*.j2k"
            << "*.jpeg" << "*.txt" << "*.log" << "*.dat" << "*.gps";
    QStringList files = directory.entryList(filters, QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    foreach (const QString &file, files) {
        QString filePath = directory.filePath(file);
        selectfiles.append(filePath);
    }

    QStringList dirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &dir, dirs) {
        QString subFolderPath = directory.filePath(dir);
        getFiles(subFolderPath,selectfiles);
    }
}

void MySqlLite::getPermissionData(const QString &userNo) {

    QList<FileInfo> file_all;

    QString execText = R"(
        WITH RECURSIVE DepartmentCTE AS (
            SELECT d.id, d.f_parentid
            FROM t_department d
            JOIN t_userinfo u ON u.f_departmentid = d.id
            WHERE u.f_userNo = :userNo
            UNION ALL
            SELECT d.id, d.f_parentid
            FROM t_department d
            JOIN DepartmentCTE c ON d.f_parentid = c.id
        )
        SELECT
            f.Id,
            f.F_Guid,
            f.F_DeviceNo,
            f.F_UserNo,
            f.F_WorkStationIp,
            f.F_FileName,
            f.F_Type,
            f.F_LocalStorageDriveLetter,
            f.F_ServerStorageDriveLetter,
            f.F_FilePath,
            f.F_ShootTime,
            f.F_UploadTime,
            f.F_FileSize,
            f.F_FileTime,
            f.F_Thumbnail,
            f.F_IsImportant,
            f.F_FtpUploadStatus,
            f.F_IsEncrypt,
            f.F_IsTranscode,
            f.F_CompStatus,
            f.F_FileDataSyncStatus,
            f.F_Remark,
            f.F_FileSource,
            f.F_IsDelete,
            u.F_UserName,
            f.F_MarkId
        FROM t_file f
        JOIN t_userinfo u ON f.f_userNo = u.f_userNo
        WHERE u.f_departmentid IN (SELECT id FROM DepartmentCTE);
    )";

    if (!sql_query->prepare(execText)) {
        qDebug() << "SQL preparation error:" << sql_query->lastError().text();
        return;
    }

    // 绑定参数
    sql_query->bindValue(":userNo", userNo);

    try{
        if (!sql_query->exec()) {
            qDebug() << "SQL execution error:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::selectt_filedata  捕获到异常: ============================ " << e.what() << std::endl;
    }


    if (sql_query->exec()) {

        while (sql_query->next()) {

                FileInfo fileinfo = fillFileInfo(*sql_query);
                file_all.append(fileinfo);

//                fileinfo.F_Type = getFileTypeDescription(sql_query->value(6).toString());
//                fileinfo.F_UserNo = sql_query->value(3).toString();
//                fileinfo.F_DeviceNo = sql_query->value(2).toString();
//                fileinfo.F_ShootTime = sql_query->value(10).toString();
//                fileinfo.F_UploadTime = sql_query->value(11).toString();
//                fileinfo.F_IsImportant = getIsImportantDescription(sql_query->value(15).toString());
//                fileinfo.F_Remark = sql_query->value(21).toString();
//                fileinfo.file_uuid = sql_query->value(0).toString();
//                fileinfo.file_name = sql_query->value(5).toString();
//                fileinfo.filepath = sql_query->value(7).toString();
//                fileinfo.F_WorkStationIp = sql_query->value(4).toString();


            }

//        qDebug() << "MySqlLite::getPermissionData found files: " << file_all.size();
        emit gettb_files_success(file_all);
    } else {
        // 如果执行查询失败，输出错误信息
        qDebug() << "Error executing query: " << sql_query->lastError().text();
    }
}

void MySqlLite::slotDelet_tfilebyid(QString workstationIp, QList<QMap<QString, QString>> tfileIds)
{


    QStringList fileIds;
    QJsonArray guidList;

    QStringList filePaths;

    for (const QMap<QString, QString>& map : tfileIds) {
        QString fileId = map.value("tfile_id");
        QString userNo = map.value("F_UserNo");
        QString f_guid = map.value("F_Guid");

        QString f_LocalStorageDriveLetter  = map.value("F_LocalStorageDriveLetter");
        int _signimp = map.value("signimp").toInt();

        if(!f_LocalStorageDriveLetter.isEmpty()){

            // QFile file(fileName);
            // file.remove();
            // QFile::link(fileName, "/dev/null");

            if(_signimp == 0){
                filePaths.append(f_LocalStorageDriveLetter);
            }


        }

        if (!fileId.isEmpty()) {
            fileIds.append(fileId);
        }
        if (!userNo.isEmpty()) {
            QJsonObject guid;
            guid["F_UserNo"] = userNo;
            guid["F_Guid"] = f_guid;
            guidList.append(guid);
        }

    }

    if (fileIds.isEmpty()) {
        qDebug() << "No IDs to delete.";
        return;
    }

    // 创建 SQL 删除语句，添加条件以防止删除重要文件
    QString deleteSql = "DELETE FROM t_file WHERE Id IN (";
    QStringList placeholders;

    // 为每个 ID 添加占位符
    for (int i = 0; i < fileIds.size(); ++i) {
        placeholders.append("?");
    }
    deleteSql.append(placeholders.join(","));
    deleteSql.append(") AND F_IsImportant = 0 AND F_MarkId = 0");

    // 准备 SQL 查询
    sql_query->prepare(deleteSql);

    // 绑定参数
    for (int i = 0; i < fileIds.size(); ++i) {
        sql_query->bindValue(i, fileIds[i]);
    }

    try{

        // 执行 SQL 查询
        if (sql_query->exec()) {
            qDebug() << "Records deleted successfully.";

            foreach (QString _tmpPath, filePaths) {

                qDebug() << "delte filepath: " << _tmpPath;

                QFile _delfile(_tmpPath);
                _delfile.remove();


            }

            emit MSQFile_ClientDelete_TONET(workstationIp, guidList);
        } else {
            qDebug() << "Error deleting records:" << sql_query->lastError().text();
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }


}

void MySqlLite::BatchInsertt_userinfoToUSER(QList<QMap<QString, QVariant>> userInfoList)
{

    sqliteDatabase.transaction();

     for(int i = 0;i < userInfoList.size();++i)
     {
         QString userNo = userInfoList.at(i)["userNo"].toString();
         if (!isValidUserName(userNo)) {
             qDebug() << "Invalid userName ERROR: " << userNo;
             return;
         }
         sql_query->prepare("INSERT INTO t_userinfo (F_UserNo, F_DeviceNo, F_UserName, F_DepartmentId, F_Status, F_RoleId, F_Password, F_CreateDate, F_UpdateDate) "
                   "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
         sql_query->addBindValue(userInfoList.at(i)["userNo"].toString());
         sql_query->addBindValue(userInfoList.at(i)["deviceNo"].toString());
         sql_query->addBindValue(userInfoList.at(i)["userName"].toString());
         sql_query->addBindValue(userInfoList.at(i)["departmentId"].toInt());
         sql_query->addBindValue(userInfoList.at(i)["statusValue"].toString());
         sql_query->addBindValue(userInfoList.at(i)["roleValue"].toInt());
         sql_query->addBindValue(userInfoList.at(i)["password"].toString());
         sql_query->addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
         sql_query->addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

         try{

             if (!sql_query->exec())
             {
                 sqliteDatabase.rollback();
                 sqliteDatabase.commit();
                 qDebug() << "MySqlLite::MSQInsertt_userinfoToUSER err" << sql_query->lastError().text();
                 return;
             }

         }catch(const std::exception& e){

             // 处理异常
              std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
         }

     }

     sqliteDatabase.commit();

     emit batchInsertUserInfoListResult(userInfoList.size());
}

int MySqlLite::insertOrUpdateDept(QString parentId, QString newDepartmentName)
{

    int parentIdValue = parentId.isEmpty() ? 0 : parentId.toInt();

    QString querySql = QString("select Id from t_department where F_Name = '%1'").arg(newDepartmentName);
    sql_query->prepare(querySql);
    if (sql_query->exec()) {
//        qDebug() << querySql;
    } else {
        qDebug() << "Failed to querySql:" << sql_query->lastError().text();
    }

    int column1 = -1;
    // 遍历查询结果
    while (sql_query->next()) {
        column1 = sql_query->value(0).toInt();
        break;
    }

    if(column1 == -1){
        //todo insert
        sql_query->prepare("INSERT INTO t_department (F_Name, F_ParentId) VALUES (?, ?)");
        sql_query->addBindValue(newDepartmentName);
        sql_query->addBindValue(parentIdValue);

        try{
            // 执行插入语句并检查结果
            if (sql_query->exec()) {
    //            qDebug() << "Inserted new department:" << newDepartmentName << "with parentId:" << parentIdValue;
            } else {
                qDebug() << "Failed to insert department:" << sql_query->lastError().text();
            }

        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================ MySqlLite::insertOrUpdateDept 捕获到异常: ============================ " << e.what() << std::endl;
        }

        return sql_query->lastInsertId().toInt();

    }else{
        return column1;
    }
}


NewUserInfo MySqlLite::getUserInfoByUserNo(QString userNo){

    NewUserInfo userInfoInfo;
    userInfoInfo.F_UserNo = userNo;

    QString queryString = QString("SELECT user.Id, user.F_UserNo, user.F_DeviceNo, user.F_UserName, "
                          "user.F_DepartmentId, dep.F_Name, user.F_WorkStationIp, "
                          "user.F_WorkStationId, user.F_IsDelete, user.F_Gender, "
                          "user.F_Password, user.F_Phone, user.F_Remark, "
                          "user.F_Status, user.F_RoleId, user.F_CreateDate, "
                          "user.F_UpdateDate, user.CreatedAt, user.UpdatedAt ,dep.F_Name "
                          "FROM t_userinfo user "
                          "INNER JOIN t_department dep ON user.F_DepartmentId = dep.Id "
                          "WHERE user.F_UserNo = '%1';").arg(userNo);


    try{

        if (!sql_query->exec(queryString)) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return userInfoInfo;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
    }


    while (sql_query->next()) {
        userInfoInfo.id = sql_query->value(0).toString();
        userInfoInfo.F_UserNo = sql_query->value(1).toString();
        userInfoInfo.F_DeviceNo = sql_query->value(2).toString();
        userInfoInfo.F_UserName = sql_query->value(3).toString();
        userInfoInfo.F_DepartmentId = sql_query->value(4).toString();
        userInfoInfo.f_name = sql_query->value(5).toString();
        userInfoInfo.F_WorkStationIp = sql_query->value(6).toString();
        userInfoInfo.F_WorkStationId = sql_query->value(7).toString();
        userInfoInfo.F_IsDelete = sql_query->value(8).toString();
        userInfoInfo.F_Gender = sql_query->value(9).toString();
        userInfoInfo.F_Password = sql_query->value(10).toString();
        userInfoInfo.F_Phone = sql_query->value(11).toString();
        userInfoInfo.F_Remark = sql_query->value(12).toString();
#ifdef RELEVANCE
        userInfoInfo.F_Status = (sql_query->value(13).toString() == "1") ? "关联" : "未关联";

#else
        userInfoInfo.F_Status = (sql_query->value(13).toString() == "1") ? "启用" : "禁用";

#endif



        const QString roleId = sql_query->value(14).toString();
        static const QMap<QString, QString> roleMap = {
            {"1", "超级管理员"},
            {"2", "普通管理员"},
            {"3", "普通用户"}
        };
        userInfoInfo.F_RoleId = roleMap.value(roleId, "未知角色");

        userInfoInfo.create_time = sql_query->value(15).toString();

        userInfoInfo.departmentName = sql_query->value(19).toString();

        break;
    }

    return userInfoInfo;
}

bool MySqlLite::isValidUserName(const QString &userNo) {
      QRegularExpression regex("^[\\p{Han}a-zA-Z0-9_\\sIVXLCDM\\-（）]+$");
    return regex.match(userNo).hasMatch();
}

void MySqlLite::createTableIfNotExists(const QString &tableName, const QString &query) {


    // 检查表是否存在
    sql_query->exec("SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "'");
    if (!sql_query->next()) {
        // 如果表不存在，创建表
        if (!sql_query->exec(query)) {
            qDebug() << "Error creating table:" << sql_query->lastError();
        } else {
            qDebug() << "Table created successfully";
        }
    } else {
        qDebug() << "Table already exists";
    }
}

void MySqlLite::initTables(){

    //finger feature table.
    QString tableName = "t_finger_feature";
    QString createQuery = "CREATE TABLE " + tableName + " (id INTEGER PRIMARY KEY AUTOINCREMENT, userNo TEXT NOT NULL,userName TEXT,featureData BLOB NOT NULL,ext1 TEXT,ext2 TEXT)";
    createTableIfNotExists(tableName,createQuery);

}


void MySqlLite::insertFingerFeature(FingerFeatureData data){

    QString queryString = QString("SELECT t.id,t.userNo,t.userName,t.featureData,t.ext1,t.ext2 "
                          "FROM t_finger_feature t "
                          "WHERE t.userNo = '%1';").arg(data.userNo);
    try{
        if (!sql_query->exec(queryString)) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return ;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::insertFingerFeature  捕获到异常: ============================ " << e.what() << std::endl;
    }

    int column1 = -1;
    // 遍历查询结果
    while (sql_query->next()) {
        column1 = sql_query->value(0).toInt();
        break;
    }

    if(column1 == -1){
        //todo insert
        sql_query->prepare("INSERT INTO t_finger_feature(userNo,userName,featureData,ext1,ext2) VALUES (?,?,?,?,?)");
        sql_query->addBindValue(data.userNo);
        sql_query->addBindValue(data.userName);
        sql_query->addBindValue(data.featureData);
        sql_query->addBindValue(data.ext1);
        sql_query->addBindValue(data.ext2);


        try{
            // 执行插入语句并检查结果
            if (sql_query->exec()) {
    //            qDebug() << "Inserted t_finger_feature:" << newDepartmentName << "with parentId:" << parentIdValue;
            } else {
                qDebug() << "Failed to insert t_finger_feature:" << sql_query->lastError().text();
            }

        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
        }


    }else{
        //todo update
        QString updateQuery = "UPDATE t_finger_feature "
                              "SET featureData = :featureData "
                              "WHERE userNo = :userNo";
        sql_query->prepare(updateQuery);

        sql_query->bindValue(":featureData", data.featureData);
        sql_query->bindValue(":userNo", data.userNo);


        try{

            if (!sql_query->exec()) {
                qDebug() << "insertFingerFeature:t_finger_feature update error on insert fuction:" << sql_query->lastError().text();
            } else {
                qDebug() << "insertFingerFeature:t_finger_feature update successfully.";
            }

        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
        }


    }

}
void MySqlLite::deleteFingerFeature(QString userNo){


    if(userNo.isEmpty()){
        return;
    }
    //todo delete.
    QString updateQuery = "delete from t_finger_feature "
                          "WHERE userNo=:userNo";
    sql_query->prepare(updateQuery);
    sql_query->bindValue(":userNo", userNo);

    try{
        if (!sql_query->exec()) {
            qDebug() << "deleteFingerFeature:t_finger_feature delete error on insert fuction:" << sql_query->lastError().text();
        } else {
            qDebug() << "deleteFingerFeature:t_finger_feature delete successfully.";
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::deleteFingerFeature 捕获到异常: ============================ " << e.what() << std::endl;
    }


}

void MySqlLite::deleteFingerFeatureList(QStringList userNoList){


    for(const QString &item : userNoList)
    {
       deleteFingerFeature(item);
    }
}

void MySqlLite::updateFingerFeature(QString userNo,FingerFeatureData data){


    QString updateQuery = "UPDATE t_finger_feature "
                          "SET featureData = :featureData "
                          "WHERE userNo = :userNo";
    sql_query->prepare(updateQuery);

    sql_query->bindValue(":featureData", data.featureData);
    sql_query->bindValue(":userNo", userNo);

    try{
        if (!sql_query->exec()) {
            qDebug() << "updateFingerFeature:t_finger_feature update error on insert fuction:" << sql_query->lastError().text();
        } else {
            qDebug() << "updateFingerFeature:t_finger_feature update successfully.";
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::updateFingerFeature 捕获到异常: ============================ " << e.what() << std::endl;
    }


}

void MySqlLite::getFingerFeatureAll(){

    mAllFingerFeature.clear();

    QString queryString = QString("SELECT t.userNo,t.userName,t.featureData,t.ext1,t.ext2 "
                          "FROM t_finger_feature t LIMIT 10000");

    try{
        if (!sql_query->exec(queryString)) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return ;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::getFingerFeatureAll 捕获到异常: ============================ " << e.what() << std::endl;
    }



    // 遍历查询结果
    while (sql_query->next()) {
        FingerFeatureData tmpData;
        tmpData.userNo = sql_query->value(0).toString();
        tmpData.userName = sql_query->value(1).toString();
        tmpData.featureData = sql_query->value(2).toByteArray();
        tmpData.ext1 = sql_query->value(3).toString();
        tmpData.ext2 = sql_query->value(4).toString();
        mAllFingerFeature.push_back(tmpData);
    }
}

std::vector<FingerFeatureData> MySqlLite::getFingerFeatureList(){

    getFingerFeatureAll();
    return mAllFingerFeature;
}

FingerFeatureData MySqlLite::queryFingerFeature(QString userNo){

    FingerFeatureData result;
    result.userNo = "";
    QString queryString = QString("SELECT t.userNo,t.userName,t.featureData,t.ext1,t.ext2 "
                          "FROM t_finger_feature t "
                          "WHERE t.userNo = '%1';").arg(userNo);

    try{
        if (!sql_query->exec(queryString)) {
            qDebug() << "Query execution error:" << sql_query->lastError().text();
            return result;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::getFingerFeatureAll 捕获到异常: ============================ " << e.what() << std::endl;
    }

    // 遍历查询结果
    while (sql_query->next()) {
        result.userNo = sql_query->value(0).toString();
        result.userName = sql_query->value(1).toString();
        result.featureData = sql_query->value(2).toByteArray();
        result.ext1 = sql_query->value(3).toString();
        result.ext2 = sql_query->value(4).toString();

        break;
    }
    return result;

}


void MySqlLite::get_realdb_files_type( QString roleId,QString usrNO ,QString filetype ,QString pernumber,QString pername, QString significance,
                           QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend , QString  dateEditstart_create ,
                                       QString  dateEditend_create,int maxRowsToDisplay  ,int page) {

       if(roleId == "1"){

           get_realdb_files_all( usrNO, filetype , pernumber, pername,  significance,  equipnumber,   remark ,   dateEditstart ,   dateEditend,  dateEditstart_create, dateEditend_create, maxRowsToDisplay,page);
       }else if(roleId == "2"){

          get_realdb_PermissionData( usrNO, filetype , pernumber, pername,  significance,  equipnumber,   remark ,   dateEditstart ,   dateEditend,  dateEditstart_create, dateEditend_create,  maxRowsToDisplay,page);
       }else if(roleId == "3"){

          get_realdb_files_onlyme(usrNO, filetype , pernumber, pername,  significance,  equipnumber,   remark ,   dateEditstart ,   dateEditend,   dateEditstart_create, dateEditend_create, maxRowsToDisplay,page);
       }

}



void MySqlLite::get_realdb_files_all( QString userid,QString filetype ,QString pernumber,QString pername, QString significance,
                                      QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend,QString  dateEditstart_create , QString  dateEditend_create, int maxRowsToDisplay ,int page) {


    int l_offset = (page -1) <= 0 ? 0 : (page -1) * maxRowsToDisplay;

    QString l_fileType = makefiletype(filetype);
     qDebug()<<   " l_fileType:  "  <<  l_fileType.toStdString().data() ;


     QString l_significance = makesignificance(significance);
     qDebug()<<   " l_significance:  "  <<  l_significance.toStdString().data() ;

     if(!l_significance.isEmpty()){

         l_significance = QString("  and     f.F_MarkId = '%1'  ").arg(l_significance);
     }


     QString l_pernumber = " ";
     QString l_pername = " ";
     QString l_equipnumber = " ";
     QString l_remark = " ";


     if(!pernumber.isEmpty()){
        l_pernumber = QString("  and  f.F_UserNo = '%1'  ").arg(pernumber);
     }

     if(!pername.isEmpty()){
        l_pername = QString("  and  u.F_UserName = '%1'  ").arg(pername);
     }

     if(!equipnumber.isEmpty()){
        l_equipnumber = QString("  and  f.F_DeviceNo = '%1'  ").arg(equipnumber);
     }

     if(!remark.isEmpty()){

         l_remark = QString("  and  f.F_Remark like  '%%1%'  ").arg(remark);

     }

     QString  l_dateEditstart =" ";
     if(!dateEditstart.isEmpty()){

          l_dateEditstart = QString("  and    strftime('%s',f.F_UploadTime)   >   strftime('%s', '%1')  ").arg(dateEditstart);

          if(dbsType){
                l_dateEditstart = QString("  and    unix_timestamp( f.F_UploadTime)   >   unix_timestamp(  '%1')  ").arg(dateEditstart);
         }

     }


     QString  l_dateEditend =" ";
     if(!dateEditend.isEmpty()){

          l_dateEditend = QString("  and    strftime('%s',f.F_UploadTime)   <   strftime('%s', '%1')  ").arg(dateEditend);

          if(dbsType){
                l_dateEditend = QString("  and    unix_timestamp( f.F_UploadTime)   <   unix_timestamp(  '%1')  ").arg(dateEditend);
         }

     }


     QString  l_dateEditstart_create =" ";
     if(!dateEditstart_create.isEmpty()){

          l_dateEditstart_create = QString("  and    strftime('%s',f.F_ShootTime)   >   strftime('%s', '%1')  ").arg(dateEditstart_create);


         if(dbsType){
                l_dateEditstart_create = QString("  and    unix_timestamp( f.F_ShootTime)   >   unix_timestamp(  '%1')  ").arg(dateEditstart_create);
         }

     }


     QString  l_dateEditend_create =" ";
     if(!dateEditend_create.isEmpty()){

          l_dateEditend_create = QString("  and    strftime('%s',f.F_ShootTime)   <   strftime('%s', '%1')  ").arg(dateEditend_create);


         if(dbsType){
                l_dateEditend_create = QString("  and    unix_timestamp( f.F_ShootTime)   <   unix_timestamp(  '%1')  ").arg(dateEditend_create);
         }

     }



    QList<FileInfo> file_all;

    QString execText = QString(R"( SELECT
                               f.Id,
                               f.F_Guid,
                               f.F_DeviceNo,
                               f.F_UserNo,
                               f.F_WorkStationIp,
                               f.F_FileName,
                               f.F_Type,
                               f.F_LocalStorageDriveLetter,
                               f.F_ServerStorageDriveLetter,
                               f.F_FilePath,
                               f.F_ShootTime,
                               f.F_UploadTime,
                               f.F_FileSize,
                               f.F_FileTime,
                               f.F_Thumbnail,
                               f.F_IsImportant,
                               f.F_FtpUploadStatus,
                               f.F_IsEncrypt,
                               f.F_IsTranscode,
                               f.F_CompStatus,
                               f.F_FileDataSyncStatus,
                               f.F_Remark,
                               f.F_FileSource,
                               f.F_IsDelete,
                               u.F_UserName,
                               f.F_MarkId
                           FROM
                               t_file f
                           LEFT JOIN
                                t_userinfo  u ON f.F_UserNo = u.F_UserNo
                           WHERE
                               f.F_Type IN (%1)
                                    %2   %3   %4   %5   %6  %7  %10 %11  %12
                               order by  f.id  desc

                           LIMIT  %8 ,%9
)").arg(l_fileType).arg(l_dateEditstart).arg(l_dateEditend).arg(l_pernumber).arg(l_pername).arg(l_equipnumber).arg(l_significance)
            .arg( l_offset).arg( maxRowsToDisplay).arg(l_dateEditstart_create).arg(l_dateEditend_create).arg(l_remark);




    try{

        if (!sql_query->exec(execText)) {

            qDebug() << "SQL error:" << sql_query->lastError().text();
            qDebug() << " =========== error lastQuery SQL Statement: =========== " << sql_query->lastQuery();
            return;
        }


    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gettb_files_all 捕获到异常: ============================ " << e.what() << std::endl;
    }


    qDebug() << " =========== succ lastQuery SQL Statement: =========== " << sql_query->lastQuery();


    while (sql_query->next()) {
        FileInfo fileinfo = fillFileInfo(*sql_query);
        file_all.append(fileinfo);
    }

    if (!file_all.isEmpty()) {
        qDebug() << " ===============  MySqlLite::gettb_files_all gettb_files_success =============== 2025 ";

        emit realdb_files_success(file_all);
    } else {
        qDebug() << "No files found in the database.";
    }
}


void MySqlLite::get_realdb_files_onlyme( QString userid,QString filetype ,QString pernumber,QString pername, QString significance,
                                    QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend,QString  dateEditstart_create , QString  dateEditend_create,int maxRowsToDisplay,int page) {


    int l_offset = (page -1) <= 0 ? 0 : (page -1) * maxRowsToDisplay;

    QString l_fileType = makefiletype(filetype);
     qDebug()<<   " l_fileType:  "  <<  l_fileType.toStdString().data() ;


     QString l_significance = makesignificance(significance);
     qDebug()<<   " l_significance:  "  <<  l_significance.toStdString().data() ;

     if(!l_significance.isEmpty()){

         l_significance = QString("  and     f.F_MarkId = '%1'  ").arg(l_significance);
     }


     QString l_pernumber = " ";
     QString l_pername = " ";
     QString l_equipnumber = " ";
     QString l_remark = " ";


     if(!pernumber.isEmpty()){
        l_pernumber = QString("  and  f.F_UserNo = '%1'  ").arg(pernumber);
     }

     if(!pername.isEmpty()){
        l_pername = QString("  and  u.F_UserName = '%1'  ").arg(pername);
     }

     if(!equipnumber.isEmpty()){
        l_equipnumber = QString("  and  f.F_DeviceNo = '%1'  ").arg(equipnumber);
     }

     if(!remark.isEmpty()){

         l_remark = QString("  and  f.F_Remark like  '%%1%'  ").arg(remark);


     }

      QString  l_dateEditstart =" ";
     if(!dateEditstart.isEmpty()){

          l_dateEditstart = QString("  and    strftime('%s',f.F_UploadTime)   >   strftime('%s', '%1')  ").arg(dateEditstart);


         if(dbsType){
                l_dateEditstart = QString("  and    unix_timestamp( f.F_UploadTime)   >   unix_timestamp(  '%1')  ").arg(dateEditstart);
         }

     }


     QString  l_dateEditend =" ";
     if(!dateEditend.isEmpty()){

          l_dateEditend = QString("  and    strftime('%s',f.F_UploadTime)   <   strftime('%s', '%1')  ").arg(dateEditend);


         if(dbsType){
                l_dateEditend = QString("  and    unix_timestamp( f.F_UploadTime)   <   unix_timestamp(  '%1')  ").arg(dateEditend);
         }

     }


      QString  l_dateEditstart_create =" ";
      if(!dateEditstart_create.isEmpty()){

           l_dateEditstart_create = QString("  and    strftime('%s',f.F_ShootTime)   >   strftime('%s', '%1')  ").arg(dateEditstart_create);

           if(dbsType){
                 l_dateEditstart_create = QString("  and    unix_timestamp( f.F_ShootTime)   >   unix_timestamp(  '%1')  ").arg(dateEditstart_create);
          }

      }


      QString  l_dateEditend_create =" ";
      if(!dateEditend_create.isEmpty()){

           l_dateEditend_create = QString("  and    strftime('%s',f.F_ShootTime)   <   strftime('%s', '%1')  ").arg(dateEditend_create);

           if(dbsType){
                 l_dateEditend_create = QString("  and    unix_timestamp( f.F_ShootTime)   <   unix_timestamp(  '%1')  ").arg(dateEditend_create);
          }

      }


    QList<FileInfo> file_all;
    QString execText = QString(R"(
        SELECT
            f.Id,
            f.F_Guid,
            f.F_DeviceNo,
            f.F_UserNo,
            f.F_WorkStationIp,
            f.F_FileName,
            f.F_Type,
            f.F_LocalStorageDriveLetter,
            f.F_ServerStorageDriveLetter,
            f.F_FilePath,
            f.F_ShootTime,
            f.F_UploadTime,
            f.F_FileSize,
            f.F_FileTime,
            f.F_Thumbnail,
            f.F_IsImportant,
            f.F_FtpUploadStatus,
            f.F_IsEncrypt,
            f.F_IsTranscode,
            f.F_CompStatus,
            f.F_FileDataSyncStatus,
            f.F_Remark,
            f.F_FileSource,
            f.F_IsDelete,
            u.F_UserName,
            f.F_MarkId
        FROM
              t_file f
        LEFT JOIN
            t_userinfo   u ON f.F_UserNo = u.F_UserNo
        WHERE
             f.F_Type IN (%1) AND f.F_UserNo = '%2'
                %3   %4   %5   %6  %7 %8   %11  %12 %13
            order by  f.id  desc

       LIMIT  %9, %10

    )").arg(l_fileType).arg(userid).arg(l_dateEditstart).arg(l_dateEditend).arg(l_pernumber).arg(l_pername).arg(l_equipnumber).arg(l_significance).arg(l_offset).arg(maxRowsToDisplay ).arg(l_dateEditstart_create).arg(l_dateEditend_create).arg(l_remark);;

//    qDebug() << "gettb_files_onlyme execText:" << execText;

    //sql_query->prepare(execText);
   // sql_query->bindValue(":userid", userid);

    try{
        if (!sql_query->exec(execText)) {
            qDebug() << "SQL error:" << sql_query->lastError().text();
             qDebug() << " =========== error lastQuery SQL Statement: =========== " << sql_query->lastQuery();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::gettb_files_onlyme 捕获到异常: ============================ " << e.what() << std::endl;
    }


     qDebug() << " =========== succ lastQuery SQL Statement: =========== " << sql_query->lastQuery();

    while (sql_query->next()) {
        FileInfo fileinfo = fillFileInfo(*sql_query);
        file_all.append(fileinfo);
    }

    if (!file_all.isEmpty()) {
        qDebug() << "MySqlLite::gettb_files_onlyme gettb_files_success";

         emit realdb_files_success(file_all);

    } else {
        qDebug() << "No files found for user:" << userid;
    }
}


void MySqlLite::get_realdb_PermissionData(  QString userid,QString filetype ,QString pernumber,QString pername, QString significance,
                                          QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend,QString  dateEditstart_create , QString  dateEditend_create, int maxRowsToDisplay, int page) {

    int l_offset = (page -1) <= 0 ? 0 : (page -1) * maxRowsToDisplay;

    QString l_fileType = makefiletype(filetype);
     qDebug()<<   " l_fileType:  "  <<  l_fileType.toStdString().data() ;


     QString l_significance = makesignificance(significance);
     qDebug()<<   " l_significance:  "  <<  l_significance.toStdString().data() ;

     if(!l_significance.isEmpty()){

         l_significance = QString("  and     f.F_MarkId = '%1'  ").arg(l_significance);
     }


     QString l_pernumber = " ";
     QString l_pername = " ";
     QString l_equipnumber = " ";
     QString l_remark = " ";


     if(!pernumber.isEmpty()){
        l_pernumber = QString("  and  f.F_UserNo = '%1'  ").arg(pernumber);
     }

     if(!pername.isEmpty()){
        l_pername = QString("  and  u.F_UserName = '%1'  ").arg(pername);
     }

     if(!equipnumber.isEmpty()){
        l_equipnumber = QString("  and  f.F_DeviceNo = '%1'  ").arg(equipnumber);
     }

     if(!remark.isEmpty()){

         l_remark = QString("  and  f.F_Remark like  '%%1%'  ").arg(remark);


     }

      QString  l_dateEditstart =" ";
     if(!dateEditstart.isEmpty()){

          l_dateEditstart = QString("  and    strftime('%s',f.F_UploadTime)   >   strftime('%s', '%1')  ").arg(dateEditstart);

          if(dbsType){
                l_dateEditstart = QString("  and    unix_timestamp( f.F_UploadTime)   >   unix_timestamp(  '%1')  ").arg(dateEditstart);
         }

     }


     QString  l_dateEditend =" ";
     if(!dateEditend.isEmpty()){

          l_dateEditend = QString("  and    strftime('%s',f.F_UploadTime)   <   strftime('%s', '%1')  ").arg(dateEditend);

          if(dbsType){
                l_dateEditend = QString("  and    unix_timestamp( f.F_UploadTime)   <   unix_timestamp(  '%1')  ").arg(dateEditend);
         }

     }


      QString  l_dateEditstart_create =" ";
      if(!dateEditstart_create.isEmpty()){

           l_dateEditstart_create = QString("  and    strftime('%s',f.F_ShootTime)   >   strftime('%s', '%1')  ").arg(dateEditstart_create);

           if(dbsType){
                 l_dateEditstart_create = QString("  and    unix_timestamp( f.F_ShootTime)   >   unix_timestamp(  '%1')  ").arg(dateEditstart_create);
          }

      }


      QString  l_dateEditend_create =" ";
      if(!dateEditend_create.isEmpty()){

           l_dateEditend_create = QString("  and    strftime('%s',f.F_ShootTime)   <   strftime('%s', '%1')  ").arg(dateEditend_create);

           if(dbsType){
                 l_dateEditend_create = QString("  and    unix_timestamp( f.F_ShootTime)   <   unix_timestamp(  '%1')  ").arg(dateEditend_create);
          }

      }



    QList<FileInfo> file_all;

    QString execText = QString(R"(
        WITH RECURSIVE DepartmentCTE AS (
            SELECT d.id, d.f_parentid
            FROM t_department d
            JOIN t_userinfo u ON u.f_departmentid = d.id
            WHERE u.f_userNo = '%1'
            UNION ALL
            SELECT d.id, d.f_parentid
            FROM t_department d
            JOIN DepartmentCTE c ON d.f_parentid = c.id
        )
        SELECT
            f.Id,
            f.F_Guid,
            f.F_DeviceNo,
            f.F_UserNo,
            f.F_WorkStationIp,
            f.F_FileName,
            f.F_Type,
            f.F_LocalStorageDriveLetter,
            f.F_ServerStorageDriveLetter,
            f.F_FilePath,
            f.F_ShootTime,
            f.F_UploadTime,
            f.F_FileSize,
            f.F_FileTime,
            f.F_Thumbnail,
            f.F_IsImportant,
            f.F_FtpUploadStatus,
            f.F_IsEncrypt,
            f.F_IsTranscode,
            f.F_CompStatus,
            f.F_FileDataSyncStatus,
            f.F_Remark,
            f.F_FileSource,
            f.F_IsDelete,
            u.F_UserName,
            f.F_MarkId
        FROM t_file f
        JOIN t_userinfo u ON f.f_userNo = u.f_userNo
        WHERE u.f_departmentid IN (SELECT id FROM DepartmentCTE)
            and   f.F_Type IN (%2)
             %3   %4   %5   %6  %7 %8   %11  %12 %13
            order by  f.id  desc

            LIMIT  %9,%10

    )").arg(userid).arg(l_fileType).arg(l_dateEditstart).arg(l_dateEditend).arg(l_pernumber).arg(l_pername).arg(l_equipnumber).arg(l_significance).arg(l_offset).arg(maxRowsToDisplay ).arg(l_dateEditstart_create).arg(l_dateEditend_create).arg(l_remark);


    //    if (!sql_query->prepare(execText)) {
    //        qDebug() << "SQL preparation error:" << sql_query->lastError().text();
    //        return;
    //    }


    try{
        if (!sql_query->exec(execText)) {
            qDebug() << "SQL execution error:" << sql_query->lastError().text();
            return;
        }

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ MySqlLite::selectt_filedata  捕获到异常: ============================ " << e.what() << std::endl;
    }



        while (sql_query->next()) {

                FileInfo fileinfo = fillFileInfo(*sql_query);
                file_all.append(fileinfo);

//                fileinfo.F_Type = getFileTypeDescription(sql_query->value(6).toString());
//                fileinfo.F_UserNo = sql_query->value(3).toString();
//                fileinfo.F_DeviceNo = sql_query->value(2).toString();
//                fileinfo.F_ShootTime = sql_query->value(10).toString();
//                fileinfo.F_UploadTime = sql_query->value(11).toString();
//                fileinfo.F_IsImportant = getIsImportantDescription(sql_query->value(15).toString());
//                fileinfo.F_Remark = sql_query->value(21).toString();
//                fileinfo.file_uuid = sql_query->value(0).toString();
//                fileinfo.file_name = sql_query->value(5).toString();
//                fileinfo.filepath = sql_query->value(7).toString();
//                fileinfo.F_WorkStationIp = sql_query->value(4).toString();


            }



        if (!file_all.isEmpty()) {
            // qDebug() << "MySqlLite::getPermissionData found files: " << file_all.size();
            // emit gettb_files_success(file_all);

            emit realdb_files_success(file_all);

        } else {
            qDebug() << "No files found for user:" << userid;
        }


}
