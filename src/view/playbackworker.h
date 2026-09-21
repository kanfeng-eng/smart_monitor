#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QObject>
#include <QImage>
#include <QString>

#include <opencv2/videoio.hpp>

class QTimer;

class PlaybackWorker : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackWorker(QObject *parent = nullptr);
    ~PlaybackWorker() override;

public slots:
    void openFile(const QString &filePath);
    void play();
    void pause();
    void resume();
    void stop();
    void seek(qint64 frame);
    void setSpeed(double speed);
    void shutdown();

signals:
    void frameReady(const QImage &frame, qint64 position, qint64 totalFrames);
    void mediaReady(double fps, qint64 totalFrames);
    void messageReady(const QString &message, bool warning);
    void playbackEnded();

private slots:
    void readNextFrame();

private:
    void updateTimerInterval();
    void renderCurrentFrame();

    QTimer *timer;
    cv::VideoCapture capture;
    double fps;
    double speed;
    qint64 totalFrames;
};

#endif
