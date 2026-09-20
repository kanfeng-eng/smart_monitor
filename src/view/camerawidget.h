#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QDateTime>
#include <QFrame>
#include <QImage>

class CameraWorker;
class QLabel;
class QThread;

class CameraWidget : public QFrame
{
    Q_OBJECT

public:
    CameraWidget(int channelIndex,
                 const QString &channelName,
                 const QString &storagePath,
                 const QString &fallbackVideo,
                 QWidget *parent = nullptr);
    ~CameraWidget() override;

    int channelIndex() const;
    QString channelName() const;
    void start();
    void stop();
    void setRecording(bool enabled);
    void setChannelName(const QString &name);
    void setSelected(bool selected);

signals:
    void selected(int channelIndex);
    void segmentFinished(int channel,
                         const QString &channelName,
                         const QDateTime &startTime,
                         const QDateTime &endTime,
                         const QString &filePath,
                         const QString &eventType);
    void recordingError(const QString &message);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateFrame(const QImage &frame);
    void updateState(bool connected, const QString &text);
    void updateRecording(bool recording);

private:
    void refreshPixmap();

    int index;
    QString name;
    QLabel *nameLabel;
    QLabel *stateLabel;
    QLabel *videoLabel;
    QLabel *recordingLabel;
    QImage lastFrame;
    QThread *thread;
    CameraWorker *worker;
    bool started;
};

#endif
