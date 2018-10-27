// Local header files
#include "vajegangui.h"
#include "vajegangui.h"
#include "vajegangui.h"
#include "ui_vajegangui.h"
#include "playerscontrol.h"
#include "playlistmodel.h"
#include "KWS.h"
#include "qdatejalali.h"
#include "login.h"

// Standard header files
#include <audiorecorder.h>
#include <QFileDialog>
#include <QMultimedia>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QSound>
#include <KWS.h>
#include <Classifier_KWS.h>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioInput>
#include <QtWidgets/QVBoxLayout>
#include <QMediaService>
#include <QMediaPlaylist>
#include <QMediaMetaData>
#include <QtWidgets>
#include <QMargins>
#include <time.h>
#include <QtConcurrent>
#include <QtWidgets>
#include <QThread>
#include <QThread>
#include <QDebug>
#include <QGuiApplication>
#include <qtconcurrentmap.h>
#include <thread>
#include <pthread.h>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <QGuiApplication>
#include <QDesktopServices>

//#define demo

using namespace QtConcurrent;
using namespace std;

static qreal getPeakValue(const QAudioFormat &format);
static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);

template <class T>
static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

VajeganGUI::VajeganGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VajeganGUI)
{
    ///======================================= Set Up =======================================

    ui->setupUi(this);

    QPixmap icon(":/icon/voice.png");
    this->setWindowIcon(QIcon(icon));

    // Initial database
    const QString DRIVER_("QSQLITE");
    db = QSqlDatabase::addDatabase(DRIVER_);

    // Set logo image
    QPixmap pix(":/loginimg/ir-logo.png");
    ui->logoLabel->setPixmap(pix.scaled(96, 96));

    QPixmap ava(":/loginimg/leader.png");
    ui->rightLabel->setPixmap(ava.scaled(96, 96));

    // Make repository
    repoPath = QDir::homePath() + "/Vazheyab";
    if(!QDir(repoPath).exists())
        QDir().mkdir(repoPath);

    // Load configuration
    loadConfiguration();

    ///======================================= Tab 1 =======================================

    // initial table counter
    resultTableCounter = 0;

    // initial url counter
    loadedUrlCounter = 0;

    // initial keywordCounter
    keywordCounter = 0;

    // Setup player
    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);

    // Setup playlist
    playlistModel = new PlaylistModel(this);
    playlistModel->setPlaylist(playlist);

    //    ui->playlistView->setModel(playlistModel);
    //    ui->playlistView->setCurrentIndex(playlistModel->index(playlist->currentIndex(), 0));

    ui->playlistTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QHeaderView* header = ui->playlistTableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    // Setup player controls
    controls = new PlayerControls(this, ui->mediaLayout);
    controls->setState(player->state());
    controls->setVolume(player->volume());
    controls->setMuted(controls->isMuted());
    controls->setButtonsEnabled(false);

    ui->slider->setRange(0, player->duration() / 1000);

    if (!isPlayerAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));

        controls->setEnabled(false);
        playlistView->setEnabled(false);
    }

    metaDataChanged();

    // Setup word list
    ui->wordlistTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    header = ui->wordlistTableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    ///======================================= Tab 2 =======================================

    // Setup result table
    header = ui->resultTable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    ui->resultTable->setContextMenuPolicy(Qt::CustomContextMenu);

    ///======================================= Tab 3 =======================================

    // Setup archive table
    header = ui->archiveTable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    ui->archiveTable->setContextMenuPolicy(Qt::CustomContextMenu);

    ///======================================= Tab 4 =======================================

    flag = 3;

    header = ui->userTableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    ui->userTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    //    ui->delQuoteButton->setEnabled(false);
    ui->quoteListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    updateUserList();
    updateQuoteListView();
    setQuoteTextView();

    ///======================================= Tab 5 =======================================

    header = ui->dicTable->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    ///======================================= Tab 1 Connections =======================================

    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(onSearchButtonClickedSlotMulti()));
    connect(ui->convertButton, SIGNAL(clicked(bool)), this, SLOT(onConvertButtonClicked()));
    //    connect(ui->playlistClearButton, SIGNAL(clicked(bool)), this, SLOT(onPlaylistClearButtonClicked()));
    //    connect(ui->reslistClearButton, SIGNAL(clicked(bool)), this, SLOT(onReslistClearButtonClicked()));
    connect(ui->delaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(onConfigurationChanged()));
    connect(ui->waitSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onConfigurationChanged()));
    connect(ui->thresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(onThresholdSliderChanged(int)));
    //    connect(ui->archiveButton, SIGNAL(clicked(bool)), this, SLOT(onArchiveButtonClicked()));
    connect(ui->phonemHelpButton, SIGNAL(clicked(bool)), this, SLOT(onPhonemHelpButtonClicked()));
    //    connect(ui->playlistItemDelete, SIGNAL(clicked(bool)), this, SLOT(onPlaylistItemDelete()));
    //    connect(ui->playlistView, SIGNAL(activated(QModelIndex)), this, SLOT(jump(QModelIndex)));
    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(open()));
    connect(ui->openFolderButton, SIGNAL(clicked(bool)), this, SLOT(openFolder()));

    // Player connections
    connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(playlist, SIGNAL(currentIndexChanged(int)), SLOT(playlistPositionChanged(int)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
    connect(player, SIGNAL(audioAvailableChanged(bool)), this, SLOT(audioAvailableChanged(bool)));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)),
            controls, SLOT(setState(QMediaPlayer::State)));
    connect(player, SIGNAL(volumeChanged(int)), controls, SLOT(setVolume(int)));
    connect(player, SIGNAL(mutedChanged(bool)), controls, SLOT(setMuted(bool)));
    connect(controls, SIGNAL(play()), player, SLOT(play()));
    connect(controls, SIGNAL(pause()), player, SLOT(pause()));
    connect(controls, SIGNAL(stop()), player, SLOT(stop()));
    connect(controls, SIGNAL(next()), playlist, SLOT(next()));
    connect(controls, SIGNAL(previous()), this, SLOT(previousClicked()));
    connect(controls, SIGNAL(changeVolume(int)), player, SLOT(setVolume(int)));
    connect(controls, SIGNAL(changeMuting(bool)), player, SLOT(setMuted(bool)));
    connect(controls, SIGNAL(changeRate(qreal)), player, SLOT(setPlaybackRate(qreal)));

    // Slider connections
    connect(ui->slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));

    // Playlist connections
    connect(ui->playlistTableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlaylistContextMenu(QPoint)));
    connect(ui->playlistTableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(onPlaylistTableWidgetDoubleClicked(int,int)));

    // Word list connections
    connect(ui->wordlistTableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showKeywordlistContextMenu(QPoint)));

    ///======================================= Tab 2 Connections =======================================

    connect(ui->resultTable, SIGNAL(cellClicked(int,int)), this, SLOT(onResultTableClicked(int,int)));
    connect(ui->resultTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showResultTableContextMenu(QPoint)));

    ///======================================= Tab 3 Connections =======================================

    connect(ui->searchArchiveButton, SIGNAL(clicked(bool)), this, SLOT(updateArchiveTable()));
    connect(ui->archiveTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showArchiveTableContextMenu(QPoint)));

    ///======================================= Tab 4 Connections =======================================

    connect(ui->addQuoteButton, SIGNAL(clicked(bool)), this, SLOT(onAddToQuoteDatabseClicked()));
    //    connect(ui->delQuoteButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteQuoteClicked()));
    //    connect(ui->delAllQuoteButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteAllQuoteClicked()));
    connect(ui->quoteListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onQuoteListWidgetClicked()));
    connect(ui->newPassLineEdit, SIGNAL(textChanged(QString)), this, SLOT(alternateAddButton()));
    connect(ui->newUserLineEdit, SIGNAL(textChanged(QString)), this, SLOT(alternateAddButton()));
    connect(ui->addUserButton, SIGNAL(clicked(bool)), this, SLOT(addNewUser()));
    //    connect(ui->userTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(enableDeleteUserButton()));
    //    connect(ui->deleteUserButton, SIGNAL(clicked(bool)), this, SLOT(deleteUser()));
    connect(ui->quoteSizeSpinner, SIGNAL(valueChanged(int)), this, SLOT(changeQuoteSize(int)));
    connect(ui->italicCheckBox, SIGNAL(toggled(bool)), this, SLOT(onItalicCheckboxToggled(bool)));
    connect(ui->boldCheckBox, SIGNAL(toggled(bool)), this, SLOT(onBoldCheckboxToggled(bool)));
    connect(ui->installFontButton, SIGNAL(clicked(bool)), this, SLOT(onInstallFontButtonClicked()));
    connect(ui->addQuotelineEdit, SIGNAL(textChanged(QString)), this, SLOT(onAddQuoteLineEditTextChanged(QString)));
    connect(ui->quoteListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showQuoteslistContextMenu(QPoint)));

    connect(ui->userTableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showUserlistContextMenu(QPoint)));

    ///======================================= Tab 5 Connections =======================================

    connect(ui->updateDicTableButton, SIGNAL(clicked(bool)), this, SLOT(updateDicTable()));
    connect(ui->addToDicButton, SIGNAL(clicked(bool)), this, SLOT(addToDicTable()));
    connect(ui->tab3HelpButton, SIGNAL(clicked(bool)), this, SLOT(onPhonemHelpButtonClicked()));

    ///======================================= Tab 6 Connections =======================================

    connect(ui->helpPushButton, SIGNAL(clicked(bool)), this, SLOT(onHelpPushButtonClicked()));


    ///======================================= Login configuration =====================================

    //    this->setEnabled(false);
    //    login *dlg = new login(this);
    //    dlg->show();

    //    connect(dlg, SIGNAL(success(bool)), this, SLOT(loginSuccessful()));
    //    connect(dlg, SIGNAL(finished(int)), this, SLOT(close()));
    //    connect(dlg, SIGNAL(closing(bool)), this, SLOT(close()));
}

// +=========================================== Player related functions =========================================

void VajeganGUI::loginSuccessful()
{
    this->setEnabled(true);

    // Initial database
    const QString DRIVER("QSQLITE");
    db = QSqlDatabase::addDatabase(DRIVER);
}

bool VajeganGUI::isPlayerAvailable() const
{
    return player->isAvailable();
}

static bool isPlaylist(const QUrl &url) // Check for ".wav" playlists.
{
    if (!url.isLocalFile())
        return false;
    const QFileInfo fileInfo(url.toLocalFile());
    return fileInfo.exists() && fileInfo.suffix().compare(QLatin1String("wav"), Qt::CaseInsensitive);
}

void VajeganGUI::addToPlaylist(const QList<QUrl> urls)
{
    QTableWidgetItem *item;
    foreach (const QUrl &url, urls) {
        if (isPlaylist(url))
        {
            playlist->load(url);
            urlList.append(url);
        }
        else
        {
            playlist->addMedia(url);
            urlList.append(url);
        }
    }
}

void VajeganGUI::previousClicked()
{
    // Go to previous track if we are within the first 5 seconds of playback
    // Otherwise, seek to the beginning.
    if(player->position() <= 5000)
        playlist->previous();
    else
        player->setPosition(0);
}

void VajeganGUI::onPlaylistClearButtonClicked()
{
    playlist->clear();
    urlList.clear();

    loadedUrlCounter = 0;
}

void VajeganGUI::jump(const QModelIndex &index)
{
    if (index.isValid()) {
        //        player->stop();
        //        playlist->setCurrentIndex(index.row() + 1);
        player->setMedia(urlList[index.row()]);
        controls->setButtonsEnabled(true);
        player->play();
    }
}

void VajeganGUI::onRecordButtonClickedSlot()
{
    AudioRecorder *recorder = new AudioRecorder(this);
    recorder->show();
}

void VajeganGUI::onStopButtonClickedSlot()
{
    player->stop();
}

void VajeganGUI::durationChanged(qint64 duration)
{
    this->duration = duration/1000;
    ui->slider->setMaximum(duration / 1000);
}

void VajeganGUI::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || duration) {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
        QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
        QString format = "mm:ss:ms";
        if (duration > 3600)
            format = "hh:mm:ss:ms";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    ui->labelDuration->setText(tStr);
}

void VajeganGUI::positionChanged(qint64 progress)
{

    if (!ui->slider->isSliderDown()) {
        ui->slider->setValue(progress / 1000);
    }
    updateDurationInfo(progress / 1000);
}

void VajeganGUI::playlistPositionChanged(int currentItem)
{
    playlistView->setCurrentIndex(playlistModel->index(currentItem, 0));
}

void VajeganGUI::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);

    // handle status message
    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Media Stalled"));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage();
        break;
    }
}

void VajeganGUI::metaDataChanged()
{
    if (player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                     .arg(player->metaData(QMediaMetaData::AlbumArtist).toString())
                     .arg(player->metaData(QMediaMetaData::Title).toString()));

        //        if (coverLabel) {
        //            QUrl url = player->metaData(QMediaMetaData::CoverArtUrlLarge).value<QUrl>();

        //            coverLabel->setPixmap(!url.isEmpty()
        //                                  ? QPixmap(url.toString())
        //                                  : QPixmap());
        //        }
    }
}

void VajeganGUI::bufferingProgress(int progress)
{
    setStatusInfo(tr("Buffering %4%").arg(progress));
}

void VajeganGUI::audioAvailableChanged(bool available)
{
}

void VajeganGUI::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if (status == QMediaPlayer::LoadingMedia ||
            status == QMediaPlayer::BufferingMedia ||
            status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void VajeganGUI::setTrackInfo(const QString &info)
{
    trackInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void VajeganGUI::setStatusInfo(const QString &info)
{
    statusInfo = info;
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
}

void VajeganGUI::displayErrorMessage()
{
    //    setStatusInfo(player->errorString());
}

void VajeganGUI::seek(int seconds)
{
    player->setPosition(seconds * 1000);
    emit positionChanged(seconds * 1000);
}

void VajeganGUI::onPlaylistItemDelete()
{
    //    int index = ui->playlistView->currentIndex().row();
    //    if(index >= 0)
    //        playlist->removeMedia(index);
    //    else
    //        QMessageBox::warning(
    //                    this,
    //                    tr("خطا"),
    //                    tr("موردی انتخاب نشده است"));
}

void VajeganGUI::showPlaylistContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->playlistTableWidget->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    QIcon mIcon;

    mIcon = QIcon(":/icon/add.png");
    myMenu.addAction(mIcon, "اضافه کردن فایل",  this, SLOT(open()));
    myMenu.addAction("اضافه کردن پوشه",  this, SLOT(openFolder()));
    myMenu.addSeparator();
    mIcon = QIcon(":/icon/delete.png");
    myMenu.addAction(mIcon, "حذف", this, SLOT(erasePlaylistItem()));
    myMenu.addAction("حذف همه",  this, SLOT(eraseAllPlaylistItems()));


    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void VajeganGUI::erasePlaylistItem()
{
    int index = ui->playlistTableWidget->currentRow();
    urlList.removeAt(index);
    ui->playlistTableWidget->removeRow(index);
    loadedUrlCounter--;
}

void VajeganGUI::eraseAllPlaylistItems()
{
    player->stop();
    player->setMedia(NULL);

    while(loadedUrlCounter >= 0)
    {
        ui->playlistTableWidget->removeRow(loadedUrlCounter--);
    }
    loadedUrlCounter = 0;
    urlList.clear();
}

void VajeganGUI::onPlaylistTableWidgetDoubleClicked(int row, int col)
{
    if(ui->playlistTableWidget->item(row, col))
    {
        controls->setButtonsEnabled(true);

        player->setMedia(urlList[row]);
        player->play();
    }
}

// +============================================ Wordlist functions ==============================================

void VajeganGUI::showKeywordlistContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->wordlistTableWidget->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    QIcon mIcon;

    mIcon = QIcon(":/icon/delete.png");
    myMenu.addAction(mIcon, "حذف", this, SLOT(eraseWordlistItem()));
    myMenu.addAction("حدف همه",  this, SLOT(eraseAllWordlistItems()));

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void VajeganGUI::eraseAllWordlistItems()
{
    for(; keywordCounter >= 0; keywordCounter--)
        ui->wordlistTableWidget->removeRow(keywordCounter);
    keywordCounter = 0;

    ui->searchButton->setEnabled(false);
}

void VajeganGUI::eraseWordlistItem()
{
    ui->wordlistTableWidget->removeRow(ui->wordlistTableWidget->currentRow());
    keywordCounter--;

    if(keywordCounter <= 0)
        ui->searchButton->setEnabled(false);
}

void VajeganGUI::onConvertButtonClicked()
{
#ifdef demo
    if(ui->wordlistTableWidget->rowCount() >= 1)
    {
        QMessageBox::information(this, tr("واژه یاب"),
                                 tr("امکان اضافه کردن بیش از یک کلمه در نسخه دمو وجود ندارد"),
                                 QMessageBox::Ok);

        return;
    }

#endif

    const QString DRIVER("QSQLITE");

    // connect to words
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("words");

        if(!db.open())
            qWarning() << "MainWindow::DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "MainWindow::DatabaseConnect - ERROR: no driver " << DRIVER << " available";

    QSqlQuery query("CREATE TABLE IF NOT EXISTS words (id INTEGER PRIMARY KEY, keyword varchar(250) NOT NULL, phoneme varchar(250) NOT NULL)");

    if(!query.isActive())
        qWarning() << "MainWindow::DatabaseInit - ERROR: " << query.lastError().text();

    query.prepare("SELECT phoneme FROM words WHERE keyword = ?");
    query.addBindValue(ui->persianWordLineEdit->text());

    if(!query.exec())
        qWarning() << "MainWindow::OnSearchClicked - ERROR: " << query.lastError().text();

    if(query.first())
    {
        ui->wordlistTableWidget->insertRow(keywordCounter);

        QTableWidgetItem* tableItem = new QTableWidgetItem();
        ui->wordlistTableWidget->setItem(keywordCounter,1,tableItem);
        ui->wordlistTableWidget->item(keywordCounter, 1)->setText(query.value(0).toString());
        ui->wordlistTableWidget->item(keywordCounter, 1)->setTextAlignment(Qt::AlignCenter);

        tableItem = new QTableWidgetItem();
        ui->wordlistTableWidget->setItem(keywordCounter,0,tableItem);
        ui->wordlistTableWidget->item(keywordCounter, 0)->setText(ui->persianWordLineEdit->text());
        ui->wordlistTableWidget->item(keywordCounter, 0)->setTextAlignment(Qt::AlignCenter);

        keywordCounter++;
    }
    else
    {
        QMessageBox::warning(
                    this,
                    tr("خطا"),
                    tr("کلمه یافت نشد"));
    }

    if(keywordCounter == 1)
        ui->searchButton->setEnabled(true);
}

void VajeganGUI::onKeywordEnteredSlot()
{
    ui->searchButton->setEnabled(true);
    ui->threshSpinBox->setEnabled(true);
}

// +======================================== Keyword spotter related functions ===================================

kwsResult runInThread(const kwsThreadInput &input)
{
    //    //    qDebug() << "in thread" << QThread::currentThreadId();
    //    kwsResult result;
    //    result.confidenceSize = 0;
    //    result.duration = 0;

    //    // Open and read the input file header
    //    FILE *fp_wav_in = NULL;
    //    fp_wav_in = fopen (input.fileUrl.toLocalFile().toStdString().c_str(), "rb");
    //    if (fp_wav_in == NULL)
    //    {
    //        qDebug() << "ERROR:   Could not open file '%s' !\r\n";
    //        return result;
    //    }

    //    int Header_Num_Bytes = 44;
    //    int wav_file_size;
    //    fseek(fp_wav_in, 0, SEEK_END);
    //    wav_file_size = (ftell(fp_wav_in) - Header_Num_Bytes) / sizeof(short);
    //    fseek(fp_wav_in, 0, SEEK_SET);

    //    char *Header;
    //    Header = new char[Header_Num_Bytes];
    //    if (Header_Num_Bytes != fread (Header, sizeof (char), Header_Num_Bytes, fp_wav_in))
    //    {
    //        qDebug() << "ERROR:   Invalid WAV header !\r\n";
    //        return result;
    //    }

    //    // In_Wav_Buff_Size is set to be 20 seconds
    //    int bufferSize, fixedBufferSize = input.sampleRate * 20;
    //    int remainedSamples = wav_file_size, overlapWindowSize;
    //    float inputData[fixedBufferSize];
    //    short inputDataShort;
    //    int resCounter = 0;

    //    // Read file till it becomes finish
    //    int iteration = 0;
    //    while (remainedSamples > 0)
    //    {
    //        if(iteration)
    //        {
    //            overlapWindowSize = input.sampleRate;

    //            // Fill first 1 second of inputData with last 1 second of previous data
    //            for(int i = 0; i < overlapWindowSize; i++)
    //                inputData[i] = inputData[bufferSize - input.sampleRate + i];
    //        }
    //        else
    //        {
    //            overlapWindowSize = 0;
    //            //            // Fill first 1 second of inputData by 0
    //            //            for(int i=0; i < overlapWindowSize; i++)
    //            //                inputData[i] = (float)0;
    //        }

    //        if (remainedSamples >= (fixedBufferSize - overlapWindowSize))
    //        {
    //            bufferSize = fixedBufferSize;
    //            remainedSamples -= (bufferSize - overlapWindowSize);
    //        }
    //        else
    //        {
    //            bufferSize = remainedSamples + overlapWindowSize;
    //            remainedSamples -= remainedSamples;
    //        }

    //        // Reading Wave samples from file
    //        for (int i = overlapWindowSize; i < bufferSize; i++)
    //        {
    //            if (fp_wav_in == NULL)
    //            {
    //                return result;
    //            }
    //            if (!fread (&inputDataShort, sizeof (short), 1, fp_wav_in)){
    //                return result;
    //            }
    //            inputData[i] = (float)inputDataShort;
    //        }

    //        // Run KWS on 20 sec of file
    //        Classifier_KWS *kws = new Classifier_KWS(input.keyword.length());
    //        float diff = Vajegan(inputData,
    //                             bufferSize,
    //                             input.keyword.toStdString(),
    //                             kws);

    //        result.duration += diff / CLOCKS_PER_SEC;
    //        result.confidenceSize += kws->confidence.size();
    //        for(int i = 0; i < kws->confidence.size(); i++, resCounter++)
    //        {
    //            result.confidence[resCounter] = kws->confidence[i];
    //            result.timeAligns[resCounter][0] = kws->time_aligns[i][0] / 100.0 + (iteration * 20 - iteration);
    //            result.timeAligns[resCounter][1] = kws->time_aligns[i][kws->time_aligns[i].size()-1] / 100.0 + (iteration * 20 - iteration);
    //        }

    //        delete kws;
    //        iteration++;
    //    }

    //    //    qDebug() << QThread::currentThreadId() << " finished";
    //    return result;
}

kwsResult VajeganGUI::scale(const kwsThreadInput &input)
{
    qDebug() << "in thread" << QThread::currentThreadId();
    kwsResult result;
    result.confidenceSize = 0;
    result.duration = 0;

    // Open and read the input file header
    FILE *fp_wav_in = NULL;
    fp_wav_in = fopen (input.fileUrl.toLocalFile().toStdString().c_str(), "rb");
    if (fp_wav_in == NULL)
    {
        qDebug() << "ERROR:   Could not open file '%s' !\r\n";
        return result;
    }

    int Header_Num_Bytes = 44;
    int wav_file_size;
    fseek(fp_wav_in, 0, SEEK_END);
    wav_file_size = (ftell(fp_wav_in) - Header_Num_Bytes) / sizeof(short);
    fseek(fp_wav_in, 0, SEEK_SET);

    char *Header;
    Header = new char[Header_Num_Bytes];
    if (Header_Num_Bytes != fread (Header, sizeof (char), Header_Num_Bytes, fp_wav_in))
    {
        qDebug() << "ERROR:   Invalid WAV header !\r\n";
        return result;
    }

    // In_Wav_Buff_Size is set to be 20 seconds
    int bufferSize, fixedBufferSize = input.sampleRate * 20;
    int remainedSamples = wav_file_size, overlapWindowSize;
    float inputData[fixedBufferSize];
    short inputDataShort;
    int resCounter = 0;

    // Read file till it becomes finish
    int iteration = 0;
    while (remainedSamples > 0)
    {
        if(iteration)
        {
            overlapWindowSize = input.sampleRate;

            // Fill first 1 second of inputData with last 1 second of previous data
            for(int i = 0; i < overlapWindowSize; i++)
                inputData[i] = inputData[bufferSize - input.sampleRate + i];
        }
        else
        {
            overlapWindowSize = 0;
            //            // Fill first 1 second of inputData by 0
            //            for(int i=0; i < overlapWindowSize; i++)
            //                inputData[i] = (float)0;
        }

        if (remainedSamples >= (fixedBufferSize - overlapWindowSize))
        {
            bufferSize = fixedBufferSize;
            remainedSamples -= (bufferSize - overlapWindowSize);
        }
        else
        {
            bufferSize = remainedSamples + overlapWindowSize;
            remainedSamples -= remainedSamples;
        }

        // Reading Wave samples from file
        for (int i = overlapWindowSize; i < bufferSize; i++)
        {
            if (fp_wav_in == NULL)
            {
                return result;
            }
            if (!fread (&inputDataShort, sizeof (short), 1, fp_wav_in)){
                return result;
            }
            inputData[i] = (float)inputDataShort;
        }

        // Keyword classifier initialization
        int minPhonemeLength, maxPhonemeLength;
        minPhonemeLength = 20;
        maxPhonemeLength = 330;

        KeywordClassifier *keywordClassifier;
        keywordClassifier = new KeywordClassifier(input.keyword.length());
        keywordClassifier->userDefinedThreshold = input.threshold;
        keywordClassifier->s_Step = 2;
        keywordClassifier->frameRate = 10;
        keywordClassifier->phi_size = 7;
        keywordClassifier->beta1 = 0.01;
        keywordClassifier->beta2 = 1.0;
        keywordClassifier->beta3 = 1.0;

        keywordClassifier->minNumberOfFrames = int(minPhonemeLength / double(keywordClassifier->frameRate));
        keywordClassifier->maxNumberOfFrames = int(maxPhonemeLength / double(keywordClassifier->frameRate));

        keywordClassifier->phonemeLengthMeanVector.resize(input.phonemeStates.width());
        keywordClassifier->phonemeLengthMeanVector = input.phonemeStates.row(0);
        keywordClassifier->phonemeLengthStdVector.resize(input.phonemeStates.width());
        keywordClassifier->phonemeLengthStdVector = input.phonemeStates.row(1);

        if (keywordClassifier->phonemeLengthMeanVector.size() != 30 || keywordClassifier->phonemeLengthStdVector.size() != 30) {
            std::cerr << "Error: number of phonemes loaded from phoneme stats is incorrect";
        }

        keywordClassifier->weigths.resize(keywordClassifier->phi_size);
        keywordClassifier->weigths.zeros();
        keywordClassifier->weigths = input.classifierWeigths;

        float diff = Vajegan(inputData,
                             bufferSize,
                             input.keyword.toStdString(),
                             keywordClassifier,
                             input.phonemeClassifier);

        result.duration += diff / CLOCKS_PER_SEC;
        result.confidenceSize += keywordClassifier->confidence.size();
        for(int i = 0; i < keywordClassifier->confidence.size(); i++, resCounter++)
        {
            result.confidence[resCounter] = keywordClassifier->confidence[i];
            result.timeAligns[resCounter][0] = keywordClassifier->timeAligns[i][0] / 100.0 + (iteration * 20 - iteration);
            result.timeAligns[resCounter][1] = keywordClassifier->timeAligns[i][keywordClassifier->timeAligns[i].size()-1] / 100.0 + (iteration * 20 - iteration);
        }

        delete keywordClassifier;
        iteration++;
    }

    qDebug() << "Thread" << QThread::currentThreadId() << "finished";
    return result;
}

void VajeganGUI::loadClassifierConfigurationFiles()
{
    // Loading keyword spotter necessary files
    char cwd[1024];

    getcwd(cwd, sizeof(cwd));

    if(ui->micRadioButton->isChecked())
    {
#ifdef Q_OS_LINUX
        // KWS model file path
        keywordClassifierModelConfig = strcat(cwd, "/KWS_EssentialFiles/db_formal/models/keywordClassifierModelConfig.model");

        // KWS model configuratio file path
        getcwd(cwd, sizeof(cwd));
        phonemClassifierConfig = strcat(cwd, "/KWS_EssentialFiles/phonemClassifierConfig");

        // Phonem GMMs file path
        getcwd(cwd, sizeof(cwd));
        phonmeModelsFileList = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/phonmeModelsFileListUnix.txt");

        // Phonem states file path
        getcwd(cwd, sizeof(cwd));
        phonemStatsConfig = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/phonemes_31.stats");

        // Phonems map file path
        getcwd(cwd, sizeof(cwd));
        phonemeMapConfig = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/phonemes_31");

        // Feature extractio configuration file path
        getcwd(cwd, sizeof(cwd));
        featureExtractionConfig = strcat(cwd, "/KWS_EssentialFiles/KWS_Feat_Config");

        // MFCC states file path
        getcwd(cwd, sizeof(cwd));
        mfccStatsConfig = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/mfcc_NoCMS.stats");


#else
        keywordClassifierModelConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\models\\keywordClassifierModelConfig.model");
        getcwd(cwd, sizeof(cwd));
        phonemClassifierConfig = strcat(cwd, "\\KWS_EssentialFiles\\phonemClassifierConfig");
        getcwd(cwd, sizeof(cwd));
        phonmeModelsFileList = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\phonmeModelsFileListWin.txt");
        getcwd(cwd, sizeof(cwd));
        phonemStatsConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\phonemes_31.stats");
        getcwd(cwd, sizeof(cwd));
        phonemeMapConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\phonemes_31");
        getcwd(cwd, sizeof(cwd));
        featureExtractionConfig = strcat(cwd, "\\KWS_EssentialFiles\\KWS_Feat_Config");
        getcwd(cwd, sizeof(cwd));
        mfccStatsConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\mfcc_NoCMS.stats");
#endif

    }
    else
    {
#ifdef Q_OS_LINUX
        // KWS model file path
        keywordClassifierModelConfig = strcat(cwd, "/KWS_EssentialFiles/db_informal/models/keywordClassifierModelConfig.model");

        // KWS model configuratio file path
        getcwd(cwd, sizeof(cwd));
        phonemClassifierConfig = strcat(cwd, "/KWS_EssentialFiles/phonemClassifierConfig");

        // Phonem GMMs file path
        getcwd(cwd, sizeof(cwd));
        phonmeModelsFileList = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/phonmeModelsFileListUnix.txt");

        // Phonem states file path
        getcwd(cwd, sizeof(cwd));
        phonemStatsConfig = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/phonemes.stats");

        // Phonems map file path
        getcwd(cwd, sizeof(cwd));
        phonemeMapConfig = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/phonemes");

        // Feature extractio configuration file path
        getcwd(cwd, sizeof(cwd));
        featureExtractionConfig = strcat(cwd, "/KWS_EssentialFiles/KWS_Feat_Config_tel");

        // MFCC states file path
        getcwd(cwd, sizeof(cwd));
        mfccStatsConfig = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/mfcc_NoCMS.stats");
#else
        // KWS model file path
        keywordClassifierModelConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\models\\keywordClassifierModelConfig.model");

        // KWS model configuratio file path
        getcwd(cwd, sizeof(cwd));
        phonemClassifierConfig = strcat(cwd, "\\KWS_EssentialFiles\\phonemClassifierConfig");

        // Phonem GMMs file path
        getcwd(cwd, sizeof(cwd));
        phonmeModelsFileList = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\phonmeModelsFileListWin.txt");

        // Phonem states file path
        getcwd(cwd, sizeof(cwd));
        phonemStatsConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\phonemes.stats");

        // Phonems map file path
        getcwd(cwd, sizeof(cwd));
        phonemeMapConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\phonemes");

        // Feature extractio configuration file path
        getcwd(cwd, sizeof(cwd));
        featureExtractionConfig = strcat(cwd, "\\KWS_EssentialFiles\\KWS_Feat_Config_tel");

        // MFCC states file path
        getcwd(cwd, sizeof(cwd));
        mfccStatsConfig = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\mfcc_NoCMS.stats");
#endif
    }
}

void VajeganGUI::loadPhonemeStats(string &filename)
{
    std::ifstream ifs(filename.c_str());
    if (!ifs.good()) {
        std::cerr << "Error: Unable to load phoneme stats from " << filename << std::endl;
        //getch();
        exit(-1);
    }

    infra::matrix tmp1(ifs);
    phonemeStates.resize(tmp1.height(), tmp1.width());
    phonemeStates = tmp1;

    ifs.close();
}

void VajeganGUI::loadKeywordClassifierWeigths(string &filename)
{
    classifierWeigths.resize(7);
    classifierWeigths.zeros();

    ifstream ifs;
    ifs.open(filename.c_str());
    if ( !ifs.good() ) {
        std::cerr << "Unable to open model file: " << filename << std::endl;
        //getch();
        exit(-1);
    }
    ifs >> classifierWeigths;

    ifs.close();
}

void VajeganGUI::onSearchButtonClickedSlotMulti()
{
    // Remove previous results from result table
    int tableSize = ui->resultTable->rowCount();
    if(tableSize > 0)
        while (tableSize >= 0)
            ui->resultTable->removeRow(tableSize--);

    if(urlList.size() > 0)
    {
        // Prepare date for history
        QDateJalali Jalali;
        QDateTime date =QDateTime::currentDateTime();
        QStringList shamsi=  Jalali.ToShamsi(  date.toString("yyyy"), date.toString("MM"),date.toString("dd"));
        QString JalailDate =shamsi.at(0)+"/"+shamsi.at(1)+"/"+shamsi.at(2);

        loadClassifierConfigurationFiles();
        loadPhonemeStats(phonemStatsConfig);
        loadKeywordClassifierWeigths(keywordClassifierModelConfig);

        // Declaration of phoneme classifier
        PhonemeClassifier phonemeClassifier;
        phonemeClassifier.loadPhonemeClassifier(phonmeModelsFileList);

        Initialize(keywordClassifierModelConfig,
                   phonemClassifierConfig,
                   ui->initialThresholdBox->value(),
                   2,
                   phonmeModelsFileList,
                   phonemStatsConfig,
                   phonemeMapConfig,
                   featureExtractionConfig,
                   mfccStatsConfig);

        // This part run a single thread for each file consecuently
        QProgressDialog dialog;
        dialog.setWindowTitle("واژه یاب");
        dialog.setWindowModality(Qt::WindowModal);
        dialog.setLabelText(QString("در حال جستجو"));


        QFutureWatcher<kwsResult> futureWatcher;
        QObject::connect(&futureWatcher, &QFutureWatcher<void>::finished, &dialog, &QProgressDialog::reset);
        QObject::connect(&dialog, &QProgressDialog::canceled, &futureWatcher, &QFutureWatcher<void>::cancel);
        //        QObject::connect(&futureWatcher,  &QFutureWatcher<void>::progressRangeChanged, &dialog, &QProgressDialog::setRange);
        //        QObject::connect(&futureWatcher, &QFutureWatcher<void>::progressValueChanged,  &dialog, &QProgressDialog::setValue);

        // Prepare keywords container
        QList<words> wordPairList;
        words temp;

        for(int i = 0; i < ui->wordlistTableWidget->rowCount(); i++)
        {
            temp.keyword = ui->wordlistTableWidget->item(i, 0)->text();
            temp.phoneme = ui->wordlistTableWidget->item(i, 1)->text();
            wordPairList.append(temp);
        }

        // Start processing for keywords
        searchResult mResult;

        // Make container clear
        resultContainer.clear();

        // Set progress bar limits
        int progressCounter = 0;
        int maxRange = wordPairList.size() * urlList.size();
        dialog.setValue((progressCounter++ / maxRange) * 100);

        QList<kwsThreadInput> processList;
        foreach (const words &word, wordPairList)
        {

            kwsThreadInput input;
            input.keyword = word.phoneme;

            if(ui->samplerate16->isChecked())
                input.sampleRate = 16000;
            else
                input.sampleRate = 8000;

            input.threshold = ui->initialThresholdBox->value();

            input.phonemeStates.resize(phonemeStates.height(), phonemeStates.width());
            input.phonemeStates = phonemeStates;

            input.classifierWeigths.resize(7);
            input.classifierWeigths.zeros();
            input.classifierWeigths = classifierWeigths;

            input.phonemeClassifier = phonemeClassifier;

            foreach (const QUrl &url, urlList)
            {
                input.fileUrl = url;
                processList.append(input);
            }

//            scale(input);
        }

        futureWatcher.setFuture(QtConcurrent::mapped(processList, scale));
        dialog.exec();
        futureWatcher.waitForFinished();

        dialog.setValue((progressCounter * 1.0 / maxRange) * 100);
        if(futureWatcher.isCanceled())
            return;

        int urlIndex = 0;
        QFutureIterator<kwsResult> futureIterator(futureWatcher.future());
        while(futureIterator.hasNext())
        {
            kwsResult res = futureIterator.next();

            if(res.confidenceSize > 0)
            {
                for(int j = 0; j < res.confidenceSize; j++)
                    if(res.confidence[j] >= ui->initialThresholdBox->minimum())
                    {
                        mResult.fileIndex = urlIndex + 1;
                        mResult.filePath = "url";
                        mResult.searchDuration = res.duration;
                        mResult.startTime = res.timeAligns[j][0];
                        mResult.endTime = res.timeAligns[j][1];
                        mResult.confidence = res.confidence[j];
                        mResult.keyword = temp.keyword;
                        mResult.date = JalailDate;
                        resultContainer.append(mResult);
                    }
            }

            urlIndex++;
        }


        //        ui->reslistClearButton->setEnabled(true);
        //        ui->archiveButton->setEnabled(true);

        QMessageBox msgBox;
        msgBox.setWindowTitle("واژه یاب");
        msgBox.setStyleSheet("QLabel{min-width: 300px;}");
        msgBox.setText("پایان جستجو");
        if(resultContainer.size())
        {
            msgBox.setInformativeText("تعداد نتیجه یافت شده: " + QString::number(resultContainer.size()) + "\nنتایج جستجو را در صفحه نتایج مشاهده فرمایید");

            // Insert search result into the result table
            QTableWidgetItem *tableItem;

            for(int j = 0, resultTableCounter = 0; j < resultContainer.size(); j++, resultTableCounter++)
            {
                mResult = resultContainer.at(j);
                if(mResult.confidence >= ui->initialThresholdBox->value())
                {
                    ui->resultTable->insertRow(resultTableCounter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter,0,tableItem);
                    ui->resultTable->item(resultTableCounter, 0)->setText(QString::number(mResult.fileIndex));
                    ui->resultTable->item(resultTableCounter, 0)->setTextAlignment(Qt::AlignCenter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter,1,tableItem);
                    ui->resultTable->item(resultTableCounter, 1)->setText(mResult.filePath.fileName());
                    ui->resultTable->item(resultTableCounter, 1)->setTextAlignment(Qt::AlignCenter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter, 2, tableItem);
                    ui->resultTable->item(resultTableCounter, 2)->setText(QString::number(mResult.searchDuration));
                    ui->resultTable->item(resultTableCounter, 2)->setTextAlignment(Qt::AlignCenter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter,3,tableItem);
                    ui->resultTable->item(resultTableCounter, 3)->setText(QString::number(mResult.startTime));
                    ui->resultTable->item(resultTableCounter, 3)->setTextAlignment(Qt::AlignCenter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter,4,tableItem);
                    ui->resultTable->item(resultTableCounter, 4)->setText(QString::number(mResult.endTime));
                    ui->resultTable->item(resultTableCounter, 4)->setTextAlignment(Qt::AlignCenter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter, 5,tableItem);
                    ui->resultTable->item(resultTableCounter, 5)->setText(QString::number(mResult.confidence));
                    ui->resultTable->item(resultTableCounter, 5)->setTextAlignment(Qt::AlignCenter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter, 6,tableItem);
                    ui->resultTable->item(resultTableCounter, 6)->setText(mResult.keyword);
                    ui->resultTable->item(resultTableCounter, 6)->setTextAlignment(Qt::AlignCenter);

                    tableItem = new QTableWidgetItem();
                    ui->resultTable->setItem(resultTableCounter, 7,tableItem);
                    ui->resultTable->item(resultTableCounter, 7)->setText(mResult.date);
                    ui->resultTable->item(resultTableCounter, 7)->setTextAlignment(Qt::AlignCenter);
                }
            }

            //            // Set result threshold box value
            //            ui->threshSpinBox->setValue(ui->initialThresholdBox->value());
            //            ui->thresholdSlider->setValue(ui->initialThresholdBox->value());
        }
        else
            msgBox.setInformativeText("تعداد نتیجه یافت شده: " + QString::number(resultContainer.size()));

        msgBox.exec();
    }
    else
    {
        QMessageBox::critical(
                    this,
                    tr("خطا"),
                    tr("لیست پخش خالی است"));
    }
}

void VajeganGUI::onSearchButtonClickedSlot()
{
    //    // Remove previous results from result table
    //    int tableSize = ui->resultTable->rowCount();
    //    if(tableSize > 0)
    //        while (tableSize >= 0)
    //            ui->resultTable->removeRow(tableSize--);

    //    if(urlList.size() > 0)
    //    {
    //        // Prepare date for history
    //        QDateJalali Jalali;
    //        QDateTime date =QDateTime::currentDateTime();
    //        QStringList shamsi=  Jalali.ToShamsi(  date.toString("yyyy"), date.toString("MM"),date.toString("dd"));
    //        QString JalailDate =shamsi.at(0)+"/"+shamsi.at(1)+"/"+shamsi.at(2);

    //        // Loading keyword spotter necessary files
    //        string KWS_Model_File_Name, KWS_Model_ConfigFile_Name, PhnClassi_Model_File_Name, PHN_StatsFile_Name, PHN_MapFile_Name, Feature_ConfigFile_Name, mfcc_stats_file_Name;
    //        char cwd[1024];

    //        getcwd(cwd, sizeof(cwd));

    //        if(ui->micRadioButton->isChecked())
    //        {
    //#ifdef Q_OS_LINUX
    //            // KWS model file path
    //            KWS_Model_File_Name = strcat(cwd, "/KWS_EssentialFiles/db_formal/models/disc_keyword_spotter.beta1_0.01.beta2_1.0.beta3_1.0.9759.model");

    //            // KWS model configuratio file path
    //            getcwd(cwd, sizeof(cwd));
    //            KWS_Model_ConfigFile_Name = strcat(cwd, "/KWS_EssentialFiles/KWS_Model_Config");

    //            // Phonem GMMs file path
    //            getcwd(cwd, sizeof(cwd));
    //            PhnClassi_Model_File_Name = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/GMMFileList30_linux.txt");

    //            // Phonem states file path
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_StatsFile_Name = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/phonemes_31.stats");

    //            // Phonems map file path
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_MapFile_Name = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/phonemes_31");

    //            // Feature extractio configuration file path
    //            getcwd(cwd, sizeof(cwd));
    //            Feature_ConfigFile_Name = strcat(cwd, "/KWS_EssentialFiles/KWS_Feat_Config");

    //            // MFCC states file path
    //            getcwd(cwd, sizeof(cwd));
    //            mfcc_stats_file_Name = strcat(cwd, "/KWS_EssentialFiles/db_formal/config/mfcc_NoCMS.stats");


    //#else
    //            KWS_Model_File_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\models\\disc_keyword_spotter.beta1_0.01.beta2_1.0.beta3_1.0.9759.model");
    //            getcwd(cwd, sizeof(cwd));
    //            KWS_Model_ConfigFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\KWS_Model_Config");
    //            getcwd(cwd, sizeof(cwd));
    //            PhnClassi_Model_File_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\GMMFileList30.txt");
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_StatsFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\phonemes_31.stats");
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_MapFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\phonemes_31");
    //            getcwd(cwd, sizeof(cwd));
    //            Feature_ConfigFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\KWS_Feat_Config");
    //            getcwd(cwd, sizeof(cwd));
    //            mfcc_stats_file_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_formal\\config\\mfcc_NoCMS.stats");
    //#endif

    //        }
    //        else
    //        {
    //#ifdef Q_OS_LINUX
    //            // KWS model file path
    //            KWS_Model_File_Name = strcat(cwd, "/KWS_EssentialFiles/db_informal/models/disc_keyword_spotter.beta1_0.01.beta2_1.0.beta3_1.0.96.model");

    //            // KWS model configuratio file path
    //            getcwd(cwd, sizeof(cwd));
    //            KWS_Model_ConfigFile_Name = strcat(cwd, "/KWS_EssentialFiles/KWS_Model_Config");

    //            // Phonem GMMs file path
    //            getcwd(cwd, sizeof(cwd));
    //            PhnClassi_Model_File_Name = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/GMMFileList_linux.txt");

    //            // Phonem states file path
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_StatsFile_Name = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/phonemes.stats");

    //            // Phonems map file path
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_MapFile_Name = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/phonemes");

    //            // Feature extractio configuration file path
    //            getcwd(cwd, sizeof(cwd));
    //            Feature_ConfigFile_Name = strcat(cwd, "/KWS_EssentialFiles/KWS_Feat_Config_tel");

    //            // MFCC states file path
    //            getcwd(cwd, sizeof(cwd));
    //            mfcc_stats_file_Name = strcat(cwd, "/KWS_EssentialFiles/db_informal/config/mfcc_NoCMS.stats");
    //#else
    //            // KWS model file path
    //            KWS_Model_File_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\models\\disc_keyword_spotter.beta1_0.01.beta2_1.0.beta3_1.0.96.model");

    //            // KWS model configuratio file path
    //            getcwd(cwd, sizeof(cwd));
    //            KWS_Model_ConfigFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\KWS_Model_Config");

    //            // Phonem GMMs file path
    //            getcwd(cwd, sizeof(cwd));
    //            PhnClassi_Model_File_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\GMMFileList.txt");

    //            // Phonem states file path
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_StatsFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\phonemes.stats");

    //            // Phonems map file path
    //            getcwd(cwd, sizeof(cwd));
    //            PHN_MapFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\phonemes");

    //            // Feature extractio configuration file path
    //            getcwd(cwd, sizeof(cwd));
    //            Feature_ConfigFile_Name = strcat(cwd, "\\KWS_EssentialFiles\\KWS_Feat_Config_tel");

    //            // MFCC states file path
    //            getcwd(cwd, sizeof(cwd));
    //            mfcc_stats_file_Name = strcat(cwd, "\\KWS_EssentialFiles\\db_informal\\config\\mfcc_NoCMS.stats");
    //#endif
    //        }

    //        // Keywordspotter initialization
    //        int KWS_Loop_Step = 2;

    //        Initialize(KWS_Model_File_Name,
    //                   KWS_Model_ConfigFile_Name,
    //                   ui->initialThresholdBox->value(),
    //                   KWS_Loop_Step,
    //                   PhnClassi_Model_File_Name,
    //                   PHN_StatsFile_Name,
    //                   PHN_MapFile_Name,
    //                   Feature_ConfigFile_Name,
    //                   mfcc_stats_file_Name);

    //        // This part run a single thread for each file consecuently
    //        QProgressDialog dialog;
    //        dialog.setWindowTitle("واژه یاب");
    //        dialog.setWindowModality(Qt::WindowModal);
    //        dialog.setLabelText(QString("در حال جستجو"));

    //        QFutureWatcher<kwsResult> futureWatcher;
    //        QObject::connect(&futureWatcher, &QFutureWatcher<void>::finished, &dialog, &QProgressDialog::reset);
    //        QObject::connect(&dialog, &QProgressDialog::canceled, &futureWatcher, &QFutureWatcher<void>::cancel);
    ////        QObject::connect(&futureWatcher,  &QFutureWatcher<void>::progressRangeChanged, &dialog, &QProgressDialog::setRange);
    ////        QObject::connect(&futureWatcher, &QFutureWatcher<void>::progressValueChanged,  &dialog, &QProgressDialog::setValue);

    //        // Prepare keywords container
    //        QList<words> wordPairList;
    //        words temp;

    //        for(int i = 0; i < ui->wordlistTableWidget->rowCount(); i++)
    //        {
    //            temp.keyword = ui->wordlistTableWidget->item(i, 0)->text();
    //            temp.phoneme = ui->wordlistTableWidget->item(i, 1)->text();
    //            wordPairList.append(temp);
    //        }

    //        // Start processing for keywords
    //        searchResult mResult;

    //        // Make container clear
    //        resultContainer.clear();

    //        // Set progress bar limits
    //        int progressCounter = 0;
    //        int maxRange = wordPairList.size() * urlList.size();
    //        dialog.setValue((progressCounter++ / maxRange) * 100);

    //        foreach (const words &temp, wordPairList)
    //        {

    //            kwsThreadInput input;
    //            input.keyword = temp.phoneme;

    //            if(ui->samplerate16->isChecked())
    //                input.sampleRate = 16000;
    //            else
    //                input.sampleRate = 8000;

    //            int urlIndex = 0;
    //            foreach (const QUrl &url, urlList)
    //            {
    //#ifdef demo
    //                if(urlIndex >= 1)
    //                    return;
    //#endif


    //                input.fileUrl = url;

    //                //                QFuture<kwsResult> f1 = run(runInThread, input);
    //                futureWatcher.setFuture(QtConcurrent::run(runInThread, input));
    //                dialog.exec();
    //                futureWatcher.waitForFinished();

    //                qDebug() << maxRange << " " << progressCounter ;
    //                dialog.setValue((progressCounter * 1.0 / maxRange) * 100);
    //                if(futureWatcher.isCanceled())
    //                    return;

    //                kwsResult res = futureWatcher.result();

    //                if(res.confidenceSize > 0)
    //                {
    //                    for(int j = 0; j < res.confidenceSize; j++)
    //                        if(res.confidence[j] >= ui->initialThresholdBox->minimum())
    //                        {
    //                            mResult.fileIndex = urlIndex + 1;
    //                            mResult.filePath = url;
    //                            mResult.searchDuration = res.duration;
    //                            mResult.startTime = res.timeAligns[j][0];
    //                            mResult.endTime = res.timeAligns[j][1];
    //                            mResult.confidence = res.confidence[j];
    //                            mResult.keyword = temp.keyword;
    //                            mResult.date = JalailDate;
    //                            resultContainer.append(mResult);
    //                        }
    //                }

    //                urlIndex++;
    //            }
    //        }

    //        //        ui->reslistClearButton->setEnabled(true);
    //        //        ui->archiveButton->setEnabled(true);

    //        QMessageBox msgBox;
    //        msgBox.setWindowTitle("واژه یاب");
    //        msgBox.setStyleSheet("QLabel{min-width: 300px;}");
    //        msgBox.setText("پایان جستجو");
    //        if(resultContainer.size())
    //        {
    //            msgBox.setInformativeText("تعداد نتیجه یافت شده: " + QString::number(resultContainer.size()) + "\nنتایج جستجو را در صفحه نتایج مشاهده فرمایید");

    //            // Insert search result into the result table
    //            QTableWidgetItem *tableItem;

    //            for(int j = 0, resultTableCounter = 0; j < resultContainer.size(); j++, resultTableCounter++)
    //            {
    //                mResult = resultContainer.at(j);
    //                if(mResult.confidence >= ui->initialThresholdBox->value())
    //                {
    //                    ui->resultTable->insertRow(resultTableCounter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter,0,tableItem);
    //                    ui->resultTable->item(resultTableCounter, 0)->setText(QString::number(mResult.fileIndex));
    //                    ui->resultTable->item(resultTableCounter, 0)->setTextAlignment(Qt::AlignCenter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter,1,tableItem);
    //                    ui->resultTable->item(resultTableCounter, 1)->setText(mResult.filePath.fileName());
    //                    ui->resultTable->item(resultTableCounter, 1)->setTextAlignment(Qt::AlignCenter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter, 2, tableItem);
    //                    ui->resultTable->item(resultTableCounter, 2)->setText(QString::number(mResult.searchDuration));
    //                    ui->resultTable->item(resultTableCounter, 2)->setTextAlignment(Qt::AlignCenter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter,3,tableItem);
    //                    ui->resultTable->item(resultTableCounter, 3)->setText(QString::number(mResult.startTime));
    //                    ui->resultTable->item(resultTableCounter, 3)->setTextAlignment(Qt::AlignCenter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter,4,tableItem);
    //                    ui->resultTable->item(resultTableCounter, 4)->setText(QString::number(mResult.endTime));
    //                    ui->resultTable->item(resultTableCounter, 4)->setTextAlignment(Qt::AlignCenter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter, 5,tableItem);
    //                    ui->resultTable->item(resultTableCounter, 5)->setText(QString::number(mResult.confidence));
    //                    ui->resultTable->item(resultTableCounter, 5)->setTextAlignment(Qt::AlignCenter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter, 6,tableItem);
    //                    ui->resultTable->item(resultTableCounter, 6)->setText(mResult.keyword);
    //                    ui->resultTable->item(resultTableCounter, 6)->setTextAlignment(Qt::AlignCenter);

    //                    tableItem = new QTableWidgetItem();
    //                    ui->resultTable->setItem(resultTableCounter, 7,tableItem);
    //                    ui->resultTable->item(resultTableCounter, 7)->setText(mResult.date);
    //                    ui->resultTable->item(resultTableCounter, 7)->setTextAlignment(Qt::AlignCenter);
    //                }
    //            }

    //            //            // Set result threshold box value
    //            //            ui->threshSpinBox->setValue(ui->initialThresholdBox->value());
    //            //            ui->thresholdSlider->setValue(ui->initialThresholdBox->value());
    //        }
    //        else
    //            msgBox.setInformativeText("تعداد نتیجه یافت شده: " + QString::number(resultContainer.size()));

    //        msgBox.exec();
    //    }
    //    else
    //    {
    //        QMessageBox::critical(
    //                    this,
    //                    tr("خطا"),
    //                    tr("لیست پخش خالی است"));
    //    }
}

void VajeganGUI::onThresholdSliderChanged(int value)
{
    ui->threshSpinBox->setValue(value * 1.0 / 100);

    if(resultContainer.size())
    {
        for(int rows = ui->resultTable->rowCount(); rows >= 0; rows--)
            ui->resultTable->removeRow(rows);

        resultTableCounter = 0;

        QTableWidgetItem *tableItem;
        for(int idx = 0; idx < resultContainer.size(); idx++)
        {
            if(resultContainer[idx].confidence >= ui->threshSpinBox->value())
            {
                ui->resultTable->insertRow(resultTableCounter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter,0,tableItem);
                ui->resultTable->item(resultTableCounter, 0)->setText(QString::number(resultContainer[idx].fileIndex));
                ui->resultTable->item(resultTableCounter, 0)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter,0,tableItem);
                ui->resultTable->item(resultTableCounter, 0)->setText(QString::number(resultContainer[idx].fileIndex));
                ui->resultTable->item(resultTableCounter, 0)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter,1,tableItem);
                ui->resultTable->item(resultTableCounter, 1)->setText(resultContainer[idx].filePath.fileName());
                ui->resultTable->item(resultTableCounter, 1)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter, 2, tableItem);
                ui->resultTable->item(resultTableCounter, 2)->setText(QString::number(resultContainer[idx].searchDuration));
                ui->resultTable->item(resultTableCounter, 2)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter,3,tableItem);
                ui->resultTable->item(resultTableCounter, 3)->setText(QString::number(resultContainer[idx].startTime));
                ui->resultTable->item(resultTableCounter, 3)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter,4,tableItem);
                ui->resultTable->item(resultTableCounter, 4)->setText(QString::number(resultContainer[idx].endTime));
                ui->resultTable->item(resultTableCounter, 4)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter, 5,tableItem);
                ui->resultTable->item(resultTableCounter, 5)->setText(QString::number(resultContainer[idx].confidence));
                ui->resultTable->item(resultTableCounter, 5)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter, 6,tableItem);
                ui->resultTable->item(resultTableCounter, 6)->setText(resultContainer[idx].keyword);
                ui->resultTable->item(resultTableCounter, 6)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->resultTable->setItem(resultTableCounter, 7,tableItem);
                ui->resultTable->item(resultTableCounter, 7)->setText(resultContainer[idx].date);
                ui->resultTable->item(resultTableCounter, 7)->setTextAlignment(Qt::AlignCenter);

                resultTableCounter++;
            }
        }
    }
}

void VajeganGUI::onResultTableClicked(int row, int col)
{
    if(ui->resultTable->item(row, col))
    {
        int delay = ui->delaySpinBox->value();
        float start = ui->resultTable->item(row, 3)->text().toFloat();
        if(start >= delay)
            start -= delay;
        else
            start = 0;

        controls->setButtonsEnabled(true);

        player->setMedia(urlList[ui->resultTable->item(row, 0)->text().toInt() - 1]);
        player->setPosition(start * 1000);
        emit positionChanged(start * 1000);
        player->play();
        //        qDebug() << "start: " << start << " | Stop: " << ui->waitSpinBox->value();
        sleepMs(ui->waitSpinBox->value() * 1000 + delay * 1000);
        player->stop();
    }
}

void VajeganGUI::onReslistClearButtonClicked()
{
    int rowCount = ui->resultTable->rowCount();
    for(;rowCount >= 0; rowCount--)
        ui->resultTable->removeRow(rowCount);
}

void VajeganGUI::onArchiveButtonClicked()
{
    if(ui->resultTable->rowCount())
    {
        const QString DRIVER("QSQLITE");

        // connect to quotes
        if(QSqlDatabase::isDriverAvailable(DRIVER))
        {
            db.setDatabaseName("archive");

            if(!db.open())
                qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
        }
        else
            qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


        QDateJalali Jalali;
        QDateTime date =QDateTime::currentDateTime();
        QStringList shamsi=  Jalali.ToShamsi(  date.toString("yyyy"), date.toString("MM"),date.toString("dd"));

        QSqlQuery query;
        bool operationSuccessful = 1;
        for(int i = 0; i < ui->resultTable->rowCount(); i++)
        {

            query.prepare("INSERT INTO archive (filename, keyword, confidence, day, month, year) VALUES (:filename, :keyword, :confidence, :day, :month, :year)");
            query.bindValue(":filename", ui->resultTable->item(i, 1)->text());
            query.bindValue(":keyword", ui->resultTable->item(i, 6)->text());
            query.bindValue(":confidence", ui->resultTable->item(i, 5)->text());
            query.bindValue(":day", shamsi.at(2));
            query.bindValue(":month", shamsi.at(1));
            query.bindValue(":year", shamsi.at(0));

            if(!query.exec())
            {
                operationSuccessful = 0;
                qDebug() << "Archive process - ERROR: " << query.lastError().text();
            }
        }

        if(operationSuccessful)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("واژه یاب");
            msgBox.setText("عملیات موفق");
            msgBox.exec();
        }
        //        ui->archiveButton->setEnabled(false);
    }
    else
    {
        QMessageBox::critical(
                    this,
                    tr("خطا"),
                    tr("لیست خالی است"));
    }
}

void VajeganGUI::showResultTableContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->resultTable->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    QIcon mIcon;

    //    mIcon = QIcon(":/icon/play.png");
    //    myMenu.addAction(mIcon, "پخش صدا",  this, SLOT(onResultTableClicked(ui->resultTable->currentRow(), 0)));
    mIcon = QIcon(":/icon/archive.png");
    myMenu.addAction(mIcon, "اضافه به آرشیو",  this, SLOT(onArchiveButtonClicked()));
    mIcon = QIcon(":/icon/delete.png");
    myMenu.addAction(mIcon, "پاک کردن لیست", this, SLOT(onReslistClearButtonClicked()));

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

// +=========================================== Quote related functions ==========================================

void VajeganGUI::updateQuoteListView()
{
    const QString DRIVER("QSQLITE");

    // connect to quotes
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("quotes");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


    QSqlQuery query;
    query.exec("SELECT * FROM quotes");
    while(query.next())
    {
        ui->quoteListWidget->addItem(query.value(1).toString());
    }
}

void VajeganGUI::setQuoteTextView()
{
    const QString DRIVER("QSQLITE");

    // connect to quotes
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("quotes");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


    QSqlQuery query;
    query.exec("SELECT COUNT(*) from quotes");
    query.first();

    if(query.value(0).toInt() > 0)
    {
        /* initialize random seed: */
        srand (time(NULL));

        /* Generate random number */
        int randomIdx = rand() % query.value(0).toInt();

        ui->quoteTextView->setText(ui->quoteListWidget->item(randomIdx)->text());
    }
}

void VajeganGUI::onAddToQuoteDatabseClicked()
{
    if(ui->addQuotelineEdit->text() != "")
    {
        const QString DRIVER("QSQLITE");

        // connect to quotes
        if(QSqlDatabase::isDriverAvailable(DRIVER))
        {
            db.setDatabaseName("quotes");

            if(!db.open())
                qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
        }
        else
            qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


        QSqlQuery query;
        query.prepare("INSERT INTO quotes (quote) VALUES (:quote)");
        query.bindValue(":quote", ui->addQuotelineEdit->text());

        if(!query.exec())
        {
            qDebug() << "Add quote - ERROR: " << query.lastError().text();
            int ret = QMessageBox::critical(this, tr("واژه یاب"),
                                            tr(" خطا در پایگاه داده\n"),
                                            QMessageBox::Ok);
        }
        else
        {
            ui->addQuotelineEdit->clear();
            ui->quoteListWidget->clear();
            this->updateQuoteListView();

            //            int ret = QMessageBox::information(this, tr("واژه یاب"),
            //                                           tr(" عملیات موفق \n"),
            //                                           QMessageBox::Ok);
        }
    }

}

void VajeganGUI::onDeleteAllQuoteClicked()
{
    const QString DRIVER("QSQLITE");

    // connect to quotes
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("quotes");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


    QSqlQuery query;
    query.exec("DELETE FROM quotes");

    if(!query.isActive())
        qWarning() << "Delete all - ERROR: " << query.lastError().text();
    else
    {
        ui->quoteListWidget->clear();
        this->updateQuoteListView();
    }
}

void VajeganGUI::onQuoteListWidgetClicked()
{
    //    ui->delQuoteButton->setEnabled(true);
}

void VajeganGUI::onDeleteQuoteClicked()
{
    const QString DRIVER("QSQLITE");

    // connect to quotes
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("quotes");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


    QSqlQuery query;
    query.prepare("DELETE FROM quotes WHERE quote = ?");
    query.addBindValue(ui->quoteListWidget->currentItem()->text());

    if(!query.exec())
        qWarning() << "Delete row - ERROR: " << query.lastError().text();
    else
    {
        ui->quoteListWidget->clear();
        this->updateQuoteListView();
    }
}

void VajeganGUI::changeQuoteSize(int size)
{
    QFont font = ui->quoteTextView->font();
    font.setPointSize(size);
    ui->quoteTextView->setFont(font);

    saveConfiguration();
}

void VajeganGUI::onItalicCheckboxToggled(bool state)
{
    QFont font(ui->quoteTextView->font());
    font.setItalic(state);
    ui->quoteTextView->setFont(font);

    saveConfiguration();
}

void VajeganGUI::onBoldCheckboxToggled(bool state)
{
    QFont font(ui->quoteTextView->font());
    font.setBold(state);
    ui->quoteTextView->setFont(font);

    saveConfiguration();
}

void VajeganGUI::onInstallFontButtonClicked()
{
    QString helpPath = qApp->applicationDirPath() + "/IRNazanin.ttf";
    QDesktopServices temp;
    temp.openUrl(QUrl(helpPath));
}

void VajeganGUI::onAddQuoteLineEditTextChanged(QString text)
{
    if(!text.isEmpty())
        ui->addQuoteButton->setEnabled(true);
    else
        ui->addQuoteButton->setEnabled(false);
}

void VajeganGUI::showQuoteslistContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->quoteListWidget->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    QIcon mIcon;

    mIcon = QIcon(":/icon/delete.png");
    myMenu.addAction(mIcon, "حذف", this, SLOT(EraseQuotelistItem()));
    myMenu.addAction("حذف همه",  this, SLOT(EraseAllQuotelistItems()));

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void VajeganGUI::EraseAllQuotelistItems()
{
    if( ui->quoteListWidget->currentIndex().row() >= 0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("واژه یاب");
        msgBox.setText("حذف همه موارد");
        msgBox.setStyleSheet("QLabel{min-width: 300px;}");
        msgBox.setInformativeText("آیا مایل به ادامه هستید؟");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Ok)
        {
            const QString DRIVER("QSQLITE");

            // connect to quotes
            if(QSqlDatabase::isDriverAvailable(DRIVER))
            {
                db.setDatabaseName("quotes");

                if(!db.open())
                    qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
            }
            else
                qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


            QSqlQuery query;
            query.exec("DELETE FROM quotes");

            if(!query.isActive())
            {
                qWarning() << "Delete all - ERROR: " << query.lastError().text();

                QMessageBox::critical(this, tr("واژه یاب"),
                                      tr("خطا در عملیات"),
                                      QMessageBox::Ok);
            }
            else
            {
                ui->quoteListWidget->clear();
                this->updateQuoteListView();

                QMessageBox::information(this, tr("واژه یاب"),
                                         tr("عملیات موفق"),
                                         QMessageBox::Ok);
            }
        }
    }
    else
    {
        QMessageBox::critical(this, tr("واژه یاب"),
                              tr("لیست خالی است"),
                              QMessageBox::Ok);
    }
}

void::VajeganGUI::EraseQuotelistItem()
{
    if(ui->quoteListWidget->currentRow() >= 0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("واژه یاب");
        msgBox.setText("حذف یک موارد");
        msgBox.setInformativeText("آیا مایل به ادامه هستید؟");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Ok)
        {
            const QString DRIVER("QSQLITE");

            // connect to quotes
            if(QSqlDatabase::isDriverAvailable(DRIVER))
            {
                db.setDatabaseName("quotes");

                if(!db.open())
                    qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
            }
            else
                qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


            QSqlQuery query;
            query.prepare("DELETE FROM quotes WHERE quote = ?");
            query.addBindValue(ui->quoteListWidget->currentItem()->text());

            if(!query.exec())
            {
                qWarning() << "Delete row - ERROR: " << query.lastError().text();
                QMessageBox::critical(this, tr("واژه یاب"),
                                      tr("خطا در عملیات"),
                                      QMessageBox::Ok);
            }
            else
            {
                ui->quoteListWidget->clear();
                this->updateQuoteListView();

                QMessageBox::information(this, tr("واژه یاب"),
                                         tr("عملیات موفق"),
                                         QMessageBox::Ok);
            }
        }
    }
    else
    {
        QMessageBox::critical(this, tr("واژه یاب"),
                              tr("موردی انتخاب نشده است"),
                              QMessageBox::Ok);
    }
}

// +===================================== User manipulation related functions ====================================

void VajeganGUI::updateUserList()
{
    // First remove all the rows
    for(int rows = ui->userTableWidget->rowCount(); rows >= 0; rows--)
        ui->userTableWidget->removeRow(rows);

    for(int row = 0; row < ui->userTableWidget->rowCount(); row++)
        if(ui->userTableWidget->item(row, 0))
        {
            ui->userTableWidget->item(row, 0)->setText("");
            ui->userTableWidget->item(row, 1)->setText("");
        }

    const QString DRIVER("QSQLITE");

    // connect to users
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("libinit");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


    int tableCounter = 0;
    QTableWidgetItem *tableItem;

    QSqlQuery query;
    query.exec("SELECT * FROM users");
    while(query.next())
    {

        ui->userTableWidget->insertRow(tableCounter);

        tableItem = new QTableWidgetItem();
        //        ui->userTableWidget->setColumnWidth(0, ui->userGroupBox->width() * 4);
        ui->userTableWidget->setItem(tableCounter,0,tableItem);
        ui->userTableWidget->item(tableCounter, 0)->setText(query.value(1).toString());
        ui->userTableWidget->item(tableCounter, 0)->setTextAlignment(Qt::AlignLeft);

        tableItem = new QTableWidgetItem();
        //        ui->userTableWidget->setColumnWidth(1, ui->userGroupBox->width() * 4);
        ui->userTableWidget->setItem(tableCounter,1,tableItem);
        ui->userTableWidget->item(tableCounter, 1)->setText(query.value(2).toString());
        ui->userTableWidget->item(tableCounter, 1)->setTextAlignment(Qt::AlignLeft);

        tableCounter++;
    }
}

void VajeganGUI::alternateAddButton()
{
    if(ui->newUserLineEdit->text() != "" && ui->newPassLineEdit->text() != "")
        ui->addUserButton->setEnabled(true);
    else
        ui->addUserButton->setEnabled(false);
}

void VajeganGUI::addNewUser()
{
    const QString DRIVER("QSQLITE");

    // connect to quotes
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("libinit");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


    int tableCounter = 0;
    QTableWidgetItem *tableItem;

    QSqlQuery query;
    query.prepare("SELECT username FROM users WHERE username = ?");
    query.addBindValue(ui->newUserLineEdit->text());

    if(!query.exec())
        qDebug() << "Add user line 1038 - ERROR: " << query.lastError().text();
    else {
        if(!query.last())
        {
            query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
            query.bindValue(":username", ui->newUserLineEdit->text());
            query.bindValue(":password", ui->newPassLineEdit->text());

            if(!query.exec())
                qDebug() << "Add user - ERROR: " << query.lastError().text();
            else
            {
                ui->newUserLineEdit->clear();
                ui->newPassLineEdit->clear();
                this->updateUserList();
            }
        }
        else
        {
            QMessageBox::information(
                        this,
                        tr("خطا در ورود اطلاعات"),
                        tr("کاربر تکراری"));
        }
    }

}

void VajeganGUI::deleteUser()
{
    int currentRow = ui->userTableWidget->currentIndex().row();

    if(currentRow >= 0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("واژه یاب");
        msgBox.setText("حذف کاربر");
        msgBox.setInformativeText("آیا مایل به ادامه هستید؟");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Ok)
        {
            QTableWidgetItem* item = ui->userTableWidget->currentItem();

            const QString DRIVER("QSQLITE");

            // connect to quotes
            if(QSqlDatabase::isDriverAvailable(DRIVER))
            {
                db.setDatabaseName("libinit");

                if(!db.open())
                    qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
            }
            else
                qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


            QSqlQuery query;
            query.prepare("DELETE FROM users WHERE username = ?");
            query.addBindValue(ui->userTableWidget->item(item->row(), 0)->text());

            if(!query.exec())
            {
                QMessageBox::critical(this, tr("واژه یاب"),
                                      tr("خطا در عملیات"),
                                      QMessageBox::Ok);
                qWarning() << "Delete row - ERROR: " << query.lastError().text();
            }
            else
            {
                ui->userTableWidget->removeRow(currentRow);
                this->updateUserList();
            }
        }
    }
    else
    {
        QMessageBox::critical(this, tr("واژه یاب"),
                              tr("موردی انتخاب نشده است"),
                              QMessageBox::Ok);
    }
}

void VajeganGUI::showUserlistContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->userTableWidget->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    QIcon mIcon;

    mIcon = QIcon(":/icon/delete.png");
    myMenu.addAction(mIcon, "حذف", this, SLOT(deleteUser()));
    //    myMenu.addAction("حذف همه",  this, SLOT(EraseAllQuotelistItems()));

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

// +=========================================== Tab 3 related functions ==========================================

void VajeganGUI::updateArchiveTable()
{
    QString year, month, day;

    QDateTime startDate = QDateTime::fromString(ui->startDayCombo->currentText() + "."
                                                + ui->startMonthCombo->currentText() + "."
                                                + ui->startYearCombo->currentText(), "dd.MM.yyyy");
    QDateTime endDate = QDateTime::fromString(ui->endDayCombo->currentText() + "."
                                              + ui->endMonthCombo->currentText() + "."
                                              + ui->endYearCombo->currentText(), "dd.MM.yyyy");

    const QString DRIVER("QSQLITE");

    // connect to archive
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("archive");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";


    int archiveTableCounter = 0;
    int tableSize = ui->archiveTable->rowCount();
    while(tableSize >= 0)
        ui->archiveTable->removeRow(tableSize--);

    QTableWidgetItem *tableItem;

    QSqlQuery query;
    query.exec("SELECT * FROM archive");

    while(query.next())
    {

        day = query.value(4).toString();
        if(day.length() < 2)
            day = "0" + day;

        month = query.value(5).toString();
        if(month.length() < 2)
            month = "0" + month;

        year = query.value(6).toString();

        QDateTime queryDate = QDateTime::fromString(day + "."
                                                    + month + "."
                                                    + year, "dd.MM.yyyy");

        if(queryDate.secsTo(endDate) >= 0 && queryDate.secsTo(startDate) <= 0)
        {
            if(ui->wordLineEdit->text() == "")
            {
                ui->archiveTable->insertRow(archiveTableCounter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter, 0, tableItem);
                ui->archiveTable->item(archiveTableCounter, 0)->setText(query.value(0).toString());
                ui->archiveTable->item(archiveTableCounter, 0)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter, 1, tableItem);
                ui->archiveTable->item(archiveTableCounter, 1)->setText(query.value(1).toString());
                ui->archiveTable->item(archiveTableCounter, 1)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter,2,tableItem);
                ui->archiveTable->item(archiveTableCounter, 2)->setText(query.value(2).toString());
                ui->archiveTable->item(archiveTableCounter, 2)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter,3,tableItem);
                ui->archiveTable->item(archiveTableCounter, 3)->setText(query.value(3).toString());
                ui->archiveTable->item(archiveTableCounter, 3)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter,4,tableItem);
                ui->archiveTable->item(archiveTableCounter, 4)->setText(query.value(6).toString() + "/" +
                                                                        query.value(5).toString() + "/" +
                                                                        query.value(4).toString());

                ui->archiveTable->item(archiveTableCounter, 4)->setTextAlignment(Qt::AlignCenter);

                archiveTableCounter++;

            }
            else if(query.value(2).toString() == ui->wordLineEdit->text())
            {
                ui->archiveTable->insertRow(archiveTableCounter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter, 0, tableItem);
                ui->archiveTable->item(archiveTableCounter, 0)->setText(query.value(1).toString());
                ui->archiveTable->item(archiveTableCounter, 0)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter,1,tableItem);
                ui->archiveTable->item(archiveTableCounter, 1)->setText(query.value(2).toString());
                ui->archiveTable->item(archiveTableCounter, 1)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter,2,tableItem);
                ui->archiveTable->item(archiveTableCounter, 2)->setText(query.value(3).toString());
                ui->archiveTable->item(archiveTableCounter, 2)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->archiveTable->setItem(archiveTableCounter,3,tableItem);
                ui->archiveTable->item(archiveTableCounter, 3)->setText(query.value(6).toString() + "/" +
                                                                        query.value(5).toString() + "/" +
                                                                        query.value(4).toString());

                ui->archiveTable->item(archiveTableCounter, 3)->setTextAlignment(Qt::AlignCenter);

                archiveTableCounter++;
            }
        }
    }
}

void VajeganGUI::showArchiveTableContextMenu(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->archiveTable->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;
    QIcon mIcon;

    mIcon = QIcon(":/icon/delete.png");
    myMenu.addAction(mIcon, "حذف", this, SLOT(EraseArchivelistItem()));
    myMenu.addAction("حذف همه",  this, SLOT(EraseAllArchivelistItems()));

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void VajeganGUI::EraseAllArchivelistItems()
{
    int currentRow = ui->archiveTable->currentIndex().row();
    int tableSize = ui->archiveTable->rowCount();

    if(currentRow >= 0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("واژه یاب");
        msgBox.setText("حذف همه موارد");
        msgBox.setInformativeText("آیا مایل به ادامه هستید؟");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Ok)
        {
            const QString DRIVER("QSQLITE");

            // connect to quotes
            if(QSqlDatabase::isDriverAvailable(DRIVER))
            {
                db.setDatabaseName("archive");

                if(!db.open())
                    qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
            }
            else
                qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";

            QSqlQuery query;
            qDebug() << tableSize;
            for(int i = 0; i < tableSize; i++)
            {
                query.prepare("DELETE FROM archive WHERE id = ?");
                query.addBindValue(ui->archiveTable->item(i, 0)->text());

                if(!query.exec())
                {
                    qWarning() << "Delete all - ERROR: " << query.lastError().text();
                    QMessageBox::critical(this, tr("واژه یاب"),
                                          tr("خطا در عملیات"),
                                          QMessageBox::Ok);
                }

                ui->archiveTable->removeRow(i);
            }

            QMessageBox::information(this, tr("واژه یاب"),
                                     tr("عملیات موفق"),
                                     QMessageBox::Ok);
        }
    }
    else
    {
        QMessageBox::critical(this, tr("واژه یاب"),
                              tr("لیست خالی است"),
                              QMessageBox::Ok);
    }
}

void VajeganGUI::EraseArchivelistItem()
{
    int currentRow = ui->archiveTable->currentIndex().row();
    if(currentRow >= 0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("واژه یاب");
        msgBox.setText("حذف یک مورد");
        msgBox.setInformativeText("آیا مایل به ادامه هستید؟");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Ok)
        {
            const QString DRIVER("QSQLITE");

            // connect to quotes
            if(QSqlDatabase::isDriverAvailable(DRIVER))
            {
                db.setDatabaseName("archive");

                if(!db.open())
                    qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
            }
            else
                qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";



            QSqlQuery query;
            query.prepare("DELETE FROM archive WHERE id = ?");
            query.addBindValue(ui->archiveTable->item(currentRow, 0)->text());

            if(!query.exec())
            {
                qWarning() << "Delete all - ERROR: " << query.lastError().text();
                QMessageBox::critical(this, tr("واژه یاب"),
                                      tr("خطا در عملیات"),
                                      QMessageBox::Ok);
            }
            else
            {
                ui->archiveTable->removeRow(currentRow);
                QMessageBox::information(this, tr("واژه یاب"),
                                         tr("عملیات موفق"),
                                         QMessageBox::Ok);
            }
        }
    }
    else
    {
        QMessageBox::critical(this, tr("واژه یاب"),
                              tr("لیست خالی است"),
                              QMessageBox::Ok);
    }
}

// +===================================== File manipulation related functions ====================================

wav_hdr VajeganGUI::getFileInformation(QString url)
{
    wav_hdr wavHeader;
    int headerSize = sizeof(wav_hdr);

    FILE* wavFile = fopen(url.toStdString().c_str(), "r");
    if (wavFile == nullptr)
    {
        qDebug() << "Unable to open wave file: " << url;
    }


    size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
    if (bytesRead > 0)
    {
        fclose(wavFile);
        return wavHeader;
    }
}

void VajeganGUI::open()
{
    QList<QUrl> urls;
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("انتخاب فایل"));
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    //    if (!supportedMimeTypes.isEmpty()) {
    //        supportedMimeTypes.append("audio/x-m3u"); // MP3 playlists
    //        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    //    }

    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
    {
        for(int cnt = 0; cnt < fileDialog.selectedUrls().size(); cnt++)
        {
#ifdef demo
            if(cnt >= 1)
            {
                QMessageBox::information(this, tr("واژه یاب"),
                                         tr("امکان اضافه کردن بیش از یک فایل در نسخه دمو وجود ندارد"),
                                         QMessageBox::Ok);
                break;
            }

            if(urlList.size() >= 1)
            {
                QMessageBox::information(this, tr("واژه یاب"),
                                         tr("امکان اضافه کردن بیش از یک فایل در نسخه دمو وجود ندارد"),
                                         QMessageBox::Ok);
                break;
            }
#endif
            QUrl newUrl = fileDialog.selectedUrls().at(cnt);

            // Following commented lines are used for copying to repository
            //            QFile::copy(fileDialog.selectedFiles().at(cnt), repoPath + url.fileName());
            //            QUrl newUrl("file:///" + repoPath + "/" + url.fileName());

            Media newMedia;
            newMedia.url = newUrl;
            newMedia.name = newUrl.fileName();
            newMedia.header = getFileInformation(newUrl.toLocalFile());
#ifdef demo
            int duration = (newMedia.header.ChunkSize / newMedia.header.SamplesPerSec) / (newMedia.header.bitsPerSample / 8) / newMedia.header.NumOfChan;
            if(duration > 20)
            {
                QMessageBox::information(this, tr("واژه یاب"),
                                         tr("نسخه دمو: تنها فایل های با طول کمتر از 20 ثانیه مجاز است"),
                                         QMessageBox::Ok);
                return;
            }
#endif
            qDebug() << "Line: 2112";
            ui->playlistTableWidget->insertRow(ui->playlistTableWidget->rowCount());

            QTableWidgetItem* tableItem = new QTableWidgetItem();
            ui->playlistTableWidget->setItem(loadedUrlCounter,0,tableItem);
            ui->playlistTableWidget->item(loadedUrlCounter, 0)->setText(newMedia.name);
            ui->playlistTableWidget->item(loadedUrlCounter, 0)->setTextAlignment(Qt::AlignLeft);

            tableItem = new QTableWidgetItem();
            ui->playlistTableWidget->setItem(loadedUrlCounter,1,tableItem);
            ui->playlistTableWidget->item(loadedUrlCounter, 1)->setText(QString::number((newMedia.header.ChunkSize / newMedia.header.SamplesPerSec) / (newMedia.header.bitsPerSample / 8) / newMedia.header.NumOfChan) + " s");
            ui->playlistTableWidget->item(loadedUrlCounter, 1)->setTextAlignment(Qt::AlignCenter);

            tableItem = new QTableWidgetItem();
            ui->playlistTableWidget->setItem(loadedUrlCounter,2,tableItem);
            ui->playlistTableWidget->item(loadedUrlCounter, 2)->setText(QString::number(newMedia.header.SamplesPerSec));
            ui->playlistTableWidget->item(loadedUrlCounter, 2)->setTextAlignment(Qt::AlignCenter);

            // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
            int audioFormatCode = newMedia.header.AudioFormat;
            QString audioFormat;
            switch(audioFormatCode)
            {
            case 1: audioFormat = "PCM"; break;
            case 6: audioFormat = "mulaw"; break;
            case 7: audioFormat = "alaw"; break;
            case 257: audioFormat = "IBM Mu-Law"; break;
            case 258: audioFormat = "IBM A-Law"; break;
            case 259: audioFormat = "ADPCM"; break;
            default: audioFormat = "Unsupported"; break;
            }

            tableItem = new QTableWidgetItem();
            ui->playlistTableWidget->setItem(loadedUrlCounter,3,tableItem);
            ui->playlistTableWidget->item(loadedUrlCounter, 3)->setText(audioFormat);
            ui->playlistTableWidget->item(loadedUrlCounter++, 3)->setTextAlignment(Qt::AlignCenter);

            urls.append(newUrl);
        }

        addToPlaylist(urls);
    }
}

void VajeganGUI::openFolder()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("انتخاب پوشه"));
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
    {
        QList<QUrl> list;
        for(int cnt = 0; cnt < fileDialog.selectedUrls().size(); cnt++)
        {
            QDirIterator it( fileDialog.selectedUrls().at(cnt).toString().mid(8), QDirIterator::Subdirectories);
            it.next();
            it.next();
            while (it.hasNext())
            {
#ifdef demo
                if(urlList.size() >= 1 || list.size() >= 1)
                {
                    QMessageBox::information(this, tr("واژه یاب"),
                                             tr("امکان اضافه کردن بیش از یک فایل در نسخه دمو وجود ندارد"),
                                             QMessageBox::Ok);

                    break;
                }

#endif

                QUrl newUrl("file:///" + it.next());

                // Following commented lines are used for copying to repository
                //                QFile::copy(fileDialog.selectedFiles().at(cnt), repoPath + "/" + url.fileName());
                //                QUrl newUrl("file:///" + repoPath + "/" + url.fileName());

                Media newMedia;
                newMedia.url = newUrl;
                newMedia.name = newUrl.fileName();
                newMedia.header = getFileInformation(newUrl.toLocalFile());
#ifdef demo
                int duration = (newMedia.header.ChunkSize / newMedia.header.SamplesPerSec) / (newMedia.header.bitsPerSample / 8) / newMedia.header.NumOfChan;
                if(duration > 20)
                {
                    QMessageBox::information(this, tr("واژه یاب"),
                                             tr("نسخه دمو: تنها فایل های با طول کمتر از 20 ثانیه مجاز است"),
                                             QMessageBox::Ok);
                    return;
                }
#endif
                ui->playlistTableWidget->insertRow(loadedUrlCounter);

                QTableWidgetItem* tableItem = new QTableWidgetItem();
                ui->playlistTableWidget->setItem(loadedUrlCounter,0,tableItem);
                ui->playlistTableWidget->item(loadedUrlCounter, 0)->setText(newMedia.name);
                ui->playlistTableWidget->item(loadedUrlCounter, 0)->setTextAlignment(Qt::AlignLeft);

                tableItem = new QTableWidgetItem();
                ui->playlistTableWidget->setItem(loadedUrlCounter,1,tableItem);
                ui->playlistTableWidget->item(loadedUrlCounter, 1)->setText(QString::number((newMedia.header.ChunkSize / newMedia.header.SamplesPerSec) / (newMedia.header.bitsPerSample / 8) / newMedia.header.NumOfChan) + " s");
                ui->playlistTableWidget->item(loadedUrlCounter, 1)->setTextAlignment(Qt::AlignCenter);

                tableItem = new QTableWidgetItem();
                ui->playlistTableWidget->setItem(loadedUrlCounter,2,tableItem);
                ui->playlistTableWidget->item(loadedUrlCounter, 2)->setText(QString::number(newMedia.header.SamplesPerSec));
                ui->playlistTableWidget->item(loadedUrlCounter, 2)->setTextAlignment(Qt::AlignCenter);

                // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
                int audioFormatCode = newMedia.header.AudioFormat;
                QString audioFormat;
                switch(audioFormatCode)
                {
                case 1: audioFormat = "PCM"; break;
                case 6: audioFormat = "mulaw"; break;
                case 7: audioFormat = "alaw"; break;
                case 257: audioFormat = "IBM Mu-Law"; break;
                case 258: audioFormat = "IBM A-Law"; break;
                case 259: audioFormat = "ADPCM"; break;
                default: audioFormat = "Unsupported"; break;
                }

                tableItem = new QTableWidgetItem();
                ui->playlistTableWidget->setItem(loadedUrlCounter,3,tableItem);
                ui->playlistTableWidget->item(loadedUrlCounter, 3)->setText(audioFormat);
                ui->playlistTableWidget->item(loadedUrlCounter++, 3)->setTextAlignment(Qt::AlignCenter);

                list.append(newUrl);
            }
        }

        addToPlaylist(list);
    }
}

// +============================================== Tab 4 functions ===============================================

void VajeganGUI::updateDicTable()
{
    int dicTableCounter = 0;
    ui->dicTable->clearContents();
    QTableWidgetItem *tableItem;

    const QString DRIVER("QSQLITE");

    // connect to words
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("words");

        if(!db.open())
            qWarning() << "MainWindow::DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "MainWindow::DatabaseConnect - ERROR: no driver " << DRIVER << " available";

    QSqlQuery query;
    query.exec("SELECT * FROM words");
    while(query.next())
    {
        ui->dicTable->insertRow(dicTableCounter);

        tableItem = new QTableWidgetItem();
        ui->dicTable->setItem(dicTableCounter, 0, tableItem);
        ui->dicTable->item(dicTableCounter, 0)->setText(query.value(1).toString());
        ui->dicTable->item(dicTableCounter, 0)->setTextAlignment(Qt::AlignCenter);

        tableItem = new QTableWidgetItem();
        ui->dicTable->setItem(dicTableCounter, 1, tableItem);
        ui->dicTable->item(dicTableCounter, 1)->setText(query.value(0).toString());
        ui->dicTable->item(dicTableCounter, 1)->setTextAlignment(Qt::AlignCenter);

        dicTableCounter++;
    }

}

void VajeganGUI::addToDicTable()
{
    if(ui->persianWord->text() != "" && ui->phonemWord->text() != "")
    {
        qDebug() << "Here!!!";
        const QString DRIVER("QSQLITE");

        // connect to words
        if(QSqlDatabase::isDriverAvailable(DRIVER))
        {
            db.setDatabaseName("words");

            if(!db.open())
                qWarning() << "MainWindow::DatabaseConnect - ERROR: " << db.lastError().text();
        }
        else
            qWarning() << "MainWindow::DatabaseConnect - ERROR: no driver " << DRIVER << " available";

        QSqlQuery query;
        query.prepare("INSERT INTO words (keyword, phonem) VALUES (:keyword, :phonem)");
        query.bindValue(":keyword", ui->persianWord->text());
        query.bindValue(":phonem", ui->phonemWord->text());

        if(!query.exec())
        {
            qDebug() << "Add keyword - ERROR: " << query.lastError().text();
            QMessageBox::information(
                        this,
                        tr("خطا"),
                        tr("خطا در ورود کلمه کلیدی"));
        }
        else
        {
            ui->persianWord->clear();
            ui->phonemWord->clear();
        }
    }
    else
    {
        qDebug() << "Not Here!!!";
        QMessageBox::information(
                    this,
                    tr("خطا"),
                    tr("اطلاعاتی وارد نشده است"));
    }
}

void VajeganGUI::onPhonemHelpButtonClicked()
{
    QString helpPath = qApp->applicationDirPath() + "/Help/manual_add_phonem.html";
    QDesktopServices temp;
    temp.openUrl(QUrl(helpPath));
}

// +============================================== Tab 5 functions ===============================================

void VajeganGUI::onHelpPushButtonClicked()
{
    QString helpPath = qApp->applicationDirPath() + "/Help/manual.html";
    QDesktopServices temp;
    temp.openUrl(QUrl(helpPath));
}

// +============================================== Other functions ===============================================

VajeganGUI::~VajeganGUI()
{
    delete ui;
}

void VajeganGUI::onGuideButtonClicked()
{
    QMessageBox::information(
                this,
                tr("راهنمای واجها"),
                tr("آ = a\t اَ = %\t اُ = o\t اِ = e\t ب = b\nپ = p\t ت = t\t س = s\t ج = j\t چ = c\n"
                   "ح = h\t خ = x\t د = d\t ر = r\t ش = $\nع = @\t ک = k\t گ = g\t ل = l\t م = m\n"
                   "ن = n\t و = v\t ی = y\t ق = q\t او = u\nف = f\t ز = z\t ای = i\t ژ = ;\n") );
}

void VajeganGUI::sleepMs(int ms) {
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

void VajeganGUI::saveConfiguration()
{
    FILE *file;
    file = fopen("vajegan.conf", "w");

    fprintf(file, "%d\n", ui->quoteSizeSpinner->text().toInt());
    fprintf(file, "%d\n", ui->boldCheckBox->isChecked());
    fprintf(file, "%d\n", ui->italicCheckBox->isChecked());
    fprintf(file, "%d\n", ui->delaySpinBox->value());
    fprintf(file, "%d\n", ui->waitSpinBox->value());

    fclose(file);
}

void VajeganGUI::loadConfiguration()
{
    FILE *file;
    file = fopen("vajegan.conf", "r");

    int val;

    fscanf(file, "%d", &val);
    ui->quoteSizeSpinner->setValue(val);

    fscanf(file, "%d", &val);
    ui->boldCheckBox->setChecked(val);
    QFont font(ui->quoteTextView->font());
    font.setBold(val);
    font.setFamily("IRNazanin");
    ui->quoteTextView->setFont(font);

    fscanf(file, "%d", &val);
    ui->italicCheckBox->setChecked(val);
    font.setItalic(val);
    ui->quoteTextView->setFont(font);

    fscanf(file, "%d", &val);
    ui->delaySpinBox->setValue(val);

    fscanf(file, "%d", &val);
    ui->waitSpinBox->setValue(val);

    fclose(file);
}

void VajeganGUI::onConfigurationChanged()
{
    saveConfiguration();
}





