#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <qaudiodecoder.h>
#include "wavfilewriter.h"
#include <QSoundEffect>
#include <QTextStream>

QT_USE_NAMESPACE

class AudioDecoder : public QObject
{
    Q_OBJECT
public:
    AudioDecoder(bool isPlayback, bool isDelete);
    ~AudioDecoder() { }

    void setSourceFilename(const QString &fileName);
    void start();
    void stop();

    void setTargetFilename(const QString &fileName);

Q_SIGNALS:
    void done();

public slots:
    void bufferReady();
    void error(QAudioDecoder::Error error);
    void stateChanged(QAudioDecoder::State newState);
    void finished();

    void playbackStatusChanged();
    void playingChanged();

private slots:
    void updateProgress();

private:
    bool m_isPlayback;
    bool m_isDelete;
    QAudioDecoder m_decoder;
    QTextStream m_cout;

    QString m_targetFilename;
    WaveFileWriter m_fileWriter;
    QSoundEffect m_soundEffect;

    qreal m_progress;
};

#endif // AUDIODECODER_H
