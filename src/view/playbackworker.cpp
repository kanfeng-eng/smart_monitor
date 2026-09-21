#include "playbackworker.h"

#include <QFileInfo>
#include <QImage>
#include <QTimer>

#include <opencv2/imgproc.hpp>

PlaybackWorker::PlaybackWorker(QObject *parent)
    : QObject(parent),
      timer(new QTimer(this)),
      fps(25.0),
      speed(1.0),
      totalFrames(0)
{
    connect(timer, &QTimer::timeout, this, &PlaybackWorker::readNextFrame);
}

PlaybackWorker::~PlaybackWorker()
{
    shutdown();
}

void PlaybackWorker::openFile(const QString &filePath)
{
    stop();
    if (!QFileInfo::exists(filePath))
    {
        emit messageReady(QString("录像文件不存在：%1").arg(filePath), true);
        return;
    }
    if (!capture.open(filePath.toStdString()))
    {
        emit messageReady(QString("录像文件无法解码或已损坏：%1")
                              .arg(QFileInfo(filePath).fileName()), true);
        return;
    }

    fps = capture.get(cv::CAP_PROP_FPS);
    if (fps <= 0.0 || fps > 240.0)
        fps = 25.0;
    totalFrames = static_cast<qint64>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    emit mediaReady(fps, totalFrames);
    emit messageReady(QString("已载入：%1").arg(QFileInfo(filePath).fileName()), false);
}

void PlaybackWorker::play()
{
    if (!capture.isOpened())
        return;
    updateTimerInterval();
    timer->start();
    emit messageReady("正在播放", false);
}

void PlaybackWorker::pause()
{
    timer->stop();
    emit messageReady("已暂停", false);
}

void PlaybackWorker::resume()
{
    if (!capture.isOpened())
        return;
    updateTimerInterval();
    timer->start();
    emit messageReady("继续播放", false);
}

void PlaybackWorker::stop()
{
    timer->stop();
    if (capture.isOpened())
        capture.release();
    totalFrames = 0;
}

void PlaybackWorker::seek(qint64 frame)
{
    if (!capture.isOpened())
        return;
    capture.set(cv::CAP_PROP_POS_FRAMES, frame);
    renderCurrentFrame();
}

void PlaybackWorker::setSpeed(double newSpeed)
{
    speed = newSpeed > 0.0 ? newSpeed : 1.0;
    updateTimerInterval();
}

void PlaybackWorker::shutdown()
{
    stop();
}

void PlaybackWorker::readNextFrame()
{
    if (!capture.isOpened())
        return;

    cv::Mat frame;
    if (!capture.read(frame) || frame.empty())
    {
        timer->stop();
        emit playbackEnded();
        emit messageReady("录像播放结束", false);
        return;
    }

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    const QImage image(rgb.data, rgb.cols, rgb.rows,
                       static_cast<int>(rgb.step), QImage::Format_RGB888);
    emit frameReady(image.copy(),
                    static_cast<qint64>(capture.get(cv::CAP_PROP_POS_FRAMES)),
                    totalFrames);
}

void PlaybackWorker::updateTimerInterval()
{
    if (timer->isActive())
        timer->start(qMax(1, static_cast<int>(1000.0 / fps / speed)));
}

void PlaybackWorker::renderCurrentFrame()
{
    cv::Mat frame;
    if (!capture.read(frame) || frame.empty())
        return;

    const qint64 current = static_cast<qint64>(capture.get(cv::CAP_PROP_POS_FRAMES));
    capture.set(cv::CAP_PROP_POS_FRAMES, qMax<qint64>(0, current - 1));
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    const QImage image(rgb.data, rgb.cols, rgb.rows,
                       static_cast<int>(rgb.step), QImage::Format_RGB888);
    emit frameReady(image.copy(), current, totalFrames);
}
