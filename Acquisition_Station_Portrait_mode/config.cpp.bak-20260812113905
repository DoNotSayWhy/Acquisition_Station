#include "config.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QProcess>
#include <QStringList>

Config::Config(QString qstrfilename)
{

    if (qstrfilename.isEmpty())
    {
        m_qstrFileName = QCoreApplication::applicationDirPath() + "/Config.ini";
    }
    else {
        m_qstrFileName = qstrfilename;
    }

    m_psetting = new QSettings(m_qstrFileName, QSettings::IniFormat);
    // qDebug() << m_qstrFileName;

    initializeConfig() ;





}

Config::~Config()
{
    delete m_psetting;
    m_psetting = 0;
}
void Config::Set(QString qstrnodename,QString qstrkeyname,QVariant qvarvalue)
{
    m_psetting->setValue(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname), qvarvalue);
}

void Config::Sync()
{
    m_psetting->sync();
}


QVariant Config::Get(QString qstrnodename,QString qstrkeyname)
{
    QVariant qvar = m_psetting->value(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname));
    return qvar;
}
void Config::initializeConfig() {

    if (!m_psetting->contains("Images/mainImage")) {
        m_psetting->setValue("Images/mainImage", "./hisense.png");
    }


    if (!m_psetting->contains("language/setlanguagefile")) {
        m_psetting->setValue("language/setlanguagefile", "lang_cn.qm");
    }


    if (!m_psetting->contains("path/HSdbpath")) {
        m_psetting->setValue("path/HSdbpath", "./HSzhifayi.db");
    }

    if (!m_psetting->contains("wsConfig/logo")) {
        m_psetting->setValue("wsConfig/logo", "./6008.png");
    }
    QString ffmpegPath = findFFmpegPath();


    m_psetting->setValue("wsConfig/FFmpegPath", ffmpegPath);

    // 提交更改
    m_psetting->sync();

}
QString Config::findFFmpegPath() {
    // 使用 which 命令查找 FFmpeg
    QProcess process;
    process.start("which", QStringList() << "ffmpeg");
    process.waitForFinished();

    QString ffmpegPath = process.readAllStandardOutput().trimmed();
    if (!ffmpegPath.isEmpty()) {
        return ffmpegPath; // 找到 FFmpeg 路径
    }

    // 如果未找到，遍历 PATH 环境变量
//    QStringList pathList = qgetenv("PATH").split(':'); // Linux 使用冒号分隔
//    for (const QString &path : pathList) {
//        QDir dir(path);
//        QString potentialPath = dir.absoluteFilePath("ffmpeg");
//        if (QFile::exists(potentialPath)) {
//            return potentialPath; // 找到 FFmpeg 路径
//        }
//    }

    qDebug() << "FFmpeg not found.";
    return QString(); // 返回空字符串表示未找到
}


Config* Config::_cfg_instance = nullptr;
std::mutex Config::_cfg_mtx;




