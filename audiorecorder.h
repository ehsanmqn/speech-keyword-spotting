#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QMainWindow>
#include <QMediaRecorder>
#include <QUrl>
#include <QAudioDecoder>
#include <wavfilewriter.h>

namespace Ui {
class AudioRecorder;
}

class QAudioRecorder;
class QAudioProbe;
class QAudioBuffer;
class QAudioLevel;

class AudioRecorder : public QMainWindow
{
    Q_OBJECT

public:
    explicit AudioRecorder(QWidget *parent = 0);
    ~AudioRecorder();


public slots:
    void processBuffer(const QAudioBuffer&);


private slots:
    void setOutputLocation();
    void togglePause();
    void toggleRecord();

    void updateStatus(QMediaRecorder::Status);
    void onStateChanged(QMediaRecorder::State);
    void updateProgress(qint64 pos);
    void displayErrorMessage();

private:
    Ui::AudioRecorder *ui;
    void clearAudioLevels();

    QAudioRecorder *audioRecorder;
    QAudioProbe *probe;
    QList<QAudioLevel*> audioLevels;
    bool outputLocationSet;
};

#endif // AUDIORECORDER_H
