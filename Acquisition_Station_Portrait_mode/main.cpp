#include "mainwindow.h"
#include "wspairdeviceform.h"
#include <QApplication>


#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <QString>
#include <QTranslator>

#include <QCryptographicHash>

const qint64 MAX_LOG_FILE_SIZE = 15 * 1024 * 1024; // 15 MB
void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 加锁
    static QMutex mutex;
    mutex.lock();
    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        break;
    default:break;
    }

    // 设置输出信息格式
    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
    QString current_date = QString("(%1)").arg(current_date_time);
    // QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);
    QString message = QString("%1 %2").arg(current_date).arg(msg);
    // 输出信息至文件中（读写、追加形式）
    QFile file("log.txt");
    // 检查文件大小
    if (file.exists() && file.size() >= MAX_LOG_FILE_SIZE) {
        // 删除旧文件
        file.remove(); // 删除文件
    }

    file.open(QIODevice::WriteOnly | QIODevice::Append);
    //QMessageBox::warning(NULL, "INFO", QString(QCoreApplication::applicationDirPath()), QMessageBox::Ok );

    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    // 解锁
    mutex.unlock();


}



QTranslator *trans =NULL;
int main(int argc, char *argv[])
{


    // QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    // 用于启用高DPI缩放支持
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

//    QGuiApplication app(argc, argv);

    // qInstallMessageHandler(outputMessage);


    QApplication a(argc, argv);

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QString language = Config::getInstance()->Get("language","setlanguagefile").toString();


    qDebug()<<"language:"<<language;
    trans = new QTranslator;

    trans->load(language);
    a.installTranslator(trans);

    MainWindow w;

    // test windows show
    //WsPairDeviceForm w;

    w.setAttribute(Qt::WA_DeleteOnClose);


    w.show();

    //LoginForm login;
    //login.show();

//        if (login.exec() == QDialog::Accepted)//调用login.exec()，阻塞主控制流，直到完成返回，继续执行主控制流
//        {
//            //w.show();
//            return a.exec();
//        }


    return a.exec();

}
