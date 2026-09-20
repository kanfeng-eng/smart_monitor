#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QDateTime>
#include <QMainWindow>
#include <QVector>

class CameraWidget;
class LoginView;
class VideoModel;
class QLabel;
class QPushButton;
class QGridLayout;
class QTimer;

class MainView : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainView(QWidget *parent = nullptr);
    ~MainView() override;

private slots:
    void openLoginView();
    void onLoginSucceeded(const QString &username);
    void selectChannel(int index);
    void showSingleView();
    void showFourView();
    void toggleRecording(bool enabled);
    void renameSelectedChannel();
    void saveSegment(int channel,
                     const QString &channelName,
                     const QDateTime &startTime,
                     const QDateTime &endTime,
                     const QString &filePath,
                     const QString &eventType);
    void updateClock();
    void updateDiskStatus();
    void showRecordingError(const QString &message);

private:
    void buildUi();
    void createCameraChannels();
    void applyTheme();
    void setLoggedIn(bool loggedIn, const QString &username = QString());
    void showStatusMessage(const QString &message, bool warning = false);

    QVector<CameraWidget *> cameras;
    LoginView *loginView;
    VideoModel *videoModel;
    QGridLayout *cameraGrid;
    QPushButton *loginButton;
    QPushButton *singleViewButton;
    QPushButton *fourViewButton;
    QPushButton *recordButton;
    QPushButton *renameButton;
    QLabel *userLabel;
    QLabel *clockLabel;
    QLabel *diskLabel;
    QLabel *statusLabel;
    QLabel *recordingSummaryLabel;
    QTimer *clockTimer;
    QTimer *diskTimer;
    QString storagePath;
    bool loggedIn;
    bool singleMode;
    bool diskWarningLogged;
    int selectedChannel;
};

#endif
