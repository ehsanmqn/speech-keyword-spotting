#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDir>
#include <QFileDialog>
#include <QMediaRecorder>
#include <QAudioDecoder>

#include "qaudiolevel.h"
#include "audiorecorder.h"
#include "ui_audiorecorder.h"
#include "audiodecoder.h"

static qreal getPeakValue(const QAudioFormat &format);
static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);

template <class T>
static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

AudioRecorder::AudioRecorder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AudioRecorder)
{
    ui->setupUi(this);

    audioRecorder = new QAudioRecorder(this);
    probe = new QAudioProbe;
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(processBuffer(QAudioBuffer)));
    probe->setSource(audioRecorder);

    //audio devices
    //    ui->audioDeviceBox->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &device, audioRecorder->audioInputs()) {
        ui->audioDeviceBox->addItem(device, QVariant(device));
    }

    //audio codecs
    //    ui->audioCodecBox->addItem(tr("audio/PCM"), QVariant(QString("audio/PCM")));
    //    foreach (const QString &codecName, audioRecorder->supportedAudioCodecs()) {
    //        ui->audioCodecBox->addItem(codecName, QVariant(codecName));
    //    }

    //containers
    //    ui->containerBox->addItem(tr("wav"), QVariant(QString("wav")));
    //    foreach (const QString &containerName, audioRecorder->supportedContainers()) {
    //        ui->containerBox->addItem(containerName, QVariant(containerName));
    //    }

    //sample rate
    //    ui->sampleRateBox->addItem(tr("8000"), QVariant(8000));
    //    ui->sampleRateBox->addItem(tr("16000"), QVariant(16000));
    //    foreach (int sampleRate, audioRecorder->supportedAudioSampleRates()) {
    //        ui->sampleRateBox->addItem(QString::number(sampleRate), QVariant(
    //                sampleRate));
    //    }

    //channels
    //    ui->channelsBox->addItem(tr("Default"), QVariant(-1));
    //    ui->channelsBox->addItem(QStringLiteral("1"), QVariant(1));
    //    ui->channelsBox->addItem(QStringLiteral("2"), QVariant(2));
    //    ui->channelsBox->addItem(QStringLiteral("4"), QVariant(4));

    //quality
    //    ui->qualitySlider->setRange(0, int(QMultimedia::VeryHighQuality));
    //    ui->qualitySlider->setValue(int(QMultimedia::NormalQuality));

    //bitrates:
    //    ui->bitrateBox->addItem(tr("Default"), QVariant(0));
    //    ui->bitrateBox->addItem(QStringLiteral("16000"), QVariant(16000));
    //    ui->bitrateBox->addItem(QStringLiteral("64000"), QVariant(64000));
    //    ui->bitrateBox->addItem(QStringLiteral("96000"), QVariant(96000));
    //    ui->bitrateBox->addItem(QStringLiteral("128000"), QVariant(128000));

    connect(audioRecorder, SIGNAL(durationChanged(qint64)), this,
            SLOT(updateProgress(qint64)));
    connect(audioRecorder, SIGNAL(statusChanged(QMediaRecorder::Status)), this,
            SLOT(updateStatus(QMediaRecorder::Status)));
    connect(audioRecorder, SIGNAL(stateChanged(QMediaRecorder::State)),
            this, SLOT(onStateChanged(QMediaRecorder::State)));
    connect(audioRecorder, SIGNAL(error(QMediaRecorder::Error)), this,
            SLOT(displayErrorMessage()));

    connect(ui->recordButton, SIGNAL(clicked(bool)), this, SLOT(toggleRecord()));
    connect(ui->pauseButton, SIGNAL(clicked(bool)), this, SLOT(togglePause()));
    connect(ui->outputButton, SIGNAL(clicked(bool)), this, SLOT(setOutputLocation()));
}

AudioRecorder::~AudioRecorder()
{
    delete ui;
    delete audioRecorder;
    delete probe;
}

void AudioRecorder::updateProgress(qint64 duration)
{
    if (audioRecorder->error() != QMediaRecorder::NoError || duration < 2000)
        return;

    ui->statusbar->showMessage(tr("Recorded %1 sec").arg(duration / 1000));
}

void AudioRecorder::updateStatus(QMediaRecorder::Status status)
{
    QString statusMessage;

    switch (status) {
    case QMediaRecorder::RecordingStatus:
        statusMessage = tr("ذخیره سازی در %1").arg(audioRecorder->actualLocation().toString());
        break;
    case QMediaRecorder::PausedStatus:
        clearAudioLevels();
        statusMessage = tr("مکث");
        break;
    case QMediaRecorder::UnloadedStatus:
    case QMediaRecorder::LoadedStatus:
        clearAudioLevels();
        statusMessage = tr("توقف");
    default:
        break;
    }

    if (audioRecorder->error() == QMediaRecorder::NoError)
        ui->statusbar->showMessage(statusMessage);
}

void AudioRecorder::onStateChanged(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::RecordingState:
        ui->recordButton->setText(tr("توقف"));
        ui->pauseButton->setText(tr("مکث"));
        break;
    case QMediaRecorder::PausedState:
        ui->recordButton->setText(tr("توقف"));
        ui->pauseButton->setText(tr("ادامه"));
        break;
    case QMediaRecorder::StoppedState:
        ui->recordButton->setText(tr("ضبط"));
        ui->pauseButton->setText(tr("مکث"));
        break;
    }

    ui->pauseButton->setEnabled(audioRecorder->state() != QMediaRecorder::StoppedState);
}

static QVariant boxValue(const QComboBox *box)
{
    int idx = box->currentIndex();
    if (idx == -1)
        return QVariant();

    return box->itemData(idx);
}

void AudioRecorder::toggleRecord()
{
    if (audioRecorder->state() == QMediaRecorder::StoppedState) {
        audioRecorder->setAudioInput(boxValue(ui->audioDeviceBox).toString());

        QAudioEncoderSettings settings;
        settings.setCodec("audio/PCM");
        settings.setSampleRate(16000);
        settings.setBitRate(16);
        settings.setChannelCount(1);
        settings.setQuality(QMultimedia::HighQuality);
        settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
        //        QString container = boxValue(ui->containerBox).toString();

        audioRecorder->setOutputLocation(QUrl::fromLocalFile("ehsan.wav"));
        audioRecorder->setEncodingSettings(settings, QVideoEncoderSettings(), QString("wav"));
        audioRecorder->record();
    }
    else
    {
        audioRecorder->stop();

        bool isPlayback = false;
        bool isDelete = false;

        QFileInfo sourceFile;
        QFileInfo targetFile;

        int sourceFileIndex = (isPlayback || isDelete) ? 2 : 1;

        sourceFile.setFile("/media/veracrypt1/Vajegan/ehsan.wav");
        targetFile.setFile("/media/veracrypt1/Vajegan/out.wav");

        AudioDecoder decoder(isPlayback, isDelete);
//        QObject::connect(&decoder, SIGNAL(done()), &decoder, SLOT(quit()));
        decoder.setSourceFilename(sourceFile.absoluteFilePath());
        decoder.setTargetFilename(targetFile.absoluteFilePath());
        decoder.start();
    }
}

void AudioRecorder::togglePause()
{
    if (audioRecorder->state() != QMediaRecorder::PausedState)
        audioRecorder->pause();
    else
        audioRecorder->record();
}

void AudioRecorder::setOutputLocation()
{
    QString fileName = QFileDialog::getSaveFileName();
    audioRecorder->setOutputLocation(QUrl::fromLocalFile(fileName));
    outputLocationSet = true;
}

void AudioRecorder::displayErrorMessage()
{
    ui->statusbar->showMessage(audioRecorder->errorString());
}

void AudioRecorder::clearAudioLevels()
{
    for (int i = 0; i < audioLevels.size(); ++i)
        audioLevels.at(i)->setLevel(0);
}

// This function returns the maximum possible sample value for a given audio format
qreal getPeakValue(const QAudioFormat& format)
{
    // Note: Only the most common sample formats are supported
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // other sample formats are not supported
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    return qreal(0);
}

// returns the audio level for each channel
QVector<qreal> getBufferLevels(const QAudioBuffer& buffer)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;

    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] = qAbs(values.at(i) - peak_value / 2) / (peak_value / 2);
        break;
    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            values = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;
    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }

    return values;
}

template <class T>
QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}

void AudioRecorder::processBuffer(const QAudioBuffer& buffer)
{
    if (audioLevels.count() != buffer.format().channelCount()) {
        qDeleteAll(audioLevels);
        audioLevels.clear();
        for (int i = 0; i < buffer.format().channelCount(); ++i) {
            QAudioLevel *level = new QAudioLevel(ui->centralwidget);
            audioLevels.append(level);
            ui->levelsLayout->addWidget(level);
        }
    }

    QVector<qreal> levels = getBufferLevels(buffer);
    for (int i = 0; i < levels.count(); ++i)
        audioLevels.at(i)->setLevel(levels.at(i));
}
