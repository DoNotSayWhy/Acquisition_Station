#include "loggerqueryform.h"
#include "ui_loggerqueryform.h"
#include <QFont>
#include <QHeaderView>

namespace {

void applyPortraitLoggerLayout(Ui::LoggerQueryForm *ui, QWidget *page)
{
    const int width = page->width();
    const int height = page->height();
    const int margin = 24;
    const int controlHeight = 48;

    ui->gridLayout->setEnabled(false);
    ui->labellogtype->setGeometry(margin, 22, 70, controlHeight);
    ui->comboBoxlogtype->setGeometry(100, 22, 70, controlHeight);
    ui->labeldata->setGeometry(180, 22, 70, controlHeight);
    ui->dateEditstart->setGeometry(260, 22, 110, controlHeight);
    ui->label->setGeometry(370, 22, 20, controlHeight);
    ui->dateEditend->setGeometry(400, 22, 110, controlHeight);
    ui->pushButtonsearch->setGeometry(510, 22, 80, controlHeight);
    ui->pushButtonclear->setGeometry(600, 22, 80, controlHeight);

    ui->tableWidget->setGeometry(margin, 92, width - margin * 2, height - 156);
    ui->labelsum->setGeometry(width - 150, height - 54, 126, 34);
    const QFont font("Sans Serif", 13);
    ui->labellogtype->setFont(font);
    ui->labeldata->setFont(font);
    ui->comboBoxlogtype->setFont(font);
    ui->dateEditstart->setFont(font);
    ui->dateEditend->setFont(font);
    ui->pushButtonsearch->setFont(font);
    ui->pushButtonclear->setFont(font);
    ui->tableWidget->setFont(QFont("Sans Serif", 12));
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(46);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(false);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->setColumnWidth(1, 180);
    ui->tableWidget->setColumnWidth(2, 110);
    ui->tableWidget->setColumnWidth(3, 350);
    ui->tableWidget->setColumnWidth(4, 80);
}

}

LoggerQueryForm::LoggerQueryForm(QString userid,QString roleId,MySqlLite *sqltie,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoggerQueryForm)
{
    ui->setupUi(this);
    mysql = sqltie;
    this->userId = userid;
    this->roleId = roleId;

    ui->dateEditstart->setDate(QDate::currentDate()); // 将开始日期设置为当前日期
    ui->dateEditend->setDate(QDate::currentDate());   // 将结束日期设置为当前日期



    // 创建表格控件
    ui->tableWidget->setColumnCount(5); //

    ui->tableWidget->setHorizontalHeaderLabels({"编号","时间", "日志类型", "日志内容", "人员编号"});

    // 设置列宽
    ui->tableWidget->setColumnWidth(0, 90);
    ui->tableWidget->setColumnWidth(1, 180);
    ui->tableWidget->setColumnWidth(2, 120);
    ui->tableWidget->setColumnWidth(3, 400);
    ui->tableWidget->setColumnWidth(4, 150);

    // 设置行高
    int rowHeight = 100; // 设置行高
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        ui->tableWidget->setRowHeight(i, rowHeight);
    }

    ui->comboBoxlogtype->addItems(QStringList() <<"全部"<< "数据查询" << "系统设置" << "解锁" <<"更新"<< "关机" << "个人中心" << "未知类型" << "执法记录仪接入"<<"执法记录仪移出"<<"开机");
    applyPortraitLoggerLayout(ui, this);
    connect(this,&LoggerQueryForm::logSelectT_logAllTOMSQ,mysql,&MySqlLite::MSQSelectT_logAllTOLog);
    connect(mysql,&MySqlLite::t_log_successTOLog,this,&LoggerQueryForm::getLogData);
    emit logSelectT_logAllTOMSQ( userid, roleId);
//    this->showMaximized();
}

void LoggerQueryForm::getLogData(QList<LogInfo> log_alls){
    getLogDatas = log_alls;

    // 获取表格控件
    QTableWidget *tableWidget = ui->tableWidget; // 直接使用ui中的tableWidget

    // 设置行数
    tableWidget->setRowCount(log_alls.size());


    for (int i = 0; i < log_alls.size(); ++i) {

        // 将其他数据添加到对应的列
        tableWidget->setItem(i, 0, new QTableWidgetItem(log_alls[i].id));
        tableWidget->setItem(i, 1, new QTableWidgetItem(log_alls[i].createdAt));
        tableWidget->setItem(i, 2, new QTableWidgetItem(log_alls[i].f_logType));
        tableWidget->setItem(i, 3, new QTableWidgetItem(log_alls[i].f_description));
        tableWidget->setItem(i, 4, new QTableWidgetItem(log_alls[i].f_loginName));

    }

    // 显示总量
    QLabel *totalLabel = ui->labelsum; // 假设总量标签在UI文件中定义
    totalLabel->setText(QString("总量: %1").arg(log_alls.size()));
}

LoggerQueryForm::~LoggerQueryForm()
{
    delete ui;
}

void LoggerQueryForm::on_pushButtonsearch_clicked() {
    QString selectedLogType = ui->comboBoxlogtype->currentText(); // 获取下拉框选中的日志类型
    QDate startDate = ui->dateEditstart->date(); // 获取开始日期
    QDate endDate = ui->dateEditend->date();     // 获取结束日期

    // 清空表格
    ui->tableWidget->setRowCount(0);

    // 过滤数据并填充表格
    for (const LogInfo &log : getLogDatas) {

        // 提取日期部分
        QString dateStr = log.createdAt.split(" ").first();

        // 转换为 QDate
        QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");

        if (!date.isValid()) {
            qDebug() << "Invalid date format:" << log.createdAt;
            continue;
        }

        bool logTypeMatches = (selectedLogType == "全部") || (log.f_logType == selectedLogType);

        if (logTypeMatches && date >= startDate && date <= endDate) {
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row); // 插入新行


            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(log.id));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(log.createdAt));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(log.f_logType));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(log.f_description));
            ui->tableWidget->setItem(row, 4, new QTableWidgetItem(log.f_loginName));
        }
    }
}

void LoggerQueryForm::on_pushButtonclear_clicked()
{
    // 重置下拉框
    ui->comboBoxlogtype->setCurrentIndex(0);

    // 重置日期选择器
    ui->dateEditstart->setDate(QDate::currentDate());
    ui->dateEditend->setDate(QDate::currentDate());

    // 清空表格
    ui->tableWidget->setRowCount(0);


    for (const LogInfo &log : getLogDatas) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row); // 插入新行
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(log.id));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(log.createdAt));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(log.f_logType));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(log.f_description));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(log.f_loginName));
    }
}
