#include "camerawidget.h"
#include "cameraworker.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QStyle>
#include <QThread>
#include <QVBoxLayout>

CameraWidget::CameraWidget(int channelIndex,
                           const QString &channelName,
                           const QString &storagePath,
                           const QString &fallbackVideo,
                           QWidget *parent)
    : QFrame(parent),
      index(channelIndex),
      name(channelName),
      thread(new QThread(this)),
      worker(new CameraWorker(channelIndex, channelName, storagePath, fallbackVideo)),
      started(false)
{
    setObjectName("cameraCard");
    setProperty("selected", false);
    setMinimumSize(360, 240);
    setCursor(Qt::PointingHandCursor);

    nameLabel = new QLabel(QString("CH %1  %2").arg(index + 1, 2, 10, QChar('0')).arg(name), this);
    nameLabel->setObjectName("cameraName");
    stateLabel = new QLabel("正在连接", this);
    stateLabel->setObjectName("cameraState");
    recordingLabel = new QLabel("REC", this);
    recordingLabel->setObjectName("recordingBadge");
    recordingLabel->hide();
    videoLabel = new QLabel(this);
    videoLabel->setObjectName("videoSurface");
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QHBoxLayout *header = new QHBoxLayout;
    header->setContentsMargins(14, 10, 14, 8);
    header->addWidget(nameLabel);
    header->addStretch();
    header->addWidget(recordingLabel);
    header->addWidget(stateLabel);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addLayout(header);
    layout->addWidget(videoLabel, 1);

    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &CameraWorker::start);
    connect(worker, &CameraWorker::frameReady, this, &CameraWidget::updateFrame);
    connect(worker, &CameraWorker::stateChanged, this, &CameraWidget::updateState);
    connect(worker, &CameraWorker::recordingChanged, this, &CameraWidget::updateRecording);
    connect(worker, &CameraWorker::segmentFinished, this, &CameraWidget::segmentFinished);
    connect(worker, &CameraWorker::recordingError, this, &CameraWidget::recordingError);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
}

CameraWidget::~CameraWidget()
{
    stop();
}

int CameraWidget::channelIndex() const
{
    return index;
}

QString CameraWidget::channelName() const
{
    return name;
}

void CameraWidget::start()
{
    if (started)
        return;
    started = true;
    thread->start();
}

void CameraWidget::stop()
{
    if (!started)
        return;
    if (thread->isRunning())
        QMetaObject::invokeMethod(worker, "stop", Qt::BlockingQueuedConnection);
    thread->quit();
    thread->wait(3000);
    started = false;
}

void CameraWidget::setRecording(bool enabled)
{
    QMetaObject::invokeMethod(worker, "setRecording", Qt::QueuedConnection,
                              Q_ARG(bool, enabled));
}

void CameraWidget::setChannelName(const QString &channelName)
{
    name = channelName;
    nameLabel->setText(QString("CH %1  %2").arg(index + 1, 2, 10, QChar('0')).arg(name));
    QMetaObject::invokeMethod(worker, "setChannelName", Qt::QueuedConnection,
                              Q_ARG(QString, name));
}

void CameraWidget::setFallbackVideo(const QString &filePath)
{
    QMetaObject::invokeMethod(worker, "setFallbackVideo", Qt::QueuedConnection,
                              Q_ARG(QString, filePath));
}

void CameraWidget::setStoragePath(const QString &path)
{
    QMetaObject::invokeMethod(worker, "setStoragePath", Qt::QueuedConnection,
                              Q_ARG(QString, path));
}

void CameraWidget::setSelected(bool selected)
{
    setProperty("selected", selected);
    style()->unpolish(this);
    style()->polish(this);
}

void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    emit selected(index);
    QFrame::mousePressEvent(event);
}

void CameraWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    refreshPixmap();
}

void CameraWidget::updateFrame(const QImage &frame)
{
    lastFrame = frame;
    refreshPixmap();
}

void CameraWidget::updateState(bool connected, const QString &text)
{
    stateLabel->setText(QString("●  %1").arg(text));
    stateLabel->setProperty("online", connected);
    stateLabel->style()->unpolish(stateLabel);
    stateLabel->style()->polish(stateLabel);
}

void CameraWidget::updateRecording(bool recording)
{
    recordingLabel->setVisible(recording);
}

void CameraWidget::refreshPixmap()
{
    if (lastFrame.isNull() || videoLabel->size().isEmpty())
        return;
    videoLabel->setPixmap(QPixmap::fromImage(lastFrame).scaled(
        videoLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}
