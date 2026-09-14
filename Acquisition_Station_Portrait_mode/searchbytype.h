#ifndef SEARCHBYTYPE_H
#define SEARCHBYTYPE_H

#include <QDialog>
#include <qdir.h>
#include<QDebug>
#include<mysqllite.h>
#include<QDesktopServices>
#include<QUrl>
#include<QFileDialog>
#include<QMessageBox>
#include"networkutility.h"
#include <QTableWidgetItem>
namespace Ui {
class SearchByType;
}

class SearchByType : public QDialog
{
    Q_OBJECT

public:
    explicit SearchByType(QString userid,QString roleId,MySqlLite *sqltie,NetworkUtility *net,bool videojur,QWidget *parent = nullptr);
    ~SearchByType();

    QWidget  *_this;


    int currPage;
    int maxRowsToDisplay;


signals:
    void goback();
    //void gotovideo(QString path,int search_type);
    void gotovideo(QVariantMap videomap);
    //0 代表all 1代表video 2代表photo 3代表log
    void get_tb_files_type(int type_jur,QString userid,QString type);
    void sendDelet_tfilebyid(QString workstationIp,QList<QMap<QString,QString>> tfileIds);
    void sendImportant_tfilebyid(QString workstationIp,QList<QMap<QString,QString>> tfileIds);
    void sendRemark_tfilebyid(const QMap<QString, QString>& file);
    void sendAutoUpload_tfilebyid(QString workstationIp,QList<QMap<QString,QString>> tfileIds);


    void sendRename_tfilebyid(const QMap<QString, QString>& file);

    void sendupload_tfilebyid(const QStringList& ids);

    void sendnetWorkEditFile(const QList<QMap<QString,QString>> &fileInfos);
    void get_realdb_files_type( QString roleId,QString usrNO, QString filetype ,QString pernumber,QString pername, QString significance,
                            QString equipnumber,  QString remark , QString  dateEditstart , QString  dateEditend,
                                QString  dateEditstart_create , QString  dateEditend_create,int maxRowsToDisplay ,int page);


private slots:

    void on_pushButton_goback_clicked();

    void on_tableWidget_cellClicked(int row, int column);

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void show_tb_files(QList<FileInfo>);
    void show_realtb_files(QList<FileInfo>);


    void on_btn_Export_clicked();

    void onPlayButtonClicked(const QString &filePath);

    void on_pushButtonsearch_clicked();

    void on_pushButtonclear_clicked();

    void on_pushButtondelete_clicked();

    void on_pushButtonImportant_clicked();

    void onRemarkButtonClicked(int displayedIndex, QString text, const QList<FileInfo>& filteredFiles);

    void on_btn_upload_clicked();

    void onUserItemSelectionChanged();
    void onUserItemCilckChanged(QTableWidgetItem *item);

    void on_pushButtonConvert_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void renameFilePath(QString fid,QString l_path,QString src_file);

    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);



    void on_pushButton_priv_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_3_clicked();

    void onHeaderClicked(int column);

private:
    Ui::SearchByType *ui;

    QFileInfoList GetFileList(QString path,QStringList filters);
    void clearTable();

    MySqlLite *mysql;
    QList<FileInfo> getfile;
    QList<QProcess*> processList;
    int type_jur = -1;
    QString roleId;
    QString userId;
    bool videojur;
    NetworkUtility *mynet;


    bool isUploadEnabled = false;
    QList<FileInfo> allFilteredFiles;
    int alldisplayedRows = 0;

   QString  getFileType(const QString& fileType);






private:
    bool matchesFilter(const FileInfo &row, const QString &filetype, const QString &pernumber,
                       const QString &equipnumber, const QString &significance, const QString &remark);
    QList<QTableWidgetItem*> createRowItems(const FileInfo &row);


};


class HsConvetWorker :public QObject{

    Q_OBJECT

public :
    QString ffmpegpath ;
    QString scrfilepath ;
    QString dstfilepath ;

    HsConvetWorker(QString ffmpeg,QString scrfile, QString dstfile){
        qDebug() << " *****************  HsConvetWorker()  ***************** " ;
        ffmpegpath = ffmpeg;
        scrfilepath = scrfile;
        dstfilepath = dstfile;
    }

    ~HsConvetWorker(){

        qDebug() << " --------------------  ~HsConvetWorker()  -------------------- " ;
    }

signals:
   void finished();

public slots:

    void  dowork(){
        int _i = 0;
        while(true){

             QThread::sleep(2);
            _i++;

            if(_i > 10){
                break;
            }
        }

        emit finished();

    }// end dowork


    void  doconvert(){

        QProcess proc;

        proc.start(ffmpegpath,QStringList() << "-i" << scrfilepath << dstfilepath << "-y");
        proc.waitForFinished(-1);

        QByteArray output = proc.readAllStandardOutput();
        QByteArray errput = proc.readAllStandardError();

        qDebug() << "readAllStandardOutput:\n" <<  output.data();
        qDebug() << "readAllStandardError:\n" <<  errput.data();


        emit finished();

    }// end doconvert


};




#endif // SEARCHBYTYPE_H
