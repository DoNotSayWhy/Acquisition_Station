#include "loginform.h"
#include "ui_loginform.h"
#include <QMouseEvent>
#include "ziangfingerutility.h"
/**
 * @brief LoginForm::LoginForm
 * @param type
 * @param parent
 */

LoginForm::LoginForm(int type,MySqlLite *sqltie,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);
    indexFace = -1;
    indexFinger = -1;
    // 在构造函数中添加事件过滤器
    ui->lineEdit_username->installEventFilter(this);
    ui->lineEdit_password->installEventFilter(this);
    ui->lineEdit_username->setEnabled(true);
    ui->lineEdit_password->setEnabled(true);
    currentFocus = ui->lineEdit_username;
    this->type = type;
    this->mysql = sqltie;
    // 移除窗口栏
    setWindowFlags(Qt::CustomizeWindowHint);
    // 是否开启指纹人脸
#ifdef USE_FACE_FINGER
    ifopen_face_finger(true);
#else
    ifopen_face_finger(false);
#endif

//    setWindowFlags(Qt::WindowCloseButtonHint);
    setFixedSize(this->width(),this->height());
    naManager = new QNetworkAccessManager(this);
    connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));


    QRegExp regx("[0-9A-Za-z]{1,20}");
    QValidator *validator = new QRegExpValidator(regx,ui->lineEdit_username);
    ui->lineEdit_username->setValidator(validator);
    // 连接Cancel按钮的clicked信号到窗口的close()槽
    connect(ui->cancelpushButton, &QPushButton::clicked, this, &LoginForm::close);
    // 连接按钮信号
    connect(ui->pushButton_2, &QPushButton::clicked, this, [=]() { appendToInput("1"); });
    connect(ui->pushButton_3, &QPushButton::clicked, this, [=]() { appendToInput("2"); });
    connect(ui->pushButton_4, &QPushButton::clicked, this, [=]() { appendToInput("3"); });
    connect(ui->pushButton_5, &QPushButton::clicked, this, [=]() { appendToInput("4"); });
    connect(ui->pushButton_6, &QPushButton::clicked, this, [=]() { appendToInput("5"); });
    connect(ui->pushButton_7, &QPushButton::clicked, this, [=]() { appendToInput("6"); });
    connect(ui->pushButton_8, &QPushButton::clicked, this, [=]() { appendToInput("7"); });
    connect(ui->pushButton_9, &QPushButton::clicked, this, [=]() { appendToInput("8"); });
    connect(ui->pushButton_10, &QPushButton::clicked, this, [=]() { appendToInput("9"); });
    connect(ui->pushButton_11, &QPushButton::clicked, this, [=]() {
        qDebug()<<"currentFocus :";
        if (currentFocus) {
            if (currentFocus == ui->lineEdit_username) {
                qDebug()<<"currentFocus 1111:"<<ui->lineEdit_username->text();
                if (!ui->lineEdit_username->text().isEmpty()) {
                    QString newText = ui->lineEdit_username->text();
                    newText.chop(1); // 删除最后一位
                    ui->lineEdit_username->setText(newText);
                }
            } else if (currentFocus == ui->lineEdit_password) {
                if (!ui->lineEdit_password->text().isEmpty()) {
                    QString newText = ui->lineEdit_password->text();
                    newText.chop(1); // 删除最后一位
                    ui->lineEdit_password->setText(newText);
                }
            }
        }
    });
    connect(ui->pushButton_12, &QPushButton::clicked, this, [=]() { appendToInput("."); });
    connect(ui->pushButton_13, &QPushButton::clicked, this, [=]() { appendToInput("0"); });


}


bool LoginForm::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QString objectName = obj->objectName();
            qDebug() <<"objectName :"<<objectName;
        if (objectName == "lineEdit_username") {
            ui->lineEdit_username->setFocus();
            currentFocus = ui->lineEdit_username;
        } else if (objectName == "lineEdit_password") {
            ui->lineEdit_password->setFocus();
            currentFocus = ui->lineEdit_password;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void LoginForm::appendToInput(const QString &digit) {
//    qDebug() << "Appending digit:" << digit; // 添加调试信息
//    ui->lineEdit_username->setFocus();
    if (currentFocus) {
        if (currentFocus == ui->lineEdit_username) {
            ui->lineEdit_username->insert(digit);
//            qDebug() << "Username field content:" << ui->lineEdit_username->text();
        } else if (currentFocus == ui->lineEdit_password) {
            ui->lineEdit_password->insert(digit);
//            qDebug() << "Password field content:" << ui->lineEdit_password->text();
        }
    }
}

LoginForm::~LoginForm()
{
    qDebug()<<"~LoginForm!!";
    if (face) {
        delete face;
    }
    delete ui;
}

void LoginForm::on_pushButton_clicked()
{

//    this->hide();

//    MainWindow *main = new MainWindow(this);
//    main->show();
//    this->destroy();

//    ui->lineEdit_username->text();
//    ui->lineEdit_password->text();
    if(ui->lineEdit_username->text()=="" || ui->lineEdit_password->text() ==""){

        qDebug()<<"username or password can't be null!";


        QMessageBox::warning(NULL, tr("INFO"), tr("username or password can't be null!"), QMessageBox::Ok );

    }else{
//        QString postData = QString("%1%2%3%4")
//                .arg("username=").arg(ui->lineEdit_username->text())
//                .arg("&password=").arg(ui->lineEdit_password->text());
//        QString url =  Config::getInstance()->Get("server","serverIp").toString() +"/api/Login/AjaxLogin";
//        qDebug()<<"loginurl:"<<url;
//        request.setUrl(QUrl(url));
//        request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
//        naManager->post(request, postData.toLocal8Bit());
          QString username = ui->lineEdit_username->text();
          QString password = ui->lineEdit_password->text();
//          qDebug()<<"LoginForm::on_pushButton_clicked() username"<<username<<"password"<<password;
          emit sigLogSelectUserNo(username,password);
//          qDebug() << "sigLogSelectUserNo signal emitted.";

    }

}



void LoginForm::requestFinished(QNetworkReply* reply) {
    // 获取http状态码
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
        qDebug() << "reason=" << reason.toString();

    QNetworkReply::NetworkError err = reply->error();
    if(err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    }
    else {
        // 获取返回内容
        QString str = reply->readAll();
        qDebug() << str;
        QJsonParseError err_rpt;
        QJsonDocument  root_Doc = QJsonDocument::fromJson(str.toLocal8Bit(), &err_rpt);//字符串格式化为JSON

        if(err_rpt.error != QJsonParseError::NoError)
        {
            qDebug() << "JSON格式错误";
            //return -1;
        }else{
            QJsonObject root_Obj = root_Doc.object();
            int result_status = root_Obj.value("status").toInt();
            if(result_status == 0){
                //login fail
                QString result_msg = root_Obj.value("msg").toString();
                qDebug()<<"result_status:"<<result_status<<"msg:"<<result_msg;

                QMessageBox::warning(NULL, tr("Login Fail"), result_msg, QMessageBox::Ok );
            }
            if(result_status == 1){
                //login success
                QJsonObject result_data = root_Obj.value("data").toObject();
                QString result_token = result_data.value("token").toString();
                QString result_jurstr = result_data.value("jurstr").toString();

                login_UserName = ui->lineEdit_username->text();
                qDebug()<<"result_status:"<<result_status<<"result_jurstr:"<<result_jurstr;

                QStringList jurstrlist = result_jurstr.split(",");
                qDebug()<<"jurstrlist"<<jurstrlist;

                emit loginsuccess(type,login_UserName,jurstrlist);
//                    this->hide();

//                    MainWindow *main = new MainWindow(ui->lineEdit_username->text(),this);
//                    main->show();
//                    this->destroy();


            }




            //qDebug()<<"result_status:"<<result_status<<"msg:"<<result_msg<<"result_token:"<<result_token<<"result_jurstr:"<<result_jurstr;
        }

    }
}

void LoginForm::LogSelectUserNoSuc(const QString &username, const QString &roleId)
{
//    qDebug()<<"LoginForm::LogSelectUserNoSuc START";

    emit loginsuccessRoleId(type,username,roleId);
}

void LoginForm::ifopen_face_finger(bool isopen)
{
    ui->pushButtonfinger->setVisible(isopen);  // 隐藏按钮
    ui->pushButtonface->setVisible(isopen);
}

void LoginForm::on_pushButtonfinger_clicked()
{
    if (mFingerForm == nullptr) {
        mFingerForm = new FingerPrintForm(mysql,this);
        indexFinger = ui->stackedWidget->addWidget(mFingerForm);
        mFingerForm->show();
        connect(mFingerForm, SIGNAL(clickBack()), this, SLOT(goBackLogDialog()));
        connect(mFingerForm, SIGNAL(recognizeResult(QString,QString)), this, SLOT(fingerRecognizeResult(QString,QString)));
    }
    if(indexFinger != -1){
        mFingerForm->processFingerFeature();
        ui->stackedWidget->setCurrentIndex(indexFinger);
    }
}

void LoginForm::on_pushButtonface_clicked()
{
    if (!face) {
        // 如果会使用注册功能，传userNo要真实
        QString userNo = "0";
        face = new FaceFormUtility(this,mysql,userNo);
        //ui->stackedWidget->insertWidget(1, face);
        indexFace = ui->stackedWidget->addWidget(face);
//        face->startCameraPreview();


        face->FaceRecognition();
        face->show();

        connect(face, SIGNAL(goback()), this, SLOT(goBackLogDialog()));
        connect(face,&FaceFormUtility::faceReSuccesToLog ,this, &LoginForm::showForFaceR);
    }

    if(indexFace != -1){
         ui->stackedWidget->setCurrentIndex(indexFace);
    }
}


void LoginForm::goBackLogDialog(){
//    qDebug()<<"LoginForm::goBackLogDialog START";

    ui->stackedWidget->setCurrentIndex(0);

    if (face) {
        disconnect(face, SIGNAL(goback()), this, SLOT(goBackLogDialog()));

        delete face; // 删除 face 对象
        face = nullptr; // 设置为 nullptr
    }
}

void LoginForm::fingerRecognizeResult(QString userNo,QString userName){
    //指纹识别之后返回的用户名
    emit sigLogSelectUserNo(userNo,"");
}


void LoginForm::showForFaceR(QString userNo)
{
    QString password = nullptr;
    emit sigLogSelectUserNo(userNo,password);
}
