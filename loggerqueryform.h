#ifndef LOGGERQUERYFORM_H
#define LOGGERQUERYFORM_H

#include <QWidget>
#include "mysqllite.h"

namespace Ui {
class LoggerQueryForm;
}

class LoggerQueryForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoggerQueryForm(QString userid,QString roleId,MySqlLite *sqltie,QWidget *parent = 0);
    ~LoggerQueryForm();

private:
    Ui::LoggerQueryForm *ui;
signals:
    void logSelectT_logAllTOMSQ(QString userid,QString roleId);
private slots:
    void getLogData(QList<LogInfo> log_alls);
    void on_pushButtonsearch_clicked();

    void on_pushButtonclear_clicked();

private:
    QList<LogInfo> getLogDatas;
    MySqlLite *mysql;
    QString roleId;
    QString userId;
};

#endif // LOGGERQUERYFORM_H
