#ifndef PLAYBACKDIALOG_H
#define PLAYBACKDIALOG_H

#include <QDialog>
#include <QImage>
#include <QThread>
#include <QVector>

#include "../model/videomodel.h"

class QComboBox;
class QDateTimeEdit;
class QLabel;
class QListWidget;
class QPushButton;
class QSlider;
class PlaybackWorker;
class PlaybackDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaybackDialog(const QString &storagePath, QWidget *parent = nullptr);
    ~PlaybackDialog() override;

private slots:
    void querySegments();
    void chooseFile();
    void play();
    void pause();
    void resume();
    void stop();
    void setSpeed(int index);
    void seek(int value);
    void showFrame(const QImage &frame, qint64 position, qint64 totalFrames);
    void showMedia(double fps, qint64 totalFrames);
    void showWorkerMessage(const QString &message, bool warning);
    void playbackEnded();

private:
    void buildUi();
    void showSegments(const QVector<VideoSegment> &segments);
    void appendFileSegment(const QString &filePath);
    bool openFile(const QString &filePath);
    void showMessage(const QString &message, bool warning = false);

    QString storagePath;
    VideoModel *videoModel;
    QDateTimeEdit *fromEdit;
    QDateTimeEdit *toEdit;
    QComboBox *channelCombo;
    QListWidget *segmentList;
    QLabel *frameLabel;
    QLabel *statusLabel;
    QSlider *progressSlider;
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *resumeButton;
    QPushButton *stopButton;
    QComboBox *speedCombo;
    QThread *playbackThread;
    PlaybackWorker *playbackWorker;
    double fps;
    qint64 totalFrames;
    bool updatingSlider;
};

#endif
