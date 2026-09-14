#include "zfycontrol.h"
#include <QtConcurrent/QtConcurrent>
#include <qthreadpool.h>
#include "videocompressor.h"




ZFYControl::ZFYControl(bool autodeletecopyfile,QObject *parent) : QObject(parent)
{
    qDebug()<<"ZFYControl::start "<<zfynum<<endl;
    path = "";
    this->autodeletecopyfile = autodeletecopyfile; 
    qRegisterMetaType<QVariant>("QVariant");

    failNums = 0;
    is_send_end = false;

    encodevideo = Config::getInstance()->Get("wsConfig","encodevideo").toInt();
    breakgather =  Config::getInstance()->Get("wsConfig", "breakgather").toInt();
}

ZFYControl::ZFYControl(int zfynum)
{
    qDebug()<<"ZFYControl::ZFYControl"<<zfynum;
    this->dirpathCP = QString("%1").arg( Config::getInstance()->Get("wsConfig","dataPath").toString());
    this->zfynum = zfynum;
    isDeleteZFYData =  Config::getInstance()->Get("wsConfig","deleteFile").toBool();
    fileTranslate =  Config::getInstance()->Get("interfaceConfig","fileTranslate").toString();
    ffmpegPath =   Config::getInstance()->Get("wsConfig","FFmpegPath").toString();

    failNums = 0;
    is_send_end = false;

    encodevideo = Config::getInstance()->Get("wsConfig","encodevideo").toInt();
    breakgather =  Config::getInstance()->Get("wsConfig", "breakgather").toInt();

}

ZFYControl::~ZFYControl()
{
    qDebug()<<"ZFYControl::~ZFYControl "<<zfynum<<endl;

}

 void ZFYControl::myCopyFile(const QString &path,int zfywindow){

     mountUsbPath = path;

     if(breakgather == 1){


        QString  _finishedNmae = mountUsbPath + "/finished.ini";
        QFile _fileFinded(_finishedNmae);
        if (!_fileFinded.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text | QIODevice::Unbuffered)) {
            // 处理打开文件失败的情况
            return;
        }

        _fileFinded.flush();
        _fileFinded.close();

        if (_fileFinded.open(QIODevice::ReadOnly | QIODevice::Text)) {
           QTextStream in(&_fileFinded);

           while (!in.atEnd()) {
               QString line = in.readLine();
               readyfiles.insert( line);
           }

           _fileFinded.close();


           for (const QString &str : readyfiles) {
               qDebug() << str;  // 打印每行内容
           }

         } else {
               qDebug() << "Failed to open file!";
         }



    }

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    if(is_mainwindow_exited){
        return;
    }

    //qDebug()<<"ZFYControl::myCopyFile=="<<QThread::currentThreadId()<<""<<path;
    if (path.isEmpty()) {
        qDebug() << "path is empty error";
        return ;
    }

    if(isFolderEmpty(path)){
        qDebug()<<"isFolderEmpty(path)";
        // emit sendzfyProgress(100, zfywindow);
        // emit currentCopyProgressInfo(zfywindow, 100.0, 0, 0);

        // QThread::msleep(500);
        // emit no_Files_Copy_ZFY(path,zfywindow);
        // return;
    }


    QVariantMap copyTUserInfo;
    QString MyHostIPV4Address = getHostIPV4Address().toString();
    QString devNo;
    QString polNo;

    QString jydevNo;
    QString jypolNo;

    QString userName;
    QString corpName;



    zfyConfig zfyconfig;

    if(true){

        if(zfydevicetype == 0 ){

            if (!readConfigFile(path, zfyconfig)) {

                QString  _jyFileNameDat = path + "/DCIM/100MEDIA/Info.dat";
                QFile _jyFile(_jyFileNameDat);


                if (_jyFile.exists()) {
                    // 打开文件
                    if (_jyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                       QByteArray _oneline  = _jyFile.readLine();
                       QString strline = QString::fromUtf8(_oneline);

                       if(!strline.isEmpty()){
                            QStringList strspl = strline.split(";");
                            qDebug() << " =========== info.dat : " << strspl[0] << " , " << strspl[2]  ;

                            jydevNo = strspl[0] ;
                            jypolNo  = strspl[2];

                       }else{

                           return;
                       }

                    }

                }else{

                    return;
                }

            }

        }


        if(zfydevicetype == 1 ){

            if (!readConfigFile(path, zfyconfig)) {

                QString  _jyFileNameDat = path + "/DCIM/100MEDIA/Info.dat";
                QFile _jyFile(_jyFileNameDat);


                if (_jyFile.exists()) {
                    // 打开文件
                    if (_jyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                       QByteArray _oneline  = _jyFile.readLine();
                       QString strline = QString::fromUtf8(_oneline);

                       if(!strline.isEmpty()){
                            QStringList strspl = strline.split(";");
                            qDebug() << " =========== info.dat : " << strspl[0] << " , " << strspl[2]  ;

                            jydevNo = strspl[0] ;
                            jypolNo  = strspl[2];

                       }else{

                           return;
                       }

                    }

                }else{

                    return;
                }


            }

        }




        //    devNo = zfyconfig.devNo;
        //#ifndef CONFIG_NAME
        //    // 如果 CONFIG_NAME 没有定义，默认取 polNo
        //    polNo = zfyconfig.polNo;
        //#else
        //    // 如果 CONFIG_NAME 被定义为 "qunhua"
        //    #if CONFIG_NAME == qunhua
        //        polNo = zfyconfig.devNo; // CONFIG_NAME 是 "qunhua"，取 devNo
        //    #else
        //        polNo = zfyconfig.polNo; // 其它情况取 polNo
        //    #endif
        //#endif



        #ifdef CAR_DEPOT
        //    if(zfyconfig.isFlashLight.compare("false", Qt::CaseInsensitive) == 0){
        //        devNo = zfyconfig.devNo;
        //        polNo = zfyconfig.polNo;
        //    }else{
        //        polNo = zfyconfig.polNo;
        //        devNo = zfyconfig.devNo;
        //    }
            value1 = zfyconfig.isFlashLight.toLower() == "true";
            polNo = zfyconfig.polNo;
            devNo = zfyconfig.devNo;
        #else
            devNo = zfyconfig.devNo.isEmpty() ?  jydevNo : zfyconfig.devNo;
        #ifndef CONFIG_NAME

            polNo = zfyconfig.polNo.isEmpty() ?  jypolNo : zfyconfig.polNo;
        #else
            #if CONFIG_NAME == qunhua
                polNo = zfyconfig.devNo;
            #else
                polNo = zfyconfig.polNo;
            #endif
        #endif // CONFIG_NAME
        #endif // CAR_DEPOT

            copyTUserInfo["F_UserNo"] = polNo;
            copyTUserInfo["F_DeviceNo"] = devNo;
            copyTUserInfo["F_WorkStationIp"] = MyHostIPV4Address;
            copyTUserInfo["F_Department"] = "中国";


            userName = zfyconfig.userName ;
            corpName = zfyconfig.department ;
            emit insertT_UserInfo(copyTUserInfo);


        #ifdef  RELEVANCE
            bool value = zfyconfig.uncorrelated.toLower() == "true";
            if(value){
                qDebug() << "未关联";
                emit no_Files_Copy_ZFY(path,zfywindow);
                return;
            }
        #endif

    }


    if(true){

        emit currentDeviceInfo(zfywindow,polNo,devNo,userName,corpName);


    }

    hsGlobalMtx.lock();
    QString _checkPath =  Config::getInstance()->Get("wsConfig","saveDir").toString();
    if(!dirpathCP.contains(_checkPath + "/")){

        this->dirpathCP = _checkPath + "/" +  Config::getInstance()->Get("wsConfig","saveFilePath").toString();

    }
    hsGlobalMtx.unlock();

    // 用户号/日期/类型
    QString dstFilePath = dirpathCP + "/" + polNo ;
    PathFile = dirpathCP +   "/" + polNo;

    QDir destinationDir(dstFilePath);
    // 如果目标目录不存在，则创建它
    if (!destinationDir.exists()) {
        if (!destinationDir.mkpath(".")) {
            qDebug() << "无法创建目标目录";
            return ;
        }
    }
    QString sourceDirFather = path;

    setZfynum(zfywindow);
    qint64 totalfilesize = 0;
    qint64 copiedfilesize = 0;
    qint64 totalFileCount = 0;
    QStringList copiedsuccessfullypath;


    getFilesSizeFileCountInFolder(path,totalfilesize,totalFileCount);
    emit currentDeviceFileCount(zfywindow,totalFileCount,0);

    if(totalFileCount == 0 ){
        emit currentCopyProgressInfo(zfywindow, 100.0, copiedfilesize, totalfilesize);

    }

    bool stopRecursion = false;

    if(polNo == "000000"){

        //return;
    }

    try{
        copyFilesRecursively(path, dstFilePath, sourceDirFather, totalfilesize,
                             copiedfilesize, polNo, devNo, copiedsuccessfullypath, zfywindow,totalFileCount,0, stopRecursion);

    }catch(const std::exception& e){

        // 处理异常
         std::cerr << " ============================ ZFYControl::copyFilesRecursively 捕获到异常: ============================ " << e.what() << std::endl;
    }

    if( totalfilesize == copiedfilesize ){
        if(!is_send_end){
             emit currentCopyProgressInfo(zfywindow, 100.0, totalfilesize, totalfilesize);
        }

    }


}

void ZFYControl::readBashStandardOutputInfo()
{

    //qDebug()<<"readBashStandardOutputInfo";
    QByteArray cmdout = m_proces_bash->readAllStandardOutput();
    if(!cmdout.isEmpty()){
    }
}

void ZFYControl::readBashStandardErrorInfo()
{
    //qDebug()<<"readBashStandardErrorInfo";
    QByteArray cmdout = m_proces_bash->readAllStandardError();
    if(!cmdout.isEmpty()){
        if(QString::fromLocal8Bit(cmdout).contains("special device") &&QString::fromLocal8Bit(cmdout).contains("does not exist")){
            emit mount_fail();
        }
    }
}


void ZFYControl::setZfynum(int value)
{
    //qDebug()<<"ZFYControl::setZfynum "<<value;
    zfynum = value;
}

void ZFYControl::pauseCopying() {

    isCopying = false; // 暂停拷贝
}

void ZFYControl::resumeCopying() {

    isCopying = true; // 恢复拷贝
    pauseCondition.wakeAll(); // 唤醒所有等待的线程
}
// 暂停拷贝操作
void ZFYControl::pauseCopyingNEW() {

    isCopyingPaused = true;
}

// 恢复拷贝操作
void ZFYControl::resumeCopyingNEW() {

    isCopyingPaused = false;
}



void ZFYControl::copyFilesRecursively(const QString &sourceDir, const QString &destinationDir, const QString &sourceDirFather,
                                          const qint64 &totalfilesize, qint64 &copiedfilesize, const QString &polNo,  QString devNo,
                                          QStringList &copiedsuccessfullypath,int winNum,int totalFileCount,int totalCompleteFileCount,bool stopRecursion) {


    QDateTime _currentDateTime = QDateTime::currentDateTime();
    int  _currStamp = _currentDateTime.toSecsSinceEpoch();

    if(is_mainwindow_exited){
        return;
    }

    QString _dataPath =  Config::getInstance()->Get("wsConfig","dataPath").toString();
    if(!destinationDir.contains(_dataPath)){
        return;
    }

    if (stopRecursion) { // 检查是否应停止递归
        return;
    }
    QDir sourceDirectory(sourceDir);
    QDir destinationDirectory(destinationDir);


    QStringList filters = {
        "*.mpg", "*.avi", "*.mpv", "*.m1v", "*.m2v", "*.wmv", "*.asf", "*.mov", "*.mp4",
        "*.mkv", "*.nmea", "*.wav", "*.wma", "*.mp3", "*.mp2", "*.aac", "*.3gpp",
        "*.tif", "*.gif", "*.jpg", "*.pcx", "*.bmp", "*.png", "*.tga", "*.jp2", "*.j2k",
        "*.jpeg", "*.txt", "*.log", "*.dat", "*.gps"
    };
    int copySuccessFileCount = 0;
    QStringList files = sourceDirectory.entryList(filters, QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    for (const QString &file : files) {

        if(!isCopying){
            return;
        }

//        QString sourceFilePath = sourceDirectory.filePath(file);
//        QString destinationFilePath = destinationDirectory.filePath(file);

        QString sourceFilePath = sourceDirectory.filePath(file);

        if(breakgather == 1){
            int _fpIdx = sourceFilePath.indexOf("/",11);
            QString _prePath = sourceFilePath.mid(_fpIdx);

            if(readyfiles.contains(_prePath)){

                continue;
            }

        }



        // 获取文件类型
        QFileInfo fileInfo(sourceFilePath);
        QString category = fileInfo.suffix();
        QString fileType = getFileType(category);

        // 获取文件的创建日期，如果不可用则用当前日期
        // 检查文件的创建日期
        QString creationDate;
        if (fileInfo.created().isValid()) {
            creationDate = fileInfo.created().toString("yyyyMMdd");
        } else {
            // 检查当前时间是否有效
            if (QDateTime::currentDateTime().isValid()) {
                creationDate = QDateTime::currentDateTime().toString("yyyyMMdd");
            } else {
                // 如果当前时间无效，则设置为 "6666"
                creationDate = "6666";
            }
        }
        QString currentTime ;
        // 检查当前时间是否有效
        if (QDateTime::currentDateTime().isValid()) {
            currentTime = QDateTime::currentDateTime().toString("yyyyMMdd");
        } else {
            // 如果当前时间无效，则设置为 "6666"
            currentTime = "6666";
        }

        #ifdef CAR_DEPOT
        QString sourcePath = sourceDirectory.filePath(file);

        // 获取文件的文件夹名称
        QFileInfo fileIf(sourcePath);
        QString directoryName = fileIf.absolutePath().split('/').last();
        devNo = value1 ? directoryName:devNo;
        QString destinationDirPath = PathFile + "/" + directoryName + "/" + creationDate + "/" + fileType + "/" + currentTime;
        #else

        creationDate = QDateTime::currentDateTime().toString("yyyyMMdd");

        //QString destinationDirPath = PathFile + "/" + creationDate + "/" + fileType + "/" + currentTime;
        QString destinationDirPath = PathFile + "/" + creationDate + "/" + fileType ;


        #endif // CAR_DEPOT


        if (!QDir().mkpath(destinationDirPath)) {
            qDebug() << "Failed to create directory:" << destinationDirPath;
            return; // 退出当前操作
        }

        QString destinationFilePath = destinationDirPath + "/" + file;

        try{

            QFile l_sourceFile(sourceFilePath);
            qint64 l_fileSize = l_sourceFile.size();

            if (!copyFile(sourceFilePath, destinationFilePath,totalfilesize,copiedfilesize,copiedsuccessfullypath,winNum, fileType)) {
    //            qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;

                if (sourceFilePathIsErr){

    //                emit  no_Files_Copy_ZFY(sourceDirFather,zfynum);
    //                return;
                    emit unplugZFYTOCT(sourceDirFather,zfynum);
                    stopRecursion = true;
                    return ;

                }

                failNums += 1;

            }else {
                // 拷贝完成删除一个START
                insert_t_file_data(sourceDirFather,copiedsuccessfullypath, devNo, polNo,winNum);
                // 拷贝完成删除一个end
            }

           // copySuccessFileCount = copiedsuccessfullypath.size() + failNums;
            copySuccessFileCount = copiedsuccessfullypath.size()  ;

            if(l_fileSize < 1024 * 1024 * 4){

            }else{

                 emit currentDeviceFileCount(winNum,totalFileCount,copySuccessFileCount);
            }

            if(totalFileCount == copySuccessFileCount){

                 emit currentDeviceFileCount(winNum,totalFileCount,copySuccessFileCount);
            }




        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================ copyfile 捕获到异常: ============================ " << e.what() << std::endl;
        }


    }


    QStringList dirs = sourceDirectory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dir : dirs) {
        QString sourceDirPath = sourceDirectory.filePath(dir);
        QString destinationDirPath = destinationDirectory.filePath(dir);

        if (sourceFilePathIsErr) { // 检查是否应停止递归
            return;
        }

        try{

            copyFilesRecursively(sourceDirPath, destinationDirPath, sourceDirFather, totalfilesize, copiedfilesize, polNo, devNo, copiedsuccessfullypath,winNum,
                             totalFileCount,copySuccessFileCount,stopRecursion);

        }catch(const std::exception& e){

            // 处理异常
             std::cerr << " ============================ MySqlLite::doinsertfiles 捕获到异常: ============================ " << e.what() << std::endl;
        }

    }

    // 全部拷贝完成删除一个START
    if (sourceDir == sourceDirFather) {
        qDebug() << "Recursive search completed. All files in all subdirectories have been found.";
        emit no_Files_Copy_ZFY(sourceDirFather,zfynum);
//        insert_t_file_data(sourceDirFather,copiedsuccessfullypath, devNo, polNo,winNum);
    }
    // 全部拷贝完成删除一个END
}



bool ZFYControl::copyFile(const QString &sourceFilePath, const QString &destinationFilePath, const qint64 &totalfilesize, qint64 &copiedfilesize, QStringList &copiedsuccessfullypath, int windowNum, QString fileType)
{

    if(is_mainwindow_exited){
        return false;
    }

    if(sourceFilePath.isNull()){
        return false;
    }

     QString _dataPath =  Config::getInstance()->Get("wsConfig","dataPath").toString();

     if(!destinationFilePath.contains(_dataPath)){
         return false;
     }



//    qDebug()<<"ZFYControl::copyFile sendzfyProgress=="<<QThread::currentThreadId();
    QFile sourceFile(sourceFilePath);
    QFile destinationFile(destinationFilePath);

    if (!sourceFile.exists()) {
        sourceFilePathIsErr = true;
//        qDebug() << "Source file does not exist:" << sourceFilePath<<" sourceFilePathIsErr:"<<sourceFilePathIsErr;
        return false;
    }

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open source file for reading:" << sourceFilePath;
        return false;
    }

    if (!destinationFile.open(QIODevice::WriteOnly)) {
//        qDebug() << "Failed to open destination file for writing:" << destinationFilePath;
        sourceFile.close();
        return false;
    }



    //const int bufferSize = 1024 * 1024; // 增大缓冲区
    const int bufferSize = 16 * 1024 * 1024; // 增大缓冲区 4M 2024.11.28
    //char buffer[bufferSize] = {0};

    char* buffer = (char*)malloc(sizeof(char) * bufferSize);
    if (buffer == nullptr) {

       sourceFile.close();
       destinationFile.close();

       return false;
    }

     memset(buffer, 0, sizeof(char) * bufferSize);

    qint64 totalBytesCopied = 0;
    qint64 fileSize = sourceFile.size();
    bool success = true;

    if(encodevideo == 1){

         if("VIDEO" == fileType  ||  "AUDIO" == fileType ){

            QByteArray _bom;
            _bom.append(char(0xEF));
            _bom.append(char(0xBB));
            _bom.append(char(0xBF));
            destinationFile.write(_bom);

            qint64 _lenboms = destinationFile.write(_bom, _bom.size());
            if (_lenboms != _bom.size()) {

                copiedfilesize += fileSize;
                sourceFile.close();
                destinationFile.close();
                free(buffer);

                return false;
            }

         }

    }

    while (totalBytesCopied < fileSize) {

        if(is_mainwindow_exited){

            sourceFile.close();
            destinationFile.close();
            free(buffer);
            buffer = nullptr;

            return false;
        }

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        if (isCopyingPaused) {
            while(isCopyingPaused) {
                QThread::msleep(100);
            }
        }


        QStorageInfo storageInfo(QFileInfo(destinationFilePath).absolutePath());
        if (!storageInfo.isReady()) {
            qDebug() << "USB drive is not ready or has been removed.";
            success = false;
            break;
        }

        qint64 bytesRead = sourceFile.read(buffer, bufferSize);
        if (bytesRead == -1) {

            qint64 _incomplete = fileSize - totalBytesCopied;
            copiedfilesize += _incomplete;


            sourceFile.close();
            destinationFile.close();
            free(buffer);
            buffer = nullptr;

            return false;
        }

        if (bytesRead == 0) break;

        qint64 bytesWritten = destinationFile.write(buffer, bytesRead);
        if (bytesWritten != bytesRead) {
//            qDebug() << "Failed to write data to destination file:" << destinationFilePath;
            success = false;

            qint64 _incomplete = fileSize - totalBytesCopied;
           // copiedfilesize += _incomplete;


            break;
        }


        totalBytesCopied += bytesWritten;
        copiedfilesize += bytesWritten;
        double progress = static_cast<double>(copiedfilesize) / totalfilesize * 100;

        static qint64 _numClocks = 1;

        if(copiedfilesize == totalfilesize || copiedfilesize <  (20 * 1024 * 1024) ) {

            if( copiedfilesize == totalfilesize){

                is_send_end = true;
            }

             emit currentCopyProgressInfo(windowNum,progress,copiedfilesize,totalfilesize);

        }else{

            int _remainder = _numClocks % 10;
            if( _remainder == 0){

                destinationFile.flush();

                emit currentCopyProgressInfo(windowNum,progress,copiedfilesize,totalfilesize);

            }

            _numClocks += 1;


        }


    }




    free(buffer);
    buffer = nullptr;

    sourceFile.close();
    destinationFile.close();

    if (success) {

       // 拷贝完成删除一个START
      if (isDeleteZFYData) {
           //qDebug() << "QFile::remove F_FilePath:" << sourceFilePath;
           //QFile::remove(sourceFilePath);
       }

      // 拷贝完成删除一个 END

       // 压缩转换 1
       if(fileTranslate == "1"){
           if("VIDEO" == fileType ){
               // 创建 VideoCompressor 实例
               VideoCompressor *compressor = new VideoCompressor(QFileInfo(destinationFile).absolutePath(),
                                                                 QFileInfo(destinationFile).baseName(),
                                                                 destinationFilePath,ffmpegPath);

               // 连接信号以处理压缩结果
               connect(compressor, &VideoCompressor::compressionFinished, this, [=](const QString&outputFile,bool success) {
                   if (success) {
                       qDebug() << "Video compression finished successfully. Output file:" << outputFile;
                   } else {
                       qDebug() << "Video compression failed.";
                   }
                   compressor->deleteLater(); // 释放内存
               });

               connect(compressor, &VideoCompressor::errorOccurred, this, [=](const QString& errorMessage) {
                   qDebug() << "Error during video compression:" << errorMessage;
                   compressor->deleteLater(); // 释放内存
               });

               // 开始压缩
               compressor->compress();
           }

       }

        FileMapping file= {sourceFilePath, destinationFilePath};
//        qDebug()<<"ZFYControl::copyFile fileVector.append=="<<file.sourcePath;


        fileVector.append(file);

        copiedsuccessfullypath.append(destinationFilePath);
    }

    return success;


}


void ZFYControl::compressVideo(const QString &sourceFile, const QString &targetFile) {

    // 创建 QProcess 对象
    QProcess process;

    // 构建命令行参数
    QStringList arguments;
    arguments << "-i" << sourceFile
              << "-s" << "640x480"
              << "-r" << "24"
              << "-b" << "400k"
              << "-vcodec" << "libx264"
              << "-preset" << "ultrafast"
              << "-qp" << "35"
              << "-y" << targetFile;

    process.start("ffmpeg", arguments);

    if (!process.waitForFinished()) {
        qDebug() << "Error:" << process.errorString();
        return;
    }

    // 获取输出信息
    QString output = process.readAllStandardOutput();
    QString errorOutput = process.readAllStandardError();

    // 打印输出信息
    qDebug() << "Output:" << output;
    qDebug() << "Error Output:" << errorOutput;

    qDebug() << "Video compressed successfully.";
}

void ZFYControl::convertToMP4(const QString &inputFile, const QString &outputFile) {
     // 检查输入文件是否存在
    if (!QFile::exists(inputFile)) {
        qCritical() << "Input file does not exist:" << inputFile;
        return;
    }

    QProcess process;
    QString program = "ffmpeg"; // FFmpeg executable
    QStringList arguments;

    // 设置转换参数
    arguments << "-i" << inputFile << "-c:v" << "libx264" << "-preset" << "fast" << outputFile;

    // 启动进程
    process.start(program, arguments);

    // 检查进程是否启动成功
    if (!process.waitForStarted()) {
        qCritical() << "Failed to start FFmpeg process.";
        return;
    }

    // 等待进程完成
    if (!process.waitForFinished()) {
        qCritical() << "FFmpeg process did not finish successfully.";
        return;
    }

    // 检查进程退出状态
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qCritical() << "FFmpeg process exited with errors.";
        QString error = process.readAllStandardError();
        qCritical() << "Error output:" << error;
        return;
    }

    // 输出结果
    QString output = process.readAllStandardOutput();
    qDebug() << "Conversion successful. Output:" << output;
}


//获取可拷贝文件总大小
void ZFYControl::getFilesSizeInFolder(const QString &folderPath, qint64 &totalFileSize)
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
        QFileInfo fileInfo(filePath);
        qint64 fileSize = fileInfo.size();
        totalFileSize += fileSize;
    }

    QStringList dirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &dir, dirs) {
        QString subFolderPath = directory.filePath(dir);
        getFilesSizeInFolder(subFolderPath,totalFileSize);
    }
}

//获取可拷贝文件总大小,文件总数量
void ZFYControl::getFilesSizeFileCountInFolder(const QString &folderPath, qint64 &totalFileSize,qint64 &totalFileCount)
{

    QDir directory(folderPath);
    QStringList filters;
    filters << "*.mpg" << "*.avi" << "*.mpv" << "*.m1v" << "*.m2v" << "*.wmv" << "*.asf" << "*.mov" << "*.mp4"
            << "*.mkv" << "*.nmea" << "*.wav" << "*.wma" << "*.mp3" << "*.mp2" << "*.aac" << "*.3gpp"
            << "*.tif" << "*.gif" << "*.jpg" << "*.pcx" << "*.bmp" << "*.png" << "*.tga" << "*.jp2" << "*.j2k"
            << "*.jpeg" << "*.txt" << "*.log" << "*.dat" << "*.gps";
    QStringList files = directory.entryList(filters, QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    totalFileCount += files.size();
    foreach (const QString &file, files) {
        QString filePath = directory.filePath(file);
        QFileInfo fileInfo(filePath);

        if(breakgather == 1){

             QString absolutePath = fileInfo.absoluteFilePath();
             // 输出绝对路径
             qDebug() << "Absolute path: " << absolutePath;

             int _fpIdx = absolutePath.indexOf("/",11);
             QString _prePath = absolutePath.mid(_fpIdx);


             if(readyfiles.contains(_prePath)){

                 totalFileCount -= 1 ;
                 continue;
             }
        }

        qint64 fileSize = fileInfo.size();
        totalFileSize += fileSize;

    }


    QStringList dirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &dir, dirs) {
        QString subFolderPath = directory.filePath(dir);
        getFilesSizeFileCountInFolder(subFolderPath,totalFileSize,totalFileCount);
    }
}


QString  ZFYControl::getParentPathAsDevNo(QString filename){

     QFile file(filename);
     QFileInfo fileInfo(filename);
     QDir dir = fileInfo.dir();

     // 获取文件所在目录
     QString dirPath = dir.absolutePath();

     // 打印目录
     qDebug() << "The directory of the file is:" << dirPath;

     QStringList parts = dirPath.split('/');
     qDebug() << parts;


     QString lastItem = parts.at(parts.size() - 1);
     qDebug() << "Last item: " << lastItem;


     return  lastItem;

}



void ZFYControl::insert_t_file_data(const QString &sourceDirFather,const QStringList &copiedsuccessfullypath,const QString &devNo,const QString &polNo,int winNum){

    QList<QVariantMap> mapCollection;
    QString MyHostIPV4Address = getHostIPV4Address().toString();

    QStringList  _lsCpyedPath;


    int _isUpload =  Config::getInstance()->Get("wsConfig", "Upload").toInt();
    if (_isUpload != 0){

        MyHostIPV4Address =    Config::getInstance()->Get("runConfig", "localIpAddress").toString();
    }

    for (const FileMapping& file : fileVector) {

        int _fpIdx = file.sourcePath.indexOf("/",11);
        QString _prePath = file.sourcePath.mid(_fpIdx);
        _lsCpyedPath << _prePath;

        const QString &path = file.copyPath;
        const QString &sourpath = file.sourcePath;
        QVariantMap copyfilemap;
        QFileInfo fileInfo(path);
        QFileInfo sourfileInfo(sourpath);

        QString fileName = fileInfo.fileName();
//        qDebug() << "File Name: " << fileName;

        QString fileType = fileInfo.completeSuffix();
        int typenum = getFileCategory(fileType);


        // 获取创建时间
        QDateTime createdTime = sourfileInfo.created();

        // 如果创建时间无效，尝试获取修改时间
        if (!createdTime.isValid()) {
            createdTime = sourfileInfo.lastModified();
        }

        // 如果修改时间也无效，尝试获取访问时间
        if (!createdTime.isValid()) {
            createdTime = sourfileInfo.lastRead();
        }

        // 仍然无效时，尝试从文件名提取日期
        if (!createdTime.isValid()) {
            QDate extractedDate;
            QRegularExpression regex("20\\d{6}"); // 匹配完整日期：yyyymmdd
            QRegularExpressionMatch match = regex.match(fileName);

            if (match.hasMatch()) {
                QString dateString = match.captured(0);
                qDebug() << "Extracted date segment:" << dateString;
                extractedDate = QDate::fromString(dateString, "yyyyMMdd");
            }

            // 如果提取日期有效，更新 createdTime
            if (extractedDate.isValid()) {
                createdTime.setDate(extractedDate);
            } else {
                createdTime = QDateTime::currentDateTime();
                qDebug() << "No valid date segment found in filename, using current date.";
            }
        }

        QString tmpdevNo ;
        if(zfydevicetype == 1 ){

            tmpdevNo =  getParentPathAsDevNo(file.sourcePath);

        }


        // 4. 当前时间
        QDateTime currentTime = QDateTime::currentDateTime();


        // 5. 文件大小（单位：MB）
        qint64 fileSize = fileInfo.size(); // 文件大小以字节为单位
        double fileSizeMB = static_cast<double>(fileSize) / (1024 * 1024); // 转换为MB
        copyfilemap["F_Guid"] = fileName+QString::number(fileSizeMB);
        copyfilemap["F_DeviceNo"] =  zfydevicetype == 0 ? devNo : tmpdevNo; // 2025.1.8
        copyfilemap["F_UserNo"] = polNo;
        copyfilemap["F_WorkStationIp"] = MyHostIPV4Address;
        copyfilemap["F_FileName"] = fileName;
        copyfilemap["F_Type"] = typenum; // Assuming F_Type is always 1
        copyfilemap["F_LocalStorageDriveLetter"] = path;
        copyfilemap["F_ServerStorageDriveLetter"] = path; // 本地路径
        copyfilemap["F_FilePath"] = sourpath; //u盘路径
        copyfilemap["F_ShootTime"] = createdTime;
        copyfilemap["F_UploadTime"] = currentTime;
        copyfilemap["F_FileSize"] = fileSizeMB;
        if(typenum == 1){
        copyfilemap["F_FileTime"] = getVideoDuration(path);
        }else {
        copyfilemap["F_FileTime"] = 1;
        }
        copyfilemap["F_Thumbnail"] = ""; // Default NULL
        copyfilemap["F_IsImportant"] = 0;
        copyfilemap["F_FtpUploadStatus"] = 0;
        copyfilemap["F_IsEncrypt"] = 0;
        copyfilemap["F_IsTranscode"] = 0;
        copyfilemap["F_CompStatus"] = 0;
        copyfilemap["F_FileDataSyncStatus"] = 0;
        copyfilemap["F_Remark"] = fileName; // Assuming F_Remark is the file name
        copyfilemap["F_FileSource"] = 1; // Default 1
        copyfilemap["F_IsDelete"] = 0; // Default 0

        //    qDebug()<<"ZFYControl::insert_t_file_data copyfilem"<<copyfilemap["F_FileTime"].toString();
        mapCollection.append(copyfilemap);
    }

    fileVector.clear(); // 拷贝完成删除一个元素

    if (breakgather == 1){
        QString  _finishedNmae = mountUsbPath + "/finished.ini";
        QFile _fileFinded(_finishedNmae);

        if (_fileFinded.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text )) {

            QTextStream out(&_fileFinded);
            for (const QString &str : _lsCpyedPath) {
                out << str << endl;
            }

            _fileFinded.flush();
            _fileFinded.close();  // 关闭文件
        }
    }
    //    qDebug()<<"ZFYControl::insert_t_file_data insertT_Files_sqlite:"<<sourceFilePathIsErr;
    emit insertT_Files_sqlite(mapCollection,sourceFilePathIsErr,sourceDirFather,winNum);

}
QHostAddress ZFYControl::getHostIPV4Address()
{
    //获取第一个非本地回环的IPv4地址，如果找不到符合条件的地址，则返回本地回环地址
    foreach(const QHostAddress& hostAddress,QNetworkInterface::allAddresses())
        if ( hostAddress != QHostAddress::LocalHost && hostAddress.toIPv4Address() )
            return hostAddress;

    return QHostAddress::LocalHost;
}

int ZFYControl::getFileCategory(const QString& fileType) {

    QString lowerCaseType = fileType.toLower();
    if (lowerCaseType.isEmpty()) {
        return 5; // 其他
    }

    if (lowerCaseType == "mp4" || lowerCaseType == "avi" || lowerCaseType == "mkv" || lowerCaseType == "mov") {
        return 1; // 视频
    } else if (lowerCaseType == "mp3" || lowerCaseType == "wav" || lowerCaseType == "flac") {
        return 2; // 音频
    } else if (lowerCaseType == "jpg" || lowerCaseType == "jpeg" || lowerCaseType == "png") {
        return 3; // 图片
    } else if (lowerCaseType == "txt" || lowerCaseType == "log") {
        return 4; // 文本日志
    } else {
        return 5; // 其他
    }
}
QString ZFYControl::getFileType(const QString& fileType) {
    // 将文件类型转换为小写
    QString lowerCaseType = fileType.toLower();
    if (lowerCaseType.isEmpty()) {
        return "OTHER"; // 其他
    }

    // 根据文件类型归类并返回大写字符串
    if (lowerCaseType == "mp4" || lowerCaseType == "avi" || lowerCaseType == "mkv" || lowerCaseType == "mov") {
        return "VIDEO"; // 视频
    } else if (lowerCaseType == "mp3" || lowerCaseType == "wav" || lowerCaseType == "flac") {
        return "AUDIO"; // 音频
    } else if (lowerCaseType == "jpg" || lowerCaseType == "jpeg" || lowerCaseType == "png") {
        return "IMAGE"; // 图片
    } else if (lowerCaseType == "txt" || lowerCaseType == "log") {
        return "TEXT"; // 文本
    } else {
        return "OTHER"; // 其他
    }
}

void ZFYControl::checkt_filedata(QString sourceFilePath){

    emit checkAndDeleteFile(sourceFilePath,zfynum);
}


void ZFYControl::recTaskDeviceInfo(int windowNum, QString userNo, QString deviceNo, QString userName, QString corpName)
{

         _task_windowNum = windowNum ;
         _task_userNo = userNo ;
         _task_deviceNo = deviceNo ;
         _task_userName = userName ;
         _task_corpName = corpName ;

        zfybeginwork = true;

}


bool ZFYControl::isFolderEmpty(const QString &folderPath) {

    QDir folderDir(folderPath);


    if (!folderDir.exists()) {
        qDebug() << "Folder does not exist:" << folderPath;
        return true; // Assuming non-existing folder is considered empty
    }

    QStringList filters;
    filters << "*.mpg" << "*.avi" << "*.mpv" << "*.m1v" << "*.m2v" << "*.wmv" << "*.asf" << "*.mov" << "*.mp4"
            << "*.mkv" << "*.nmea" << "*.wav" << "*.wma" << "*.mp3" << "*.mp2" << "*.aac" << "*.3gpp"
            << "*.tif" << "*.gif" << "*.jpg" << "*.pcx" << "*.bmp" << "*.png" << "*.tga" << "*.jp2" << "*.j2k"
            << "*.jpeg" << "*.txt" << "*.log" << "*.dat" << "*.gps";

    QStringList files = folderDir.entryList(filters, QDir::Files);

    if (!files.isEmpty()) {
        qDebug() << "Folder is not empty, contains files:" << folderPath;
        return false;
    }

    QStringList subDirs = folderDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &subDir : subDirs) {
        QString subDirPath = folderPath + "/" + subDir;
        if (!isFolderEmpty(subDirPath)) {
            return false;
        }
    }

//    qDebug() << "All subfolders are empty or do not contain specified files in:" << folderPath;
    return true;
}

bool ZFYControl::readConfigFile(const QString &path, zfyConfig &config) {
    QStringList possibleNames = {"PARAM.INI", "param.ini","Param.ini"};
    QStringList possibleGroups = {"ZFY", "ZHY"};

    foreach (const QString &name, possibleNames) {
        QFile file(path + "/" + name);
        if (file.exists()) {

            QSettings setting(file.fileName(), QSettings::IniFormat);

            setting.setIniCodec("utf-8");

            foreach (const QString &group, possibleGroups) {
                if (setting.childGroups().contains(group)) {
                    setting.beginGroup(group);

                    config.devNo = setting.value("DevNo", config.devNo).toString();
                    config.polNo = setting.value("PolNo", config.polNo).toString();

                    //config.department = setting.value("Department", config.department).toString();
                    config.department = setting.value("CorpName", config.department).toString();
                    config.userName = setting.value("UserName", config.userName).toString();

                    config.dev = setting.value("Dev", config.dev).toString();
                    config.pol = setting.value("Pol", config.pol).toString();
                    config.isFlashLight = setting.value("isFlashLight", config.isFlashLight).toString();
                    config.uncorrelated = setting.value("uncorrelated", config.uncorrelated).toString();
                    setting.endGroup();

                    if (config.polNo.isEmpty() && name == "PARAM.INI") {
                        config.polNo = config.devNo;
                    }

                    if (config.devNo.isEmpty() && name != "PARAM.INI") {
                        config.devNo = "000000";
                    }
                    if (config.polNo.isEmpty() && name != "PARAM.INI" ) {
                        config.polNo = "000000";
                    }

                   // qDebug() << "Config file used:" << file.fileName();
                   // qDebug() << "Group used:" << group;
                    return true;
                }
            }
        }
    }
    return false;
}
int  ZFYControl::getVideoDuration(const QString &filePath) {

    QProcess ffprobe;
    ffprobe.start("ffprobe", QStringList() << "-v" << "error" << "-show_entries"
                    << "format=duration" << "-of"
                    << "default=noprint_wrappers=1:nokey=1" << filePath);
    ffprobe.waitForFinished();
    QString output(ffprobe.readAllStandardOutput());

    // 通过 toDouble 获取精准的秒数，随后转换为 int
    double durationInSeconds = output.trimmed().toDouble();
//    qDebug() << "Video duration (in seconds):" << durationInSeconds;

    // 取整返回
    return static_cast<int>(durationInSeconds); // 返回整数部分
}
