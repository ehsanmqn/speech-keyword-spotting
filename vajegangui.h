#ifndef VAJEGANGUI_H
#define VAJEGANGUI_H

#include <QMainWindow>
#include <QString>
#include <QChart>
#include <QWidget>
#include <QLineSeries>
#include <xyseriesiodevice.h>
#include <QAudioInput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QSound>
#include <Classifier_KWS.h>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <qaudiolevel.h>
#include <playerscontrol.h>
#include <QAbstractItemView>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <playlistmodel.h>
#include <QStringListModel>
#include <QtConcurrent>
#include <QSqlDatabase>
#include <QTableWidgetItem>
#include <QPair>

class QAbstractItemView;
class QMediaPlayer;
class PlaylistModel;

using namespace QtCharts;
using namespace std;

namespace Ui {
class VajeganGUI;
}

class QAudioLevels;

struct kwsResult{
    float duration;
    int confidenceSize;
    float confidence[1000];
    float timeAligns[1000][2];
};

struct kwsResultThread{
    struct kwsResult res[100];
    int resCounter = 0;
};

struct kwsThreadInput{
    QUrl fileUrl;
    QString keyword;
    int sampleRate;
    double threshold;
    infra::matrix phonemeStates;
    infra::vector classifierWeigths;
    PhonemeClassifier phonemeClassifier;
};

struct searchResult{
    int fileIndex;
    QUrl filePath;
    float searchDuration;
    float startTime;
    float endTime;
    float confidence;
    QString keyword;
    QString date;
};

struct words{
    QString keyword;
    QString phoneme;
};

typedef struct  WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;

struct Media{
    QUrl url;
    QString name;
    wav_hdr header;
};

class VajeganGUI : public QMainWindow
{
    Q_OBJECT

public:
    int resultTableCounter, flag;
    QChart *m_chart;
    QLineSeries *m_series;
    XYSeriesIODevice *m_device;
    QAudioInput *m_audioInput;
    QMediaPlayer *player;
    QString audioFilePath;
    PlayerControls *controls;
    QMediaPlaylist *playlist;
    QStringListModel *model;
    QList<QUrl> urlList;
    QList<Media> mediaList;
    QFutureWatcher<KeywordClassifier> *futureWatcher;
    QList<searchResult> resultContainer;
    QList<kwsResult> kwsResultContainer;
    QList<words> wordList;
    int loadedUrlCounter;
    string keywordClassifierModelConfig;
    string phonemClassifierConfig;
    string phonmeModelsFileList;
    string phonemStatsConfig;
    string phonemeMapConfig;
    string featureExtractionConfig;
    string mfccStatsConfig;
    infra::matrix phonemeStates;
    infra::vector classifierWeigths;

    explicit VajeganGUI(QWidget *parent = 0);
    ~VajeganGUI();
    bool isPlayerAvailable() const;
    void addToPlaylist(const QList<QUrl> urls);
    static kwsResult scale(const kwsThreadInput &input);


private:
    Ui::VajeganGUI *ui;

    PlaylistModel *playlistModel;
    QAbstractItemView *playlistView;
    QString trackInfo;
    QString statusInfo;
    qint64 duration;
    QStringList supportedMimeTypes;
    QSqlDatabase db;
    QString repoPath;
    int keywordCounter;

    void handleCursor(QMediaPlayer::MediaStatus status);
    void setStatusInfo(const QString &info);
    void setTrackInfo(const QString &info);
    void readInputFile(QString fileUrl, int inoutType);
    void updateQuoteListView();
    void setQuoteTextView();
    void updateUserList();
    void sleepMs(int ms);
    void saveConfiguration();
    void loadConfiguration();
    wav_hdr getFileInformation(QString url);
    void hideWindow(QMainWindow *window);
    void loadClassifierConfigurationFiles();
    void loadPhonemeStats(string &filename);
    void loadKeywordClassifierWeigths(string &filename);

private slots:
    void onKeywordEnteredSlot();
    void onSearchButtonClickedSlot();
    void onRecordButtonClickedSlot();
    void onStopButtonClickedSlot();
    void durationChanged(qint64 duration);
    void updateDurationInfo(qint64 currentInfo);
    void positionChanged(qint64 progress);
    void playlistPositionChanged(int currentItem);
    void seek(int seconds);
    void onResultTableClicked(int row, int col);
    void onGuideButtonClicked();
    void statusChanged(QMediaPlayer::MediaStatus status);
    void displayErrorMessage();
    void metaDataChanged();
    void bufferingProgress(int progress);
    void audioAvailableChanged(bool available);
    void open();
    void openFolder();
    void jump(const QModelIndex &index);
    void previousClicked();
    void onConvertButtonClicked();
    void onPlaylistClearButtonClicked();
    void onReslistClearButtonClicked();
    void onAddToQuoteDatabseClicked();
    void onDeleteQuoteClicked();
    void onDeleteAllQuoteClicked();
    void onQuoteListWidgetClicked();
    void alternateAddButton();
    void addNewUser();
    void deleteUser();
//    void enableDeleteUserButton();
    void changeQuoteSize(int size);
    void onConfigurationChanged();
    void onItalicCheckboxToggled(bool state);
    void onBoldCheckboxToggled(bool state);
    void onThresholdSliderChanged(int value);
    void onArchiveButtonClicked();
    void updateArchiveTable();
    void onInstallFontButtonClicked();
    void onHelpPushButtonClicked();
    void updateDicTable();
    void addToDicTable();
    void onPhonemHelpButtonClicked();
    void loginSuccessful();
    void showKeywordlistContextMenu(const QPoint &pos);
    void onPlaylistItemDelete();
    void eraseWordlistItem();
    void eraseAllWordlistItems();
    void showPlaylistContextMenu(const QPoint &pos);
    void erasePlaylistItem();
    void eraseAllPlaylistItems();
    void onPlaylistTableWidgetDoubleClicked(int row, int col);
    void onAddQuoteLineEditTextChanged(QString text);
    void showQuoteslistContextMenu(const QPoint &pos);
    void EraseAllQuotelistItems();
    void EraseQuotelistItem();
    void showArchiveTableContextMenu(const QPoint &pos);
    void EraseArchivelistItem();
    void EraseAllArchivelistItems();
    void showUserlistContextMenu(const QPoint &pos);
    void showResultTableContextMenu(const QPoint &pos);
    void onSearchButtonClickedSlotMulti();
};

#endif // VAJEGANGUI_H
