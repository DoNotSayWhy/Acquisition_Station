#include "audioplayer.h"
#include <QMessageBox>
#include <QDebug>
#include <QMessageBox>
#include <QDebug>
#include <qfile.h>

AudioPlayer::AudioPlayer(const QString &audioFilePath, bool loop, QObject *parent)
    : QThread(parent), m_audioFilePath(audioFilePath), m_loop(loop), m_process(nullptr) {
}

AudioPlayer::~AudioPlayer() {

    // Ensure the process is terminated if it's still running
    if (m_process) {
        qDebug() << "  AudioPlayer::~AudioPlayer delete m_process  ============= ";
        m_process->terminate();
        m_process->waitForFinished();
        delete m_process;


    }
}




void AudioPlayer::run() {
    QFile audioFile(m_audioFilePath);
    if (!audioFile.exists()) {
        qDebug() << "Audio file does not exist:" << m_audioFilePath;
        return; // End thread if file does not exist
    }

    // Create a QProcess to run ffplay
    m_process = new QProcess(this);


    QStringList arguments;

     qDebug() << "paly  m_audioFilePath :" << m_audioFilePath;
    // Add arguments for ffplay
    arguments << "-nodisp" << "-autoexit"; // Use -nodisp for no video display, -autoexit to exit after playback
    if (m_loop) {
        arguments << "-loop"; // Loop the audio
    }
    arguments << m_audioFilePath; // The audio file path

    // Start ffplay
    m_process->start("ffplay", arguments);

    // Wait for the process to finish if not looping
    if (!m_loop) {
        m_process->waitForFinished(); // Wait for playback to finish

        qDebug() << "  paly succ  over  :" << m_audioFilePath;

    }

    exec(); // Enter event loop


}
