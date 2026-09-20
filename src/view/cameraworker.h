#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QDateTime>
#include <QImage>
#include <QObject>
#include <QString>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

class QTimer;

class CameraWorker : public QObject
{
    Q_OBJECT

public:
    CameraWorker(int channelIndex,
                 const QString &channelName,
                 const QString &storagePath,
                 const QString &fallbackVideo = QString());

public slots:
    void start();
    void stop();
    void setRecording(bool enabled);
    void setChannelName(const QString &name);

signals:
    void frameReady(const QImage &frame);
    void stateChanged(bool connected, const QString &text);
    void recordingChanged(bool recording);
    void recordingError(const QString &message);
    void segmentFinished(int channel,
                         const QString &channelName,
                         const QDateTime &startTime,
                         const QDateTime &endTime,
                         const QString &filePath,
                         const QString &eventType);

private slots:
    void readFrame();

private:
    cv::Mat makeTestFrame() const;
    bool beginSegment(const cv::Size &frameSize);
    void finishSegment();
    QString nextSegmentPath(const QDateTime &startTime) const;

    int channelIndex;
    QString channelName;
    QString storagePath;
    QString fallbackVideo;
    QTimer *captureTimer;
    cv::VideoCapture capture;
    cv::VideoWriter writer;
    cv::Size writerSize;
    bool usingFallback;
    bool usingTestPattern;
    bool recordingRequested;
    QDateTime segmentStart;
    QString segmentPath;
};

#endif
