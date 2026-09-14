#ifndef FINGERPRINTFORM_H
#define FINGERPRINTFORM_H

#include <QWidget>
#include "mysqllite.h"
#include "ziangfingerutility.h"
#include <vector>

using namespace std;

namespace Ui {
class FingerPrintForm;
}

class FingerPrintForm : public QWidget
{
    Q_OBJECT

public:
    explicit FingerPrintForm(MySqlLite *sqlite,QWidget *parent = 0,QString userNo = "",QString userName = "");
    ~FingerPrintForm();

public:
    MySqlLite *mSqlite;
    QString mUserNo;
    QString mUserName;
    ZiangFingerUtility* mFingerUtils;
    QThread* mWorkerThread;
    std::vector<FingerFeatureData> mFeatureDataList;



public:
    void initFingerUtils();
    void processFingerFeature();

private:
    void initFingerData();
    FingerFeatureData getRecognizeResult(QByteArray fingerFeature);

public slots:
    void on_pushButtonBack_clicked();

    void uploadFingerChar(const QByteArray featureData);

    void uploadRecognizeResult(const QByteArray featureData);

    void updateFingerStatus(QString msg);

    void updateSearchStatus(QString msg);

signals:
   void clickBack();

   void startEnroll();
   void startRecognize();

   void enrollResult(QString userNo);
   void recognizeResult(QString userNo,QString userName);

private:
    Ui::FingerPrintForm *ui;
};

#endif // FINGERPRINTFORM_H
