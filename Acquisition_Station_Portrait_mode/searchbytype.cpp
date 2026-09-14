#include "searchbytype.h"
#include "ui_searchbytype.h"
#include <QCheckBox>
#include<QDateTime>
#include <QHBoxLayout>
#include <QMediaPlayer>
#include <QProgressDialog>
#include <qinputdialog.h>
#include <QFont>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTimer>

#include "videocompressor.h" 


namespace {

void applyPortraitSearchLayout(Ui::SearchByType *ui, QWidget *page)
{
    const int width = page->width();
    const int height = page->height();
    const int margin = 32;
    const int gap = 24;
    const int columnWidth = (width - margin * 2 - gap) / 2;
    const int labelWidth = 150;
    const int fieldWidth = columnWidth - labelWidth - 10;
    const int fieldHeight = 48;
    const QFont labelFont("Sans Serif", 14, QFont::DemiBold);
    const QFont controlFont("Sans Serif", 13);
    const QFont buttonFont("Sans Serif", 13, QFont::DemiBold);

    auto placeField = [&](QLabel *label, QWidget *field, int column, int y) {
        const int x = margin + column * (columnWidth + gap);
        label->setGeometry(x, y, labelWidth, fieldHeight);
        field->setGeometry(x + labelWidth + 10, y, fieldWidth, fieldHeight);
        label->setFont(labelFont);
        field->setFont(controlFont);
    };

    ui->label->setGeometry(margin, 24, 340, 82);
    ui->label->setText(QString::fromUtf8("数据查询"));
    ui->label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    ui->label->setStyleSheet("QLabel { color: white; font: 24pt 'Sans Serif'; font-weight: 600; }");
    ui->pushButton_goback->setGeometry(width - 156, 26, 124, 66);
    ui->pushButton_goback->setFont(QFont("Sans Serif", 15, QFont::DemiBold));

    placeField(ui->labelfiletype, ui->filetypecomboBox, 0, 90);
    placeField(ui->labelpernumber, ui->lineEditpernum, 1, 90);
    placeField(ui->labelpername, ui->lineEditpername, 0, 160);
    placeField(ui->labelequipnumber, ui->lineEditshebnum, 1, 160);
    placeField(ui->labelNum, ui->spinBoxNum, 0, 230);
    placeField(ui->labelsignificance, ui->significancecomboBox, 1, 230);

    ui->label_3->setGeometry(margin, 300, labelWidth, fieldHeight);
    ui->label_3->setFont(labelFont);
    ui->lineEditrem->setGeometry(margin + labelWidth + 10, 300, 360, fieldHeight);
    ui->lineEditrem->setFont(controlFont);
    ui->pushButtonclear->setGeometry(width - 286, 300, 118, fieldHeight);
    ui->pushButton->setGeometry(width - 150, 300, 118, fieldHeight);
    ui->pushButtonclear->setFont(buttonFont);
    ui->pushButton->setFont(buttonFont);

    ui->label_6->setGeometry(margin, 370, labelWidth, fieldHeight);
    ui->label_6->setFont(labelFont);
    ui->dateEditstart_create->setGeometry(margin + labelWidth + 10, 370, 250, fieldHeight);
    ui->label_7->setGeometry(margin + labelWidth + 275, 370, 28, fieldHeight);
    ui->dateEditend_create->setGeometry(margin + labelWidth + 310, 370, 250, fieldHeight);

    ui->label_4->setGeometry(margin, 440, labelWidth, fieldHeight);
    ui->label_4->setFont(labelFont);
    ui->dateEditstart->setGeometry(margin + labelWidth + 10, 440, 250, fieldHeight);
    ui->label_5->setGeometry(margin + labelWidth + 275, 440, 28, fieldHeight);
    ui->dateEditend->setGeometry(margin + labelWidth + 310, 440, 250, fieldHeight);

    const int toolbarY = 510;
    const int toolbarGap = 10;
    const int toolbarWidth = (width - margin * 2 - toolbarGap * 6) / 7;
    QPushButton *toolbarButtons[] = {
        ui->pushButton_priv, ui->pushButton_next, ui->pushButtonConvert,
        ui->btn_upload, ui->pushButtonImportant, ui->pushButtondelete,
        ui->btn_Export
    };
    for (int i = 0; i < 7; ++i) {
        toolbarButtons[i]->setGeometry(margin + i * (toolbarWidth + toolbarGap), toolbarY, toolbarWidth, 58);
        toolbarButtons[i]->setFont(buttonFont);
    }

    ui->tableWidget->setGeometry(margin/2, 580, width - margin, height - 805);
    ui->tableWidget->setFont(QFont("Sans Serif", 11));
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(48);
    ui->label_2->setGeometry(width / 2 - 45, height - 92, 90, 52);

    ui->pushButtonsearch->hide();
    ui->pushButton_2->hide();
    ui->pushButton_3->hide();
}

}

SearchByType::SearchByType(QString userId,QString roleId,MySqlLite *sqltie,NetworkUtility *net,bool videojur,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchByType)
{
    ui->setupUi(this);

    currPage = 1;

    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(this->width(),this->height());
    showFullScreen();

     QLabel *mytime = parent->findChild<QLabel*>("timeLabel");
     QLabel *mydata = parent->findChild<QLabel*>("dateTimeLabel");
     if (mytime) {
         mytime->hide();
//         qDebug()<<"time"<<mytime->text();
     }
     if (mydata) {
         mydata->hide();
//         qDebug()<<"time"<<mytime->text();
     }
    //path = "/mnt/usb/zfy1";
    //filters << QString("*.mp4") << QString("*.MP4");
     ui->spinBoxNum->setRange(0, 1000); // 设置最小值为 0，最大值为 1000
     ui->spinBoxNum->setValue(30);
     bool conversionOk;
     int uploadNum =  Config::getInstance()->Get("wsConfig", "Upload").toInt(&conversionOk);

     // 定义布尔变量


     // 根据 uploadNum 的值赋值给布尔变量
     if (!conversionOk || uploadNum != 0) {
         // 如果转换失败或 uploadNum 不是 0，设置为 false
         isUploadEnabled = false;
     } else {
         // 如果 uploadNum 是 0，设置为 true
         isUploadEnabled = true;
     }

    ui->btn_upload->setEnabled(isUploadEnabled);

    mysql = sqltie;
    mynet = net;
    //mysql->openmysql();
    this->userId = userId;
    this->roleId = roleId;
    this->videojur =videojur;
    connect(this,SIGNAL(get_tb_files_type(int,QString,QString)),mysql,SLOT(get_tb_files_type(int,QString,QString)));
    connect(mysql,SIGNAL(gettb_files_success(QList<FileInfo>)),this,SLOT(show_tb_files(QList<FileInfo>)));

    connect(this,&SearchByType::get_realdb_files_type,mysql, &MySqlLite::get_realdb_files_type);
    connect(mysql,&MySqlLite::realdb_files_success,this, &SearchByType::show_realtb_files);

    connect(this,&SearchByType::sendDelet_tfilebyid,mysql,&MySqlLite::slotDelet_tfilebyid);
    connect(mysql,&MySqlLite::MSQFile_ClientDelete_TONET,mynet,&NetworkUtility::netWorkFileClientDelete);
    connect(this,&SearchByType::sendImportant_tfilebyid,mysql,&MySqlLite::slotImportant_tfilebyid);
    connect(this,&SearchByType::sendRemark_tfilebyid,mysql,&MySqlLite::slotRemark_tfilebyid);

    connect(this,&SearchByType::sendAutoUpload_tfilebyid,mysql,&MySqlLite::slotAutoUpload_tfilebyid);

    connect(this,&SearchByType::sendnetWorkEditFile,mynet,&NetworkUtility::netWorkFileClientEdit);

    connect(this,&SearchByType::sendRename_tfilebyid,mysql,&MySqlLite::slotRename_tfilebyid);


//    connect(this,SIGNAL(get_tb_files_type(int,QString,int)),mysql,SLOT(get_tb_files_typehs(int,QString,int)));
//    connect(mysql,SIGNAL(gettb_files_success(QList<FileInfo>)),this,SLOT(show_tb_fileshs(QList<FileInfo>)));

    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setPalette(QPalette(QColor(234,239,247)));

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setColumnWidth(0, 90);
    ui->tableWidget->setColumnWidth(1, 100);
    ui->tableWidget->setColumnWidth(2, 100);

    ui->tableWidget->setColumnWidth(3, 70);
    ui->tableWidget->setColumnWidth(4, 160);
    ui->tableWidget->setColumnWidth(5, 160);

    ui->tableWidget->setColumnWidth(6, 95); // 新增列用于放置播放按钮
    ui->tableWidget->setColumnWidth(7, 110);
    ui->tableWidget->setColumnWidth(8, 95);
    ui->tableWidget->setColumnWidth(9, 110);

    ui->tableWidget->setColumnWidth(10, 95);
    ui->tableWidget->setColumnWidth(11, 400); // 备注
    ui->tableWidget->setColumnWidth(12, 50);


   QDate currentDate = QDate::currentDate();
   // 获取前一周的日期
   QDate lastWeek = currentDate.addDays(-90);
   QDate lastWeek_create = currentDate.addDays(-365);

   //ui->dateEditstart->setDate(QDate::currentDate());
   ui->dateEditstart->setDate(lastWeek);
   ui->dateEditstart_create->setDate(lastWeek_create);

   ui->dateEditstart->setCalendarPopup(true);
   ui->dateEditstart_create->setCalendarPopup(true);

   ui->dateEditend->setCalendarPopup(true);
   ui->dateEditend_create->setCalendarPopup(true);

   ui->dateEditstart->setStyleSheet("QDateEdit::drop-down { image: url(:/image/drop_rl.png) ;  }");
   ui->dateEditend->setStyleSheet("QDateEdit::drop-down { image: url(:/image/drop_rl.png) ;  }");
   ui->dateEditstart_create->setStyleSheet("QDateEdit::drop-down { image: url(:/image/drop_rl.png) ;  }");
   ui->dateEditend_create->setStyleSheet("QDateEdit::drop-down { image: url(:/image/drop_rl.png) ;  }");

    connect(ui->tableWidget->horizontalHeader(),&QHeaderView::sectionClicked,this,&SearchByType::onHeaderClicked);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->dateEditend->setDate(QDate::currentDate());
    ui->dateEditend_create->setDate(QDate::currentDate());


    ui->tableWidget->setStyleSheet( "QScrollBar:vertical { width: 40px; }");



     ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
     ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
     ui->tableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
    QObject::connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &SearchByType::onUserItemSelectionChanged);
    //QObject::connect(ui->tableWidget,&QTableWidget::itemClicked,this,&SearchByType::onUserItemCilckChanged);
    QObject::connect(ui->tableWidget,&QTableWidget::itemDoubleClicked,this,&SearchByType::onUserItemCilckChanged);


    ui->pushButtonConvert->setVisible(true);
    ui->pushButtonsearch->setHidden(true);
    ui->pushButton_2->setHidden(true);
    ui->pushButton_3->setHidden(true);

    applyPortraitSearchLayout(ui, this);


    on_pushButton_clicked();

}

SearchByType::~SearchByType()
{
    delete ui;
}

void SearchByType::clearTable(){

    ui->tableWidget->setRowCount(0);
}



void SearchByType::on_pushButton_goback_clicked()
{

   this->close();
}



void SearchByType::on_tableWidget_cellClicked(int row, int column)
{
    qDebug()<<"row:"<<row<<" column: "<<column;
    //qDebug()<<"clicked:"<<ui->tableWidget->item(row,column)->text();
    //qDebug()<<file_list.at(row).filePath();

}

void SearchByType::on_tableWidget_cellDoubleClicked(int row, int column)
{
    if(column == 0){
        qDebug()<<"double clicked"<<"getfile.at(row).filepath()";

        if(!getfile.isEmpty()){
            if(getfile.at(row).file_est =="MP4" || getfile.at(row).file_est =="m4a"){
                qDebug()<<"double click video";
                QVariantMap videomap;
                videomap["searchtype"] = 2;
                videomap["userid"] = "userid";
                videomap["driverid"] = getfile.at(row).file_driverid;
                videomap["videopath"] = getfile.at(row).filepath;
                videomap["shortpath"] = getfile.at(row).file_shortpath;
                videomap["oldfilename"] = getfile.at(row).file_name;
                videomap["videojur"] = videojur;
                //emit gotovideo(getfile.at(row).filepath,2);
                emit gotovideo(videomap);
            }else {
                qDebug()<<"getfile.at(row).filepath"<<getfile.at(row).filepath;
                QDesktopServices::openUrl(QUrl(getfile.at(row).filepath));

            }

        }

    }

}

void SearchByType::show_tb_files(QList<FileInfo> file)
{
//    QList<QProcess*> *processList = new QList<QProcess*>(); // 使用指针来防止销毁后访问问题
//    processList.clear();
//    int rowcount = ui->tableWidget->rowCount();
    getfile = file;
//    for (int i = file.count() - 1; i >= 0; i--) {
//        ui->tableWidget->insertRow(rowcount);
//        ui->tableWidget->setItem(rowcount,0,new QTableWidgetItem(file.at(i).F_Type));
//        ui->tableWidget->setItem(rowcount,1,new QTableWidgetItem(file.at(i).F_UserNo));
//        ui->tableWidget->setItem(rowcount,2,new QTableWidgetItem(file.at(i).F_DeviceNo));
//        ui->tableWidget->setItem(rowcount,3,new QTableWidgetItem(file.at(i).F_UserName));
//        ui->tableWidget->setItem(rowcount,4,new QTableWidgetItem(file.at(i).F_ShootTime));
//        ui->tableWidget->setItem(rowcount,5,new QTableWidgetItem(file.at(i).F_UploadTime));
//        ui->tableWidget->setItem(rowcount,7,new QTableWidgetItem(file.at(i).F_IsImportant));
////        ui->tableWidget->setItem(rowcount,8,new QTableWidgetItem(file.at(i).F_Remark));
//        QTableWidgetItem *check = new QTableWidgetItem;
//        check->setCheckState(Qt::Unchecked);

//        ui->tableWidget->setItem(rowcount, 9, check);

//        QPushButton *playButton = new QPushButton("播放");
//        playButton->setStyleSheet("background-color: #2196F3; color: white;");
//        ui->tableWidget->setCellWidget(rowcount, 6, playButton);

//        connect(playButton, &QPushButton::clicked, this, [this, i]() {
//            onPlayButtonClicked(i);
//        });

//        QPushButton *remarkButton = new QPushButton("备注");
//        remarkButton->setStyleSheet("background-color: #2196F3; color: white;");
//        ui->tableWidget->setCellWidget(rowcount, 8, remarkButton);

//        connect(remarkButton, &QPushButton::clicked, this, [this, i]() {
//            bool ok;
//            // 弹出输入对话框，让用户输入内容
//            QString text = QInputDialog::getText(this, tr("输入备注"),
//                                                  tr("备注："), QLineEdit::Normal,
//                                                  "", &ok);
//            // 如果用户点击 OK，处理输入内容
//            if (ok && !text.isEmpty()) {
//                // 这里可以处理用户输入的文本
//                onRemarkButtonClicked(i, text);
//            }
//        });

//        rowcount++;
//    }

    on_pushButtonsearch_clicked();
}


void SearchByType::show_realtb_files(QList<FileInfo> file)
{

    getfile = file;

    ui->label_2->setText("当前页:" + QString::number(currPage));
    if(currPage == 1){
        ui->pushButton_priv->setVisible(false);
    }

    // 获取允许显示的最大行数
    int maxRowsToDisplay = ui->spinBoxNum->value(); // 从 QSpinBox 获取值

    if(file.isEmpty() || file.size() < (maxRowsToDisplay -1)){

        ui->pushButton_next->setVisible(false);
    }else{
        ui->pushButton_next->setVisible(true);
    }


//    int maxRowsToDisplay = 10;
    int displayedRows = 0; // 计数已显示的行

//    QList<FileInfo> filteredFiles; // 用于存储过滤后的数据
    QList<FileInfo> filteredFiles;

    // 遍历文件列表
    for (const auto &row : getfile) {

        // 检查是否已达到最大显示行数
        if (displayedRows >= maxRowsToDisplay) {
            break; // 如果已达到最大显示行数，则终止循环
        }

        // 将符合条件的数据添加到 filteredFiles 列表
        filteredFiles.append(row);


        // 获取当前行数
        int rowcount = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(rowcount);

        // 插入数据到表格
        ui->tableWidget->setItem(rowcount, 0, new QTableWidgetItem(row.F_Type));              // 列0
        ui->tableWidget->setItem(rowcount, 1, new QTableWidgetItem(row.F_UserNo));            // 列1
        ui->tableWidget->setItem(rowcount, 2, new QTableWidgetItem(row.F_DeviceNo));           // 列2
        ui->tableWidget->setItem(rowcount, 3,new QTableWidgetItem(row.F_UserName));
        ui->tableWidget->setItem(rowcount, 4, new QTableWidgetItem(row.F_ShootTime));         // 列4
        ui->tableWidget->setItem(rowcount, 5, new QTableWidgetItem(row.F_UploadTime));        // 列5
        ui->tableWidget->setItem(rowcount, 7, new QTableWidgetItem(row.F_MarkId));
        ui->tableWidget->setItem(rowcount, 9, new QTableWidgetItem(row.F_IsImportant));
        ui->tableWidget->setItem(rowcount, 11, new QTableWidgetItem(row.F_Remark));


        QTableWidgetItem *check = new QTableWidgetItem;
        check->setCheckState(Qt::Unchecked);
        ui->tableWidget->setItem(rowcount, 12, check); // 列9

        // 添加播放按钮
        QPushButton *playButton = new QPushButton("播放");
        playButton->setStyleSheet("background-color: #2196F3; color: white;");
        ui->tableWidget->setCellWidget(rowcount, 6, playButton); // 播放按钮列

        // 连接按钮的点击事件
        connect(playButton, &QPushButton::clicked, this, [this, displayedRows, filteredFiles]() {
            const FileInfo &file = filteredFiles.at(displayedRows);
            const QString localPath = file.F_LocalStorageDriveLetter;
            onPlayButtonClicked(QFileInfo(localPath).isFile() ? localPath : file.filepath);
        });


        // 修改文件名按钮
        QPushButton *renameButton = new QPushButton("修改文件名");
        renameButton->setStyleSheet("background-color: #2196F3; color: white;");
        ui->tableWidget->setCellWidget(rowcount, 8, renameButton); // 修改文件名

        // 修改文件名按钮的点击事件
        connect(renameButton, &QPushButton::clicked, this, [this, displayedRows, filteredFiles]() {
            //onPlayButtonClicked(filteredFiles[displayedRows].filepath); // 获取当前过滤后的文件路径

            QString fid = filteredFiles.at(displayedRows).file_uuid;
            QString l_path = filteredFiles.at(displayedRows).F_LocalStorageDriveLetter;

            renameFilePath(fid,l_path, filteredFiles[displayedRows].filepath);

        });



        QPushButton *remarkButton = new QPushButton("备注");
        remarkButton->setStyleSheet("background-color: #2196F3; color: white;");
        ui->tableWidget->setCellWidget(rowcount, 10, remarkButton);

        connect(remarkButton, &QPushButton::clicked, this, [this, displayedRows, filteredFiles]() {
            bool ok;
            // 弹出输入对话框，让用户输入内容
            QString text = QInputDialog::getText(this, tr("输入备注"),
                                                  tr("备注："), QLineEdit::Normal,
                                                  "", &ok);
            // 如果用户点击 OK，处理输入内容
            if (ok && !text.isEmpty()) {
                // 这里可以处理用户输入的文本
                onRemarkButtonClicked(displayedRows, text, filteredFiles); // 传递过滤后的索引
            }
        });
        displayedRows++; // 增加已显示行数计数
    }
    alldisplayedRows = displayedRows;
    allFilteredFiles = filteredFiles;



}


QString SearchByType::getFileType(const QString &fileType)
{

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

}//



/*
 * play
 */
void SearchByType::onPlayButtonClicked(const QString &filePath) {
    QString playbackPath = filePath;
    QFileInfo sourceFile(filePath);

    // Query results can remain after a retention task has removed the local
    // copy.  Starting the desktop player with such a path makes it exit
    // immediately, which looks like the query page has crashed.
    if (!sourceFile.exists() || !sourceFile.isFile()) {
        QMessageBox::warning(this, "无法播放",
                             "本机没有找到该视频文件，无法播放。\n"
                             "文件可能已被清理，或该记录仅来自服务器。\n\n"
                             "请先下载视频到本机后再播放。");
        return;
    }

    if (sourceFile.suffix().compare("avi", Qt::CaseInsensitive) == 0) {
        const QString convertedPath = filePath + ".mp4";
        QFileInfo convertedFile(convertedPath);
        if (!convertedFile.exists() || !convertedFile.isFile()) {
            QMessageBox::information(this, "格式不支持",
                                     "AVI 视频请先使用“格式转换”，转换完成后再播放。");
            return;
        }
        playbackPath = convertedPath;
    }

    const QString playerProgram = QStandardPaths::findExecutable("deepin-movie");
    if (playerProgram.isEmpty()) {
        QMessageBox::warning(this, "无法播放",
                             "未找到系统视频播放器 Deepin Movie。\n"
                             "请安装或恢复系统默认视频播放器后再试。");
        return;
    }

    QProcess *currentProcess = new QProcess(this);
    // The global /usr/local SQLite library lacks the metadata symbols needed
    // by Deepin Movie's Qt SQLite plugin.  Use the compatible system library
    // only for the player process; the acquisition program is unaffected.
    QProcessEnvironment playerEnvironment = QProcessEnvironment::systemEnvironment();
    const QString systemLibraryPath = "/usr/lib/x86_64-linux-gnu";
    const QString inheritedLibraryPath = playerEnvironment.value("LD_LIBRARY_PATH");
    playerEnvironment.insert("LD_LIBRARY_PATH",
                             inheritedLibraryPath.isEmpty()
                             ? systemLibraryPath
                             : systemLibraryPath + ":" + inheritedLibraryPath);
    currentProcess->setProcessEnvironment(playerEnvironment);
    connect(currentProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, currentProcess]
            (int exitCode, QProcess::ExitStatus) {
                qDebug() << "Video player exited:" << exitCode;
                currentProcess->deleteLater();
            });
    connect(currentProcess, &QProcess::errorOccurred, this,
            [this, currentProcess](QProcess::ProcessError) {
                QMessageBox::warning(this, "无法播放",
                                     "系统视频播放器启动失败，请检查播放器是否可用。");
                currentProcess->deleteLater();
            });

    currentProcess->start(playerProgram, QStringList() << playbackPath);
    processList.append(currentProcess);
    qDebug() << "-----------deepin-movie filePath: -----------------" << playbackPath;
}

void SearchByType::on_btn_Export_clicked()
{

    /*获取选中的列表里的所有条目*/
   QList<QTableWidgetItem*> list = ui->tableWidget->selectedItems();

   QSet<int> _selectedRows;

   for (QTableWidgetItem* item : list) {
       int row = item->row();
       _selectedRows.insert(row);
   }


   for (int _row : _selectedRows) {

       qDebug() << "Selected row:" << _row;
       QTableWidgetItem *_tmpRow = ui->tableWidget->item(_row,11);
       int _tmpChk = _tmpRow->checkState();
       Q_UNUSED(_tmpChk);

       QWidget* pWidget = 0;
       pWidget = ui->tableWidget->cellWidget(_row,11);

       QCheckBox *pTmpCheckBox = static_cast<QCheckBox *>(pWidget);
       if (pTmpCheckBox == nullptr)
       {
           continue;
       }

   }

   qDebug() << list.count() << " \r\n";


    QString defaultPath = "/media/";

    QString output_path = QFileDialog::getExistingDirectory(this,tr("select path"),defaultPath);
    qDebug()<<"output_path"<<output_path;
    qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();

    if(output_path.length() >0){

        QMessageBox messageBox(QMessageBox::Information, "正在下载", "请耐心等待！", QMessageBox::Ok, this);
        messageBox.resize(500,500);
        messageBox.show();


        // QProgressDialog progressDialog(this);
        // progressDialog.setWindowTitle(tr("Progress"));
        // progressDialog.setLabelText(tr("Performing operation. Please wait..."));
        // progressDialog.setMinimumDuration(2000);  // 设置最小显示时间（毫秒）
        // progressDialog.setModal(true);           // 设置为模态对话框
        // progressDialog.setCancelButtonText(tr("&Cancel"));
        // progressDialog.setRange(0, 100);


        // check download;
        int i = 0;
        int l_counts = 0;

        bool export_result = false;
        for (int i=0;i<ui->tableWidget->rowCount();i++) {
            //qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();

            if(ui->tableWidget->item(i,12)->checkState() == Qt::Checked){
//                qDebug()<<"true i"<<getfile.at(i).filepath;

//                QString out_file = output_path + QDir::separator() + getfile.at(i).file_name;
//                qDebug()<<"out_file"<<out_file;
//                QFile::setPermissions(out_file, QFile::WriteOwner);
//                export_result = QFile::copy(getfile.at(i).filepath, out_file);
//                qDebug()<<"export_result:"<<export_result;
//                qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();

               // i = i + 1;
                l_counts = l_counts + 1;

                qDebug()<<"正在下载 true i : "<<allFilteredFiles.at(i).filepath;

                QString out_file = output_path + QDir::separator() + allFilteredFiles.at(i).file_name;
                qDebug()<<"正在下载 out_file: "<<out_file;
                QFile::setPermissions(out_file, QFile::WriteOwner | QFile::WriteOther | QFile::ExeOwner | QFile::WriteGroup );
                export_result = QFile::copy(allFilteredFiles.at(i).filepath, out_file);
                qDebug()<<"正在下载 export_result: " << export_result;
                qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();

                if(export_result){
                   // i = i -1;
                    l_counts = l_counts -  1;
                }


            }




        } // end for

        if(l_counts == 0){
            QMessageBox::warning(NULL, "下载", tr("下载完成!"), QMessageBox::Ok );
        }else {
            QMessageBox::warning(NULL, "下载", tr("下载失败!"), QMessageBox::Ok );
        }




    }

}



void SearchByType::on_pushButtonsearch_clicked()
{
    processList.clear();
    allFilteredFiles.clear();

    QString selectedfiletype = ui->filetypecomboBox->currentText();
    QString selectedpernumber = ui->lineEditpernum->text();
    QString selectedpername = ui->lineEditpername->text();
    QString selectedsignificance = ui->significancecomboBox->currentText();
    QString selectedequipnumber = ui->lineEditshebnum->text();
    QString selectedRemark = ui->lineEditrem->text();
    QDate startDate = ui->dateEditstart->date();
    QDate endDate = ui->dateEditend->date();

    ui->tableWidget->setRowCount(0); // 清空表格

    // 获取允许显示的最大行数
    int maxRowsToDisplay = ui->spinBoxNum->value(); // 从 QSpinBox 获取值
//    int maxRowsToDisplay = 10;
    int displayedRows = 0; // 计数已显示的行

//    QList<FileInfo> filteredFiles; // 用于存储过滤后的数据
    QList<FileInfo> filteredFiles;

    // 遍历文件列表
    for (const auto &row : getfile) {

        // 过滤逻辑
        if (!matchesFilter(row, selectedfiletype, selectedpernumber, selectedequipnumber, selectedsignificance, selectedRemark)) {
            continue;
        }

        // 日期过滤逻辑
        if (!row.F_UploadTime.isEmpty()) {
            QString uploadTime = row.F_UploadTime.mid(0, 10);
            QDate targetDate = QDate::fromString(uploadTime, "yyyy-MM-dd");
            if (startDate.isValid() && endDate.isValid() && (targetDate < startDate || targetDate > endDate)) {
                continue;
            }
        }

        // 检查是否已达到最大显示行数
        if (displayedRows >= maxRowsToDisplay) {
            break; // 如果已达到最大显示行数，则终止循环
        }

        // 将符合条件的数据添加到 filteredFiles 列表
        filteredFiles.append(row);


        // 获取当前行数
        int rowcount = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(rowcount);

        // 插入数据到表格
        ui->tableWidget->setItem(rowcount, 0, new QTableWidgetItem(row.F_Type));              // 列0
        ui->tableWidget->setItem(rowcount, 1, new QTableWidgetItem(row.F_UserNo));            // 列1
        ui->tableWidget->setItem(rowcount, 2, new QTableWidgetItem(row.F_DeviceNo));           // 列2
        ui->tableWidget->setItem(rowcount, 3,new QTableWidgetItem(row.F_UserName));
        ui->tableWidget->setItem(rowcount, 4, new QTableWidgetItem(row.F_ShootTime));         // 列4
        ui->tableWidget->setItem(rowcount, 5, new QTableWidgetItem(row.F_UploadTime));        // 列5
        ui->tableWidget->setItem(rowcount, 7, new QTableWidgetItem(row.F_MarkId));
        ui->tableWidget->setItem(rowcount, 8, new QTableWidgetItem(row.F_IsImportant));
        ui->tableWidget->setItem(rowcount, 10, new QTableWidgetItem(row.F_Remark));


        QTableWidgetItem *check = new QTableWidgetItem;
        check->setCheckState(Qt::Unchecked);
        ui->tableWidget->setItem(rowcount, 11, check); // 列9

        // 添加播放按钮
        QPushButton *playButton = new QPushButton("播放");
        playButton->setStyleSheet("background-color: #2196F3; color: white;");
        ui->tableWidget->setCellWidget(rowcount, 6, playButton); // 播放按钮列

        // 连接按钮的点击事件
        connect(playButton, &QPushButton::clicked, this, [this, displayedRows, filteredFiles]() {
            const FileInfo &file = filteredFiles.at(displayedRows);
            const QString localPath = file.F_LocalStorageDriveLetter;
            onPlayButtonClicked(QFileInfo(localPath).isFile() ? localPath : file.filepath);
        });

        QPushButton *remarkButton = new QPushButton("备注");
        remarkButton->setStyleSheet("background-color: #2196F3; color: white;");
        ui->tableWidget->setCellWidget(rowcount, 9, remarkButton);

        connect(remarkButton, &QPushButton::clicked, this, [this, displayedRows, filteredFiles]() {
            bool ok;
            // 弹出输入对话框，让用户输入内容
            QString text = QInputDialog::getText(this, tr("输入备注"),
                                                  tr("备注："), QLineEdit::Normal,
                                                  "", &ok);
            // 如果用户点击 OK，处理输入内容
            if (ok && !text.isEmpty()) {
                // 这里可以处理用户输入的文本
                onRemarkButtonClicked(displayedRows, text, filteredFiles); // 传递过滤后的索引
            }
        });
        displayedRows++; // 增加已显示行数计数
    }
    alldisplayedRows = displayedRows;
    allFilteredFiles = filteredFiles;
}

bool SearchByType::matchesFilter(const FileInfo &row, const QString &filetype, const QString &pernumber,
                                 const QString &equipnumber, const QString &significance, const QString &remark) {
    if (!filetype.isEmpty() && filetype != "全部" && row.F_Type != filetype) {
        return false;
    }
    if (!pernumber.isEmpty() && row.F_UserNo != pernumber) {
        return false;
    }
    if (!equipnumber.isEmpty() && row.F_DeviceNo != equipnumber) {
        return false;
    }
    if (!significance.isEmpty() && significance != "全部" && row.F_MarkId != significance) {
        return false;
    }
    if (!remark.isEmpty() && row.F_Remark != remark) {
        return false;
    }
    return true;
}

QList<QTableWidgetItem*> SearchByType::createRowItems(const FileInfo &row) {
    QList<QTableWidgetItem*> items;
    items.append(new QTableWidgetItem(row.F_Type));
    items.append(new QTableWidgetItem(row.F_UserNo));
    items.append(new QTableWidgetItem(row.F_DeviceNo));
    items.append(new QTableWidgetItem(row.F_ShootTime));
    items.append(new QTableWidgetItem(row.F_UploadTime));
    items.append(new QTableWidgetItem(row.F_IsImportant));
    items.append(new QTableWidgetItem(row.F_Remark));

    QTableWidgetItem *check = new QTableWidgetItem;
    check->setCheckState(Qt::Unchecked);
    items.append(check);

    return items;
}



void SearchByType::on_pushButtonclear_clicked()
{
    ui->filetypecomboBox->setCurrentIndex(0);
    ui->significancecomboBox->setCurrentIndex(0);

    // 获取当前日期
        QDate currentDate = QDate::currentDate();
     // 获取前一周的日期
     QDate lastWeek = currentDate.addDays(-7);

    //ui->dateEditstart->setDate(QDate::currentDate());
    ui->dateEditstart->setDate(lastWeek);
    ui->dateEditstart_create->setDate(lastWeek);

    ui->dateEditend->setDate(QDate::currentDate());
    ui->dateEditend_create->setDate(QDate::currentDate());

    ui->lineEditrem->clear();
    ui->lineEditpernum->clear();
    ui->lineEditpername->clear();
    ui->lineEditshebnum->clear();

   on_pushButton_clicked();

}

void SearchByType::on_pushButtondelete_clicked()
{


//        bool export_result = false;
        QStringList tfileId ;
        QList<QMap<QString,QString>> fileInfos;
        QString workstationIp = "";
        for (int i=0;i<ui->tableWidget->rowCount();i++) {
            //qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();
            if(ui->tableWidget->item(i,12)->checkState() == Qt::Checked){
                QMap<QString,QString> file;
//                qDebug()<<"true i"<<getfile.at(i).filepath;
//                qDebug()<<"getfile.at(i).file_uuid"<<getfile.at(i).file_uuid;
//                file["tfile_id"] = getfile.at(i).file_uuid;
//                file["F_UserNo"] = getfile.at(i).F_UserNo;
//                file["F_Guid"] = getfile.at(i).F_Guid;
////                tfileId.append(getfile.at(i).file_uuid);
//                if(workstationIp.isEmpty()){
//                workstationIp = getfile.at(0).F_WorkStationIp;
//                }



                file["tfile_id"] = allFilteredFiles.at(i).file_uuid;
                file["F_UserNo"] = allFilteredFiles.at(i).F_UserNo;
                file["F_Guid"] = allFilteredFiles.at(i).F_Guid;


                file["F_LocalStorageDriveLetter"] = allFilteredFiles.at(i).F_LocalStorageDriveLetter;
                file["signimp"] = "0";

//                tfileId.append(getfile.at(i).file_uuid);
                if(workstationIp.isEmpty()){
                    workstationIp = allFilteredFiles.at(0).F_WorkStationIp;
                }


                // F_IsImportant > 0
                int _tmpImp =  (allFilteredFiles.at(i).F_IMP_VALUES).toInt();
                int _tmpMarkId =  (allFilteredFiles.at(i).F_MARKID_VALUES).toInt();

                for (int j = 0; j < getfile.size(); ++j) {

                    if( _tmpImp > 0 || _tmpMarkId == 1 ) {
                        file["signimp"] = "1";
                        continue;
                    }

                    if (getfile[j].file_uuid == allFilteredFiles.at(i).file_uuid) {
//                        getfile[j].F_MarkId = "标注重要"; // 更新备注
                        getfile.removeAt(j);
                        break; // 找到后退出循环
                    }
                }

                fileInfos.append(file);
            }



        }
        emit sendDelet_tfilebyid(workstationIp,fileInfos);

        on_pushButton_clicked();

    }


void SearchByType::on_pushButtonImportant_clicked()
{

//        QStringList tfileId ;
        QList<QMap<QString,QString>> fileInfos;
        QString workstationIp = "";
        for (int i=0;i<ui->tableWidget->rowCount();i++) {

            if(ui->tableWidget->item(i,12)->checkState() == Qt::Checked){
//                QMap<QString,QString> file;

//                file["tfile_id"] = getfile.at(i).file_uuid;
//                file["F_UserNo"] = getfile.at(i).F_UserNo;

//                if(workstationIp.isEmpty()){
//                workstationIp = getfile.at(0).F_WorkStationIp;
//                }
//                file["F_WorkStationIp"] = workstationIp;

//                QString markId = getfile.at(i).F_MarkId; // 获取当前文件的 F_MarkId

//                // 根据 F_MarkId 的值进行赋值
//                if (markId == "标注不重要") {
//                    file["F_MarkId"] = "1"; // 如果 F_MarkId 是 0，则赋值为 1
//                    getfile[i].F_MarkId = "标注重要";
//                } else if (markId == "标注重要") {
//                    file["F_MarkId"] = "0"; // 如果 F_MarkId 是 1，则赋值为 0
//                    getfile[i].F_MarkId = "标注不重要";
//                } else {
//                    // 处理其他情况（可选）
//                    file["F_MarkId"] = markId; // 如果不是 0 或 1，保持原值或进行其他处理
//                }
//                file["F_Guid"] = getfile.at(i).F_Guid;

//                file["F_Remark"] = getfile.at(i).F_Remark;

//                fileInfos.append(file);





                QMap<QString,QString> file;

                file["tfile_id"] = allFilteredFiles.at(i).file_uuid;
                file["F_UserNo"] = allFilteredFiles.at(i).F_UserNo;

                if(workstationIp.isEmpty()){
                workstationIp = allFilteredFiles.at(0).F_WorkStationIp;
                }
                file["F_WorkStationIp"] = workstationIp;

                QString markId = allFilteredFiles.at(i).F_MarkId; // 获取当前文件的 F_MarkId

                // 根据 F_MarkId 的值进行赋值
                if (markId == "标注不重要") {
                    file["F_MarkId"] = "1"; // 如果 F_MarkId 是 0，则赋值为 1
//                    getfile[i].F_MarkId = "标注重要";

                    for (int j = 0; j < getfile.size(); ++j) {
                        if (getfile[j].file_uuid == allFilteredFiles.at(i).file_uuid) {
                            getfile[j].F_MarkId = "标注重要"; // 更新备注
                            break; // 找到后退出循环
                        }
                    }
                } else if (markId == "标注重要") {
                    file["F_MarkId"] = "0"; // 如果 F_MarkId 是 1，则赋值为 0
//                    getfile[i].F_MarkId = "标注不重要";


                    for (int j = 0; j < getfile.size(); ++j) {
                        if (getfile[j].file_uuid == allFilteredFiles.at(i).file_uuid) {
                            getfile[j].F_MarkId = "标注不重要"; // 更新备注
                            break; // 找到后退出循环
                        }
                    }
                } else {
                    // 处理其他情况（可选）
                    file["F_MarkId"] = markId; // 如果不是 0 或 1，保持原值或进行其他处理
                }
                file["F_Guid"] = allFilteredFiles.at(i).F_Guid;

                file["F_Remark"] = allFilteredFiles.at(i).F_Remark;

                fileInfos.append(file);
            }


        }
        emit sendImportant_tfilebyid(workstationIp,fileInfos);
        emit sendnetWorkEditFile(fileInfos);
        on_pushButton_clicked();

}


void  SearchByType::renameFilePath(QString fid,QString l_path,QString src_file){

        QFile l_existsfile(src_file);
        if(!l_existsfile.exists()){
            return;
        }

       QFileInfo fileInfo(src_file);

       // 获取文件名（不包含路径）
       QString fileName = fileInfo.fileName();
       qDebug() << "File Name:" << fileName;

       // 获取基础名（不包含路径和扩展名）
       QString baseName = fileInfo.baseName();
       qDebug() << "Base Name:" << baseName;


       QDir dir = fileInfo.absoluteDir(); // 获取文件所在的目录对象

       // 获取目录的路径（不包括文件名）
       QString dirPath = dir.absolutePath(); // 注意：这会包含最后的斜杠（例如："/path/to/your/"）
       qDebug() << "Directory Path:" << dirPath;

    ///////
    QDialog dialog;

    QHBoxLayout *horizontalLayout = new QHBoxLayout();

    QLabel *carLabel = new QLabel("原件名:");
    carLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QLineEdit *carLineEdit = new QLineEdit;
    carLineEdit->setObjectName("carLineEdit");
    carLineEdit->setMinimumSize(140, 35);
    carLineEdit->setStyleSheet("QLineEdit {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");
    carLineEdit->setDisabled(true);
    carLineEdit->setText(fileName);


    horizontalLayout->addWidget(carLabel);
    horizontalLayout->addWidget(carLineEdit);

    // 设置伸缩系数，button1: 1, button2: 2, button3: 1
    horizontalLayout->setStretch(0, 1);
    horizontalLayout->setStretch(1, 6);

    // 4.2 用户号
    QHBoxLayout *horizontalLayout1 = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("修改名:");
    nameLabel->setStyleSheet("QLabel {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");

    QLineEdit *nameLineEdit = new QLineEdit;
    nameLineEdit->setObjectName("nameLineEdit");
    nameLineEdit->setStyleSheet("QLineEdit {  color: rgb(0, 0, 0); font: 14pt \"Sans Serif\";  }");

    nameLineEdit->setMinimumSize(140, 35);
    // nameLineEdit->setEnabled(false);
    //nameLineEdit->setText(dirPath);

    horizontalLayout1->addWidget(nameLabel);
    horizontalLayout1->addWidget(nameLineEdit);


    // 设置伸缩系数，button1: 1, button2: 2, button3: 1
    horizontalLayout1->setStretch(0, 1);
    horizontalLayout1->setStretch(1, 6);
    //horizontalLayout->setStretch(2, 1);

    // 4.4 按钮 栏
    QHBoxLayout *horizontalLayout3 = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定");
    okButton->setStyleSheet("QPushButton {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");
    okButton->resize(150,41);
    okButton->setFixedSize(150,41);

    QPushButton *cancelButton = new QPushButton("取消");
    cancelButton->setStyleSheet("QPushButton {  color: rgb(255, 255, 255); font: 14pt \"Sans Serif\";  }");
    cancelButton->resize(150,41);
    cancelButton->setFixedSize(150,41);

    horizontalLayout3->addWidget(okButton);
    horizontalLayout3->addWidget(cancelButton);

    // 创建垂直布局
    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addLayout(horizontalLayout); // 将水平布局添加到垂直布局
    verticalLayout->addLayout(horizontalLayout1);
    verticalLayout->addLayout(horizontalLayout3);

    verticalLayout->setStretch(0,1);
    verticalLayout->setStretch(1,1);
    verticalLayout->setStretch(2,8);
    verticalLayout->setStretch(3,1);
    //verticalLayout->setStretch(4,1);

    QObject::connect(okButton, &QPushButton::clicked ,[=,&dialog](){

           QString _usrNo =  nameLineEdit->text();
           QString _devNo =  carLineEdit->text();

           if(_usrNo.isEmpty() || _devNo.isEmpty()){
               QMessageBox::warning(nullptr, "警告", "请填写目标文件名!");
               return;

           }


           // 创建 QProcess 对象
           QProcess process;

           QString dst_file = dirPath + "/" + _usrNo;
           process.start("mv", QStringList() << src_file << dst_file);

           process.waitForFinished();
           QString result = process.readAllStandardOutput();

           QMap<QString, QString> file;
           file["tfile_id"] = fid;
           file["F_LocalStorageDriveLetter"] = dst_file; //

          emit  sendRename_tfilebyid(file);


          dialog.close();



    });


    dialog.setLayout(verticalLayout);
    //dialog.setParent(this);

    dialog.resize(583,250);
    dialog.setStyleSheet("background-image: url(:/image/usermanagement8321.png);");
    dialog.setWindowFlag(Qt::FramelessWindowHint);

    connect(cancelButton,&QPushButton::clicked,&dialog,&QDialog::close);


    dialog.exec();



}



void SearchByType::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{

    QProcess *process = qobject_cast<QProcess*>(sender());  // 获取发送信号的对象

    if (process) {

        qDebug() << " 删除对象: " << exitCode;
        process->deleteLater();
    }

}




void SearchByType::onRemarkButtonClicked(int displayedIndex, QString text, const QList<FileInfo>& filteredFiles)
{
    qDebug() << "SearchByType::onRemarkButtonClicked" << text;

    QList<QMap<QString, QString>> fileInfos;
    QMap<QString, QString> file;
    QMap<QString, QString> fileNetWork;

    // 使用 filteredFiles 列表中的索引
    file["tfile_id"] = filteredFiles.at(displayedIndex).file_uuid;
    file["F_UserNo"] = filteredFiles.at(displayedIndex).F_UserNo;
    file["F_WorkStationIp"] = filteredFiles.at(0).F_WorkStationIp; // 这里可能需要确认是否使用第一个元素
    file["F_Remark"] = text;

    fileNetWork["F_Guid"] = filteredFiles.at(displayedIndex).F_Guid;

    QString markId = filteredFiles.at(displayedIndex).F_MarkId; // 获取当前文件的 F_MarkId

    // 根据 F_MarkId 的值进行赋值
    if (markId == "标注不重要") {
        file["F_MarkId"] = "0"; // 如果 F_MarkId 是 0，则赋值为 1
    } else if (markId == "标注重要") {
        file["F_MarkId"] = "1"; // 如果 F_MarkId 是 1，则赋值为 0
    } else {
        // 处理其他情况（可选）
        file["F_MarkId"] = "0"; // 如果不是 0 或 1，保持原值或进行其他处理
    }

    fileNetWork["F_WorkStationIp"] = filteredFiles.at(0).F_WorkStationIp; // 这里可能需要确认是否使用第一个元素
    fileNetWork["F_Remark"] = text;

    fileInfos.append(fileNetWork);
    emit sendRemark_tfilebyid(file);
    emit sendnetWorkEditFile(fileInfos);

    // 更新原始数据
    // 查找 getfile 中与 filteredFiles 中的文件 UUID 匹配的索引
    for (int i = 0; i < getfile.size(); ++i) {
        if (getfile[i].file_uuid == filteredFiles.at(displayedIndex).file_uuid) {
            getfile[i].F_Remark = text; // 更新备注
            break; // 找到后退出循环
        }
    }

    emit get_tb_files_type(0,roleId,userId);
}

void SearchByType::on_btn_upload_clicked()
{
    qDebug()<<"on_btn_upload_clicked :"<<ui->tableWidget->rowCount();
    connect(this,&SearchByType::sendupload_tfilebyid,mysql,&MySqlLite::doQueryFilesById );
    connect(mysql,SIGNAL(mysqlWorkFileClientAdd(QList<QVariantMap> )),mynet,SLOT(netWorkFileClientAdd(QList<QVariantMap> )));

//        bool export_result = false;
        QStringList tfileId ;
        for (int i=0;i<ui->tableWidget->rowCount();i++) {
            //qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();
            if(ui->tableWidget->item(i,12)->checkState() == Qt::Checked){


//                tfileId.append(getfile.at(i).file_uuid);

                tfileId.append(allFilteredFiles.at(i).file_uuid);
            }


        }
        emit sendupload_tfilebyid(tfileId);


}



void SearchByType::onUserItemSelectionChanged(){


   QList<QTableWidgetItem*> list = ui->tableWidget->selectedItems();

   QSet<int> _selectedRows;

   for (QTableWidgetItem* item : list) {
       int row = item->row();
       _selectedRows.insert(row);
   }

   for (int _row : _selectedRows) {
        ui->tableWidget->item(_row,12)->setCheckState(Qt::Checked);

   }


}



void SearchByType::onUserItemCilckChanged(QTableWidgetItem *item){


    int row = item->row(); // 获取行号
    int column = item->column(); // 获取列号
    QString itemText = item->text(); // 获取单元格文本


    int _stat = ui->tableWidget->item(row,12)->checkState();
    if( _stat == Qt::Unchecked){

        ui->tableWidget->item(row,12)->setCheckState(Qt::Checked);

    }else if (_stat == Qt::Checked) {

        ui->tableWidget->item(row,12)->setCheckState(Qt::Unchecked);
    }


}



void SearchByType::on_pushButtonConvert_clicked()
{
    QString _title = "格式转换";
    QString _context = "转码中,请等待完成!";
    QMessageBox messageBox(QMessageBox::Information, _title, _context, QMessageBox::Ok, this);

    qDebug()<<"on_btn_upload_clicked :"<<ui->tableWidget->rowCount();

    QStringList tfileId ;
    QStringList filepaths ;
    for (int i=0;i<ui->tableWidget->rowCount();i++) {
        //qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();
        if(ui->tableWidget->item(i,12)->checkState() == Qt::Checked){

            tfileId.append(allFilteredFiles.at(i).file_uuid);
            filepaths.append(allFilteredFiles.at(i).F_LocalStorageDriveLetter);
        }


    }


    foreach (const QString& strfile, filepaths) {

        qDebug() << strfile;

        // 获取文件类型
        QFileInfo  fileInfo(strfile);
        QString  category = fileInfo.suffix();
        QString  fileType = getFileType(category);
        QString  ffmpegPath  =  Config::getInstance()->Get("wsConfig","FFmpegPath").toString();

        qDebug() << category;
        qDebug() << fileType;
        qDebug() << ffmpegPath;
        qDebug() << " ";


        if(category.toLower() == "avi"){

            if("VIDEO" == fileType ){

                HsConvetWorker * _hswk = new HsConvetWorker(ffmpegPath,strfile,strfile +  ".mp4");
                QThread * _th = new QThread;
                _hswk->moveToThread(_th);

                // 线程结束时清理
                //connect(_th, &QThread::started, _hswk, &HsConvetWorker::dowork);
                connect(_th, &QThread::started, _hswk, &HsConvetWorker::doconvert);
                connect(_th, &QThread::finished, _th, &QThread::deleteLater);
                connect(_hswk, &HsConvetWorker::finished, _th, &QThread::quit);
                connect(_hswk, &HsConvetWorker::finished, _hswk, &HsConvetWorker::deleteLater);

                QObject::connect(_th, &QThread::finished, &messageBox, &QMessageBox::accept);


                _th->start();

            }

        }


    }


    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &messageBox, &QMessageBox::accept);

    messageBox.exec(); // 显示消息框



}


void SearchByType::on_pushButton_clicked()
{
    processList.clear();
    allFilteredFiles.clear();
    ui->tableWidget->setRowCount(0); // 清空表格

    // 1. filetypecomboBox   文件类型
    QString selectedfiletype = ui->filetypecomboBox->currentText();
    // 2. lineEditpernum   人员编号
    QString selectedpernumber = ui->lineEditpernum->text();

    // 3. lineEditpername   人员姓名
    QString selectedpername = ui->lineEditpername->text();
    // 4. significancecomboBox  重要性
    QString selectedsignificance = ui->significancecomboBox->currentText();

    // 5. lineEditshebnum   设备编号
    QString selectedequipnumber = ui->lineEditshebnum->text();
    // 6. lineEditrem   文件备注
    QString selectedRemark = ui->lineEditrem->text();

    // 7. dateEditstart   采集开始时间
    QDate startDate = ui->dateEditstart->date();
    // 8. dateEditend    采集结束时间
    QDate endDate = ui->dateEditend->date();

    QString  dateEditstart = startDate.toString("yyyy-MM-dd 00:00:00");
    QString  dateEditend  =  endDate.toString("yyyy-MM-dd 23:59:59");

    // 拍摄时间
    QDate startDate_create_data  = ui->dateEditstart_create->date();
    QDate endDate_create_data    = ui->dateEditend_create->date();

    QString  startDate_create = startDate_create_data.toString("yyyy-MM-dd 00:00:00");
    QString  endDate_create  =  endDate_create_data.toString("yyyy-MM-dd 23:59:59");


    int maxRowsToDisplay = ui->spinBoxNum->value(); // 从 QSpinBox 获取值

     qDebug() <<  "selectedfiletype:" << selectedfiletype  ;
     qDebug() <<  "selectedpernumber:" << selectedpernumber  ;
     qDebug() <<  "selectedpername:" << selectedpername  ;

     qDebug() <<  "selectedsignificance:" << selectedsignificance  ;
     qDebug() <<  "selectedequipnumber:" << selectedequipnumber  ;
     qDebug() <<  "selectedRemark:" << selectedRemark  ;

     qDebug() <<  "dateEditstart:" << dateEditstart  ;
     qDebug() <<  "dateEditend:" << dateEditend  ;

     qDebug() <<  "startDate_create:" << startDate_create  ;
     qDebug() <<  "endDate_create:" << endDate_create  ;

     qDebug() <<  "maxRowsToDisplay:" << maxRowsToDisplay  ;

     emit get_realdb_files_type(  roleId, userId,   selectedfiletype ,  selectedpernumber,  selectedpername,
                                  selectedsignificance,  selectedequipnumber,    selectedRemark ,    dateEditstart ,
                                  dateEditend , startDate_create , endDate_create, maxRowsToDisplay, currPage);

}

void SearchByType::on_pushButton_2_clicked()
{

}


void SearchByType::on_pushButton_priv_clicked()
{
    currPage--;
    if(currPage == 1){
        ui->pushButton_priv->setVisible(false);
        ui->pushButton_next->setVisible(true);

    }

    on_pushButton_clicked();
}



void SearchByType::on_pushButton_next_clicked()
{

    currPage++;
    ui->pushButton_priv->setVisible(true);

    on_pushButton_clicked();
}



void SearchByType::on_pushButton_3_clicked()
{


    qDebug()<<"ui->tableWidget->rowCount()"<<ui->tableWidget->rowCount();

//        QStringList tfileId ;
        QList<QMap<QString,QString>> fileInfos;
        QString workstationIp = "";
        for (int i=0;i<ui->tableWidget->rowCount();i++) {

            if(ui->tableWidget->item(i,12)->checkState() == Qt::Checked){

                QMap<QString,QString> file;

                file["tfile_id"] = allFilteredFiles.at(i).file_uuid;

                fileInfos.append(file);
            }

        }

        // 标记上传
        emit sendAutoUpload_tfilebyid(workstationIp,fileInfos);

        QString _title = "标记上传";
        QString _context = "标记成功请等待后台自动上传!";
        QMessageBox messageBox(QMessageBox::Information, _title, _context, QMessageBox::Ok, this);


        QTimer timer;
        QObject::connect(&timer, &QTimer::timeout, &messageBox, &QMessageBox::accept);
        timer.setSingleShot(true);
        timer.start(3000);

        messageBox.exec(); // 显示消息框

        on_pushButton_clicked();


}



void SearchByType::onHeaderClicked(int column)
{
    static bool _checked = false;
    if(column == 12){

        for(int row = 0 ; row <ui->tableWidget->rowCount() ;row++ ){

            int _stat = ui->tableWidget->item(row,column)->checkState();
            if(_checked == false){

                ui->tableWidget->item(row,column)->setCheckState(Qt::Checked);
            }else{

                 ui->tableWidget->item(row,column)->setCheckState(Qt::Unchecked);
            }

        }


        if(_checked == false){
             _checked = true;

        }else{
            _checked = false;

        }


    }


}
