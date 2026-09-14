#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H


#include <QThread>
#include <QProcess>
class AudioPlayer : public QThread
{
    Q_OBJECT
public:
    explicit AudioPlayer(const QString &audioFilePath, bool loop = false, QObject *parent = nullptr);
    ~AudioPlayer();

public slots:


protected:
    void run() override;

private:
    QString m_audioFilePath;
    bool m_loop;
    QProcess *m_process; // Process to run ffplay
};

#endif // AUDIOPLAYER_H
