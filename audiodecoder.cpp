#include "audiodecoder.h"
#include <iostream>

AudioDecoder::AudioDecoder(bool isPlayback, bool isDelete)
    : m_cout(stdout, QIODevice::WriteOnly)
{
    m_isPlayback = isPlayback;
    m_isDelete = isDelete;

    // Make sure the data we receive is in correct PCM format.
    // Our wav file writer only supports SignedInt sample type.
    QAudioFormat format;
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setSampleRate(16000);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::SignedInt);
    m_decoder.setAudioFormat(format);

    connect(&m_decoder, SIGNAL(bufferReady()), this, SLOT(bufferReady()));
    connect(&m_decoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(error(QAudioDecoder::Error)));
    connect(&m_decoder, SIGNAL(stateChanged(QAudioDecoder::State)), this, SLOT(stateChanged(QAudioDecoder::State)));
    connect(&m_decoder, SIGNAL(finished()), this, SLOT(finished()));
    connect(&m_decoder, SIGNAL(positionChanged(qint64)), this, SLOT(updateProgress()));
    connect(&m_decoder, SIGNAL(durationChanged(qint64)), this, SLOT(updateProgress()));

    connect(&m_soundEffect, SIGNAL(statusChanged()), this, SLOT(playbackStatusChanged()));
    connect(&m_soundEffect, SIGNAL(playingChanged()), this, SLOT(playingChanged()));

    m_progress = -1.0;
}

void AudioDecoder::setSourceFilename(const QString &fileName)
{
    m_decoder.setSourceFilename(fileName);
}

void AudioDecoder::start()
{
    m_decoder.start();
}

void AudioDecoder::stop()
{
    m_decoder.stop();
}

void AudioDecoder::setTargetFilename(const QString &fileName)
{
    m_targetFilename = fileName;
}

void AudioDecoder::bufferReady()
{
    // read a buffer from audio decoder
    QAudioBuffer buffer = m_decoder.read();
    if (!buffer.isValid())
        return;

    if (!m_fileWriter.isOpen() && !m_fileWriter.open(m_targetFilename, buffer.format())) {
        m_decoder.stop();
        return;
    }

    m_fileWriter.write(buffer);
}

void AudioDecoder::error(QAudioDecoder::Error error)
{
    switch (error) {
    case QAudioDecoder::NoError:
        return;
    case QAudioDecoder::ResourceError:
        m_cout << "Resource error" << endl;
        break;
    case QAudioDecoder::FormatError:
        m_cout << "Format error" << endl;
        break;
    case QAudioDecoder::AccessDeniedError:
        m_cout << "Access denied error" << endl;
        break;
    case QAudioDecoder::ServiceMissingError:
        m_cout << "Service missing error" << endl;
        break;
    }

    emit done();
}

void AudioDecoder::stateChanged(QAudioDecoder::State newState)
{
    switch (newState) {
    case QAudioDecoder::DecodingState:
        m_cout << "Decoding..." << endl;
        break;
    case QAudioDecoder::StoppedState:
        m_cout << "Decoding stopped" << endl;
        break;
    }
}

void AudioDecoder::finished()
{
    if (!m_fileWriter.close())
        m_cout << "Failed to finilize output file" << endl;

    m_cout << "Decoding finished" << endl;

    if (m_isPlayback) {
        m_cout << "Starting playback" << endl;
        m_soundEffect.setSource(QUrl::fromLocalFile(m_targetFilename));
        m_soundEffect.play();
    } else {
        emit done();
    }
}

void AudioDecoder::playbackStatusChanged()
{
    if (m_soundEffect.status() == QSoundEffect::Error) {
        m_cout << "Playback error" << endl;
        emit done();
    }
}

void AudioDecoder::playingChanged()
{
    if (!m_soundEffect.isPlaying()) {
        m_cout << "Playback finished" << endl;
        if (m_isDelete)
            QFile::remove(m_targetFilename);
        emit done();
    }
}

void AudioDecoder::updateProgress()
{
    qint64 position = m_decoder.position();
    qint64 duration = m_decoder.duration();
    qreal progress = m_progress;
    if (position >= 0 && duration > 0)
        progress = position / (qreal)duration;

    if (progress > m_progress + 0.1) {
        m_cout << "Decoding progress: " << (int)(progress * 100.0) << "%" << endl;
        m_progress = progress;
    }
}
