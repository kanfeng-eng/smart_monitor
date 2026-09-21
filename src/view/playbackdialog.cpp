#include "playbackdialog.h"
#include "playbackworker.h"

#include "../model/videomodel.h"

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMetaObject>
#include <QPushButton>
#include <QPixmap>
#include <QSlider>
#include <QVBoxLayout>

PlaybackDialog::PlaybackDialog(const QString &storagePath, QWidget *parent)
    : QDialog(parent),
      storagePath(storagePath),
      videoModel(new VideoModel),
      fromEdit(nullptr),
      toEdit(nullptr),
      channelCombo(nullptr),
      segmentList(nullptr),
      frameLabel(nullptr),
      statusLabel(nullptr),
      progressSlider(nullptr),
      playButton(nullptr),
      pauseButton(nullptr),
      resumeButton(nullptr),
      stopButton(nullptr),
      speedCombo(nullptr),
      playbackThread(new QThread(this)),
      playbackWorker(new PlaybackWorker),
      fps(25.0),
      totalFrames(0),
      updatingSlider(false)
{
    setWindowTitle("录像检索与回放");
    resize(1120, 700);
    buildUi();
    playbackWorker->moveToThread(playbackThread);
    connect(playbackThread, &QThread::finished, playbackWorker, &QObject::deleteLater);
    connect(playbackWorker, &PlaybackWorker::frameReady,
            this, &PlaybackDialog::showFrame, Qt::QueuedConnection);
    connect(playbackWorker, &PlaybackWorker::mediaReady,
            this, &PlaybackDialog::showMedia, Qt::QueuedConnection);
    connect(playbackWorker, &PlaybackWorker::messageReady,
            this, &PlaybackDialog::showWorkerMessage, Qt::QueuedConnection);
    connect(playbackWorker, &PlaybackWorker::playbackEnded,
            this, &PlaybackDialog::playbackEnded, Qt::QueuedConnection);
    playbackThread->start();
    querySegments();
}

PlaybackDialog::~PlaybackDialog()
{
    stop();
    delete videoModel;
    if (playbackThread->isRunning())
    {
        QMetaObject::invokeMethod(playbackWorker, "shutdown", Qt::BlockingQueuedConnection);
        playbackThread->quit();
        playbackThread->wait();
    }
}

void PlaybackDialog::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    auto *filters = new QHBoxLayout;
    fromEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-7), this);
    toEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    fromEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    toEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    fromEdit->setCalendarPopup(true);
    toEdit->setCalendarPopup(true);
    channelCombo = new QComboBox(this);
    channelCombo->addItem("全部通道", 0);
    for (int channel = 1; channel <= 4; ++channel)
        channelCombo->addItem(QString("通道 %1").arg(channel), channel);
    auto *queryButton = new QPushButton("查询录像", this);
    auto *fileButton = new QPushButton("打开文件", this);
    auto *storageLabel = new QLabel(
        QString("录像目录：%1").arg(QDir::toNativeSeparators(storagePath)), this);
    storageLabel->setToolTip(QDir::toNativeSeparators(storagePath));
    filters->addWidget(new QLabel("开始"));
    filters->addWidget(fromEdit);
    filters->addWidget(new QLabel("结束"));
    filters->addWidget(toEdit);
    filters->addWidget(channelCombo);
    filters->addWidget(queryButton);
    filters->addWidget(fileButton);
    filters->addStretch();
    root->addLayout(filters);
    root->addWidget(storageLabel);

    auto *content = new QHBoxLayout;
    segmentList = new QListWidget(this);
    segmentList->setMinimumWidth(390);
    content->addWidget(segmentList);

    auto *viewer = new QVBoxLayout;
    frameLabel = new QLabel("请选择录像后播放", this);
    frameLabel->setAlignment(Qt::AlignCenter);
    frameLabel->setMinimumSize(640, 390);
    frameLabel->setStyleSheet("QLabel { background: #101b29; color: #8ea3bd; border-radius: 8px; }");
    viewer->addWidget(frameLabel, 1);

    progressSlider = new QSlider(Qt::Horizontal, this);
    progressSlider->setRange(0, 1000);
    viewer->addWidget(progressSlider);

    auto *controls = new QHBoxLayout;
    playButton = new QPushButton("播放", this);
    pauseButton = new QPushButton("暂停", this);
    resumeButton = new QPushButton("继续", this);
    stopButton = new QPushButton("停止", this);
    speedCombo = new QComboBox(this);
    speedCombo->addItem("1×", 1.0);
    speedCombo->addItem("2×", 2.0);
    speedCombo->addItem("4×", 4.0);
    speedCombo->setCurrentIndex(0);
    controls->addWidget(playButton);
    controls->addWidget(pauseButton);
    controls->addWidget(resumeButton);
    controls->addWidget(stopButton);
    controls->addSpacing(12);
    controls->addWidget(new QLabel("速度"));
    controls->addWidget(speedCombo);
    controls->addStretch();
    viewer->addLayout(controls);
    content->addLayout(viewer, 1);
    root->addLayout(content, 1);

    statusLabel = new QLabel("正在加载录像记录…", this);
    root->addWidget(statusLabel);

    connect(queryButton, &QPushButton::clicked, this, &PlaybackDialog::querySegments);
    connect(fileButton, &QPushButton::clicked, this, &PlaybackDialog::chooseFile);
    connect(playButton, &QPushButton::clicked, this, &PlaybackDialog::play);
    connect(pauseButton, &QPushButton::clicked, this, &PlaybackDialog::pause);
    connect(resumeButton, &QPushButton::clicked, this, &PlaybackDialog::resume);
    connect(stopButton, &QPushButton::clicked, this, &PlaybackDialog::stop);
    connect(speedCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &PlaybackDialog::setSpeed);
    connect(progressSlider, &QSlider::sliderMoved, this, &PlaybackDialog::seek);
}

void PlaybackDialog::querySegments()
{
    stop();
    QString error;
    const int channel = channelCombo->currentData().toInt();
    QVector<VideoSegment> segments = videoModel->findSegments(
        fromEdit->dateTime(), toEdit->dateTime(), channel, &error);
    showSegments(segments);
    if (!segments.isEmpty())
    {
        showMessage(QString("找到 %1 条录像记录").arg(segments.size()));
        return;
    }

    // 数据库暂时没有记录时，仍允许从本地录像目录检索，便于离线部署和历史文件回放。
    QDirIterator iterator(storagePath,
                          QStringList() << "*.avi" << "*.mp4" << "*.mov" << "*.mkv",
                          QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        const QString path = iterator.next();
        const QFileInfo info(path);
        if (info.lastModified() >= fromEdit->dateTime()
            && info.lastModified() <= toEdit->dateTime())
            appendFileSegment(path);
    }
    if (segmentList->count() > 0)
        showMessage(QString("数据库暂无记录，已列出本地目录中的 %1 个录像文件").arg(segmentList->count()));
    else if (!error.isEmpty())
        showMessage(QString("暂无录像记录：%1，可点击“打开文件”直接回放").arg(error), true);
    else
        showMessage("暂无符合条件的录像，可点击“打开文件”直接回放");
}

void PlaybackDialog::showSegments(const QVector<VideoSegment> &segments)
{
    segmentList->clear();
    for (const VideoSegment &segment : segments)
    {
        auto *item = new QListWidgetItem(
            QString("%1  |  %2  |  %3  |  %4")
                .arg(segment.startTime.toString("yyyy-MM-dd HH:mm:ss"),
                     segment.channelName.isEmpty() ? QString("通道 %1").arg(segment.channel)
                                                    : segment.channelName,
                     segment.eventType,
                     QFileInfo(segment.filePath).fileName()));
        item->setData(Qt::UserRole, segment.filePath);
        segmentList->addItem(item);
    }
}

void PlaybackDialog::appendFileSegment(const QString &filePath)
{
    const QFileInfo info(filePath);
    auto *item = new QListWidgetItem(
        QString("%1  |  本地文件  |  normal  |  %2")
            .arg(info.lastModified().toString("yyyy-MM-dd HH:mm:ss"), info.fileName()));
    item->setData(Qt::UserRole, filePath);
    segmentList->addItem(item);
}

void PlaybackDialog::chooseFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, "选择录像文件", storagePath,
        "录像文件 (*.avi *.mp4 *.mov *.mkv);;所有文件 (*.*)");
    if (filePath.isEmpty())
        return;
    appendFileSegment(filePath);
    segmentList->setCurrentRow(segmentList->count() - 1);
    openFile(filePath);
}

bool PlaybackDialog::openFile(const QString &filePath)
{
    if (!QFileInfo::exists(filePath))
    {
        showMessage(QString("录像文件不存在：%1").arg(filePath), true);
        return false;
    }
    QMetaObject::invokeMethod(playbackWorker, "openFile", Qt::QueuedConnection,
                              Q_ARG(QString, filePath));
    return true;
}

void PlaybackDialog::play()
{
    if (!segmentList->currentItem())
    {
        showMessage("请先选择一条录像记录。", true);
        return;
    }
    if (!openFile(segmentList->currentItem()->data(Qt::UserRole).toString()))
        return;
    QMetaObject::invokeMethod(playbackWorker, "play", Qt::QueuedConnection);
}

void PlaybackDialog::pause()
{
    QMetaObject::invokeMethod(playbackWorker, "pause", Qt::QueuedConnection);
}

void PlaybackDialog::resume()
{
    QMetaObject::invokeMethod(playbackWorker, "resume", Qt::QueuedConnection);
}

void PlaybackDialog::stop()
{
    QMetaObject::invokeMethod(playbackWorker, "stop", Qt::QueuedConnection);
    totalFrames = 0;
    if (progressSlider)
        progressSlider->setValue(0);
}

void PlaybackDialog::setSpeed(int index)
{
    const double speed = speedCombo->itemData(index).toDouble();
    QMetaObject::invokeMethod(playbackWorker, "setSpeed", Qt::QueuedConnection,
                              Q_ARG(double, speed));
}

void PlaybackDialog::seek(int value)
{
    if (updatingSlider || totalFrames <= 0)
        return;
    QMetaObject::invokeMethod(playbackWorker, "seek", Qt::QueuedConnection,
                              Q_ARG(qint64, static_cast<qint64>(value)));
}

void PlaybackDialog::showFrame(const QImage &frame, qint64 position, qint64 total)
{
    frameLabel->setPixmap(QPixmap::fromImage(frame).scaled(
        frameLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    if (total > 0)
    {
        updatingSlider = true;
        if (progressSlider->maximum() != static_cast<int>(qMin<qint64>(total, 1000000)))
            progressSlider->setRange(0, static_cast<int>(qMin<qint64>(total, 1000000)));
        progressSlider->setValue(static_cast<int>(qMin<qint64>(position, progressSlider->maximum())));
        updatingSlider = false;
    }
}

void PlaybackDialog::showMedia(double mediaFps, qint64 frames)
{
    fps = mediaFps;
    totalFrames = frames;
    progressSlider->setRange(0, totalFrames > 0
                                    ? static_cast<int>(qMin<qint64>(totalFrames, 1000000))
                                    : 1000);
}

void PlaybackDialog::showWorkerMessage(const QString &message, bool warning)
{
    showMessage(message, warning);
}

void PlaybackDialog::playbackEnded()
{
    showMessage("录像播放结束");
}

void PlaybackDialog::showMessage(const QString &message, bool warning)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(warning ? "color: #d66a6a;" : "color: #6b829e;");
}
