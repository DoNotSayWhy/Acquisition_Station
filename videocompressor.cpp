#include "videocompressor.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

VideoCompressor::VideoCompressor(const QString &uploadPath, const QString &newFileDoc, const QString &newPath, const QString &ffmpegPath,QObject *parent)
    : QObject(parent), uploadPath(uploadPath), newFileDoc(newFileDoc), newPath(newPath),myffmpegPath(ffmpegPath) {}

void VideoCompressor::compress() {
    QString compressPath = uploadPath + "/compress/";
    QDir dir;

    // 创建压缩文件夹
    if (!dir.exists(compressPath)) {
        dir.mkpath(compressPath);
    }

    //QString newFile = compressPath + newFileDoc + "_2.mp4";
    QString newFile = compressPath + newFileDoc + "_1.avi";

    // 创建 QThread 实例
    QThread *thread = new QThread(this);
    this->moveToThread(thread); // 将当前对象移动到新线程

    connect(thread, &QThread::started, [this, newFile]() {
        createdPreviewFile(newPath, newFile);
    });

    connect(this, &VideoCompressor::compressionFinished, thread, &QThread::quit); // 压缩完成后退出线程
    connect(this, &VideoCompressor::errorOccurred, thread, &QThread::quit); // 发生错误时退出线程
    connect(thread, &QThread::finished, thread, &QObject::deleteLater); // 清理线程

    thread->start(); // 启动线程
}

void VideoCompressor::createdPreviewFile(const QString &sourceFile, const QString &targetFile) {
    QProcess *process = new QProcess(this); // 确保 process 的父对象为 this

    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [this, process, targetFile](int exitCode,QProcess::ExitStatus exitStatus) {
        // 确认进程的结束状态
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            emit compressionFinished(targetFile, true);
        } else {
            emit errorOccurred("FFmpeg process exited with error code: " + QString::number(exitCode));
        }

        // 在此处安全删除 QProcess 对象
        process->deleteLater();
    });

    // 配置 FFmpeg 参数
    QString ffmpegPath = myffmpegPath;
    QStringList arguments;

    // mp4
    /*
    arguments << "-i" << sourceFile
              << "-s" << "640x480"
              << "-r" << "24"
              << "-b:v" << "400k"
              << "-vcodec" << "libx264"
              << "-preset" << "ultrafast"
              << "-qp" << "35"
              << "-y" << targetFile;
    */

     arguments << "-i" << sourceFile
              << "-s" << "640x480"
              << "-c:v" << "libx264"
              << "-c:a" << "copy"
              << "-y" << targetFile;


    process->start(ffmpegPath, arguments);
    if (!process->waitForStarted()) {
        emit errorOccurred("Failed to start FFmpeg process: " + process->errorString());
        delete process; // 启动失败，立即释放
    }
}

QProcess* VideoCompressor::configureFFmpegProcess(const QString &sourceFile, const QString &targetFile) {
    QString ffmpegPath = myffmpegPath;
    QProcess *process = new QProcess(this);

    // 配置 FFmpeg 参数
    QStringList arguments;
    arguments << "-i" << sourceFile
              << "-s" << "640x480"
              << "-r" << "24"
              << "-b:v" << "400k"
              << "-vcodec" << "libx264"
              << "-preset" << "ultrafast"
              << "-qp" << "35"
              << "-y" << targetFile;

    process->start(ffmpegPath, arguments);
    if (!process->waitForStarted()) {
        emit errorOccurred("Failed to start FFmpeg process: " + process->errorString());
        delete process; // 释放内存
        return nullptr;
    }

    return process;
}
