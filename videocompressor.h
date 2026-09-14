#ifndef VIDEOCOMPRESSOR_H
#define VIDEOCOMPRESSOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QProcess>
#include <QThread>

class VideoCompressor : public QObject
{
    Q_OBJECT
public:
    explicit VideoCompressor(const QString &uploadPath, const QString &newFileDoc, const QString &newPath, const QString &ffmpegPath,QObject *parent = nullptr);

    // 开始压缩视频
    void compress();

signals:
    // 压缩完成信号
    void compressionFinished(const QString &outputFile, bool success);

    // 错误发生信号
    void errorOccurred(const QString &errorMessage);

private:
    QString uploadPath;  // 上传路径
    QString newFileDoc;  // 新文件文档名
    QString newPath;     // 源文件路径
    QString myffmpegPath = nullptr;

    // 创建压缩文件
    void createdPreviewFile(const QString &sourceFile, const QString &targetFile);

    // 配置 FFmpeg 进程
    QProcess* configureFFmpegProcess(const QString &sourceFile, const QString &targetFile);
};

#endif // VIDEOCOMPRESSOR_H
