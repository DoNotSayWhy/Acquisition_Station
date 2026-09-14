#include "fingerprintform.h"
#include "ui_fingerprintform.h"
#include "MatchTwoM.h"
#include "QDebug"

FingerPrintForm::FingerPrintForm(MySqlLite *sqlite,QWidget *parent,QString userNo,QString userName) :
    QWidget(parent),mUserNo(userNo),mUserName(userName),mSqlite(sqlite),
    ui(new Ui::FingerPrintForm)
{
    ui->setupUi(this);
    mWorkerThread = NULL;
    initFingerUtils();
    //processFingerFeature();
    initFingerData();
}

FingerPrintForm::~FingerPrintForm()
{
    if(mWorkerThread != NULL){
        mWorkerThread->quit();
    }
    delete mFingerUtils;
    delete ui;
}


void FingerPrintForm::on_pushButtonBack_clicked(){

    emit clickBack();

}


void FingerPrintForm::initFingerUtils(){
    mWorkerThread = new QThread;
    mFingerUtils = new ZiangFingerUtility;
    mFingerUtils->moveToThread(mWorkerThread);

    connect(mFingerUtils, &ZiangFingerUtility::updateStatus, this, &FingerPrintForm::updateFingerStatus);
    connect(mFingerUtils, &ZiangFingerUtility::uploadFingerChar, this, &FingerPrintForm::uploadFingerChar);
    connect(mFingerUtils, &ZiangFingerUtility::uploadRecognizeChar,this,&FingerPrintForm::uploadRecognizeResult);
    connect(mFingerUtils, &ZiangFingerUtility::searchFingerStatus,this,&FingerPrintForm::updateSearchStatus);

    connect(this, &FingerPrintForm::startEnroll, mFingerUtils, &ZiangFingerUtility::startEnroll);
    connect(this, &FingerPrintForm::startRecognize, mFingerUtils, &ZiangFingerUtility::startRecognize);

    mWorkerThread->start();
}

void FingerPrintForm::updateFingerStatus(QString msg){
    ui->labelHintMsg->setText(msg);
}

void FingerPrintForm::updateSearchStatus(QString msg){
    ui->labelHintMsg->setText(msg);
}

void FingerPrintForm::uploadRecognizeResult(const QByteArray featureData){
    //
    ui->labelHintMsg->setText("now comparing...");
    FingerFeatureData data = getRecognizeResult(featureData);
    if(!data.userNo.isEmpty()){
        ui->labelHintMsg->setText(data.userNo);
        emit recognizeResult(data.userNo,data.userName);
    } else {
        ui->labelHintMsg->setText("Not recognize.");
        //continue to recognize.
        emit startRecognize();

    }
}

void FingerPrintForm::uploadFingerChar(const QByteArray featureData){
    //采集到指纹数据
    FingerFeatureData data;
    data.featureData = featureData;
    data.userNo = mUserNo;
    data.userName = mUserName;
    data.ext1 = "";
    data.ext2 = "";

    mSqlite->insertFingerFeature(data);

    //
    emit enrollResult(mUserNo);
}

void FingerPrintForm::processFingerFeature(){
    if(!mUserNo.isEmpty()){
        ui->labelHintTitle->setText("指纹采集");
        //register
        emit startEnroll();
    } else {
        //recognize
        ui->labelHintTitle->setText("指纹识别");
        emit startRecognize();
    }

}

void FingerPrintForm::initFingerData(){
    mFeatureDataList = mSqlite->getFingerFeatureList();
    qDebug() << "mFeatureDataList.size() = " << mFeatureDataList.size();
}

FingerFeatureData FingerPrintForm::getRecognizeResult(QByteArray fingerFeature){
    unsigned char *fingerData = (unsigned char *)fingerFeature.data();
    FingerFeatureData resultData;
    for(int i = 0;i < mFeatureDataList.size();++i){
        unsigned char *tmp = (unsigned char *)(mFeatureDataList[i].featureData.data());
        short tmpScore = MatchScore(fingerData,tmp);
        qDebug() << "comparing:" << mFeatureDataList[i].userNo << " Score = " << tmpScore;
        if(tmpScore >= 50){
            resultData.userNo = mFeatureDataList[i].userNo;
            resultData.userName = mFeatureDataList[i].userName;
            break;
        }
    }
    return resultData;
}















