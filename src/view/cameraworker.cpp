#include "cameraworker.h"

#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include <opencv2/imgproc.hpp>

CameraWorker::CameraWorker(int channelIndex,
                           const QString &channelName,
                           const QString &storagePath,
                           const QString &fallbackVideo)
    : channelIndex(channelIndex),
      channelName(channelName),
      storagePath(storagePath),
      fallbackVideo(fallbackVideo),
      captureTimer(nullptr),
      usingFallback(false),
      usingTestPattern(false),
      recordingRequested(false)
{
}

void CameraWorker::start()
{
#ifdef Q_OS_WIN
    capture.open(channelIndex, cv::CAP_DSHOW);
#else
    capture.open(channelIndex);
#endif

    if (!capture.isOpened() && !fallbackVideo.isEmpty())
    {
        capture.open(fallbackVideo.toStdString());
        usingFallback = capture.isOpened();
    }

    usingTestPattern = !capture.isOpened();
    emit stateChanged(!usingTestPattern,
                      usingTestPattern ? "测试画面"
                                       : (usingFallback ? "测试视频" : "实时在线"));

    captureTimer = new QTimer(this);
    captureTimer->setTimerType(Qt::PreciseTimer);
    connect(captureTimer, &QTimer::timeout, this, &CameraWorker::readFrame);
    captureTimer->start(40);
}

void CameraWorker::stop()
{
    if (captureTimer)
        captureTimer->stop();
    finishSegment();
    capture.release();
}

void CameraWorker::setRecording(bool enabled)
{
    recordingRequested = enabled;
    if (!enabled)
        finishSegment();
}

void CameraWorker::setChannelName(const QString &name)
{
    channelName = name;
}

void CameraWorker::setFallbackVideo(const QString &filePath)
{
    fallbackVideo = filePath;
    if (!usingTestPattern && !usingFallback)
        return;

    capture.release();
    capture.open(fallbackVideo.toStdString());
    usingFallback = capture.isOpened();
    usingTestPattern = !usingFallback;
    emit stateChanged(usingFallback,
                      usingFallback ? "备用视频" : "备用视频不可用 · 测试画面");
}

void CameraWorker::setStoragePath(const QString &path)
{
    storagePath = path;
    QDir().mkpath(storagePath);
}

void CameraWorker::readFrame()
{
    cv::Mat frame;
    if (usingTestPattern)
    {
        frame = makeTestFrame();
    }
    else if (!capture.read(frame) || frame.empty())
    {
        if (usingFallback)
        {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            capture.read(frame);
        }

        if (frame.empty())
        {
            usingTestPattern = true;
            emit stateChanged(false, "测试画面");
            frame = makeTestFrame();
        }
    }

    if (recordingRequested)
    {
        if (!writer.isOpened() && !beginSegment(frame.size()))
        {
            recordingRequested = false;
            emit recordingChanged(false);
        }

        if (writer.isOpened())
        {
            if (segmentStart.secsTo(QDateTime::currentDateTime()) >= 60)
            {
                finishSegment();
                beginSegment(frame.size());
            }

            cv::Mat outputFrame;
            if (frame.size() != writerSize)
                cv::resize(frame, outputFrame, writerSize);
            else
                outputFrame = frame;
            writer.write(outputFrame);
        }
    }

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    emit frameReady(QImage(rgb.data, rgb.cols, rgb.rows,
                           static_cast<int>(rgb.step), QImage::Format_RGB888).copy());
}

cv::Mat CameraWorker::makeTestFrame() const
{
    cv::Mat frame(720, 1280, CV_8UC3, cv::Scalar(46, 29, 18));

    return frame;
}

bool CameraWorker::beginSegment(const cv::Size &frameSize)
{
    segmentStart = QDateTime::currentDateTime();
    segmentPath = nextSegmentPath(segmentStart);
    writerSize = frameSize;

    if (!QDir().mkpath(QFileInfo(segmentPath).absolutePath()))
    {
        emit recordingError(QString("通道 %1 无法创建录像目录：%2")
                                .arg(channelIndex + 1)
                                .arg(QFileInfo(segmentPath).absolutePath()));
        segmentPath.clear();
        return false;
    }

    const double fps = capture.isOpened() ? capture.get(cv::CAP_PROP_FPS) : 25.0;
    const double safeFps = (fps >= 5.0 && fps <= 120.0) ? fps : 25.0;

    writer.open(segmentPath.toStdString(),
                cv::VideoWriter::fourcc('a', 'v', 'c', '1'),
                safeFps, writerSize, true);
    if (!writer.isOpened())
    {
        writer.open(segmentPath.toStdString(),
                    cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                    safeFps, writerSize, true);
    }

    if (!writer.isOpened())
    {
        emit recordingError(QString("通道 %1 无法创建录像文件：%2")
                                .arg(channelIndex + 1)
                                .arg(segmentPath));
        segmentPath.clear();
        return false;
    }

    emit recordingChanged(true);
    return true;
}

void CameraWorker::finishSegment()
{
    if (!writer.isOpened())
        return;

    writer.release();
    const QDateTime endTime = QDateTime::currentDateTime();
    const QFileInfo file(segmentPath);
    if (file.exists() && file.size() > 0)
        emit segmentFinished(channelIndex + 1, channelName, segmentStart,
                             endTime, segmentPath, "normal");
    else
        emit recordingError(QString("通道 %1 录像文件保存失败：%2")
                                .arg(channelIndex + 1)
                                .arg(segmentPath));
    emit recordingChanged(false);
    segmentPath.clear();
}

QString CameraWorker::nextSegmentPath(const QDateTime &startTime) const
{
    const QString folder = QString("%1/channel_%2/%3")
                               .arg(storagePath)
                               .arg(channelIndex + 1)
                               .arg(startTime.toString("yyyyMMdd"));
    return QString("%1/normal_ch%2_%3.avi")
        .arg(folder)
        .arg(channelIndex + 1)
        .arg(startTime.toString("yyyyMMdd_HHmmss"));
}
