#include "mainview.h"
#include "camerawidget.h"
#include "loginview.h"
#include "../model/dbconn.h"
#include "../model/videomodel.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

MainView::MainView(QWidget *parent)
    : QMainWindow(parent),
      loginView(nullptr),
      videoModel(new VideoModel),
      loggedIn(false),
      singleMode(false),
      diskWarningLogged(false),
      selectedChannel(0)
{
    setWindowTitle("智能监控系统");
    resize(1440, 900);
    setMinimumSize(1080, 700);

    QSettings settings;
    storagePath = settings.value(
        "video/storagePath",
        QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
            .filePath("SmartMonitor/recordings")).toString();
    QDir().mkpath(storagePath);

    buildUi();
    applyTheme();
    createCameraChannels();

    QString tableError;
    if (DbConn::getInstance().database().isOpen() && !videoModel->ensureTables(&tableError))
        showStatusMessage(QString("数据库表初始化失败：%1").arg(tableError), true);
    else if (!DbConn::getInstance().database().isOpen())
        showStatusMessage("数据库未连接：实时预览可用，登录和录像入库不可用。", true);
    loginButton->setEnabled(DbConn::getInstance().database().isOpen());
    loginButton->setToolTip(loginButton->isEnabled() ? QString() : "连接数据库后可登录");

    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout, this, &MainView::updateClock);
    clockTimer->start(1000);
    updateClock();

    diskTimer = new QTimer(this);
    connect(diskTimer, &QTimer::timeout, this, &MainView::updateDiskStatus);
    diskTimer->start(5000);
    updateDiskStatus();
    setLoggedIn(false);
}

MainView::~MainView()
{
    for (CameraWidget *camera : cameras)
        camera->stop();
    delete videoModel;
}

void MainView::buildUi()
{
    QWidget *center = new QWidget(this);
    center->setObjectName("appRoot");
    setCentralWidget(center);

    QVBoxLayout *root = new QVBoxLayout(center);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QFrame *topBar = new QFrame(center);
    topBar->setObjectName("topBar");
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(28, 18, 28, 18);

    QLabel *brand = new QLabel("SENTRY LOCAL", topBar);
    brand->setObjectName("brandLabel");
    QLabel *title = new QLabel("智能监控控制台", topBar);
    title->setObjectName("titleLabel");
    clockLabel = new QLabel(topBar);
    clockLabel->setObjectName("clockLabel");
    userLabel = new QLabel("未登录", topBar);
    userLabel->setObjectName("userLabel");
    loginButton = new QPushButton("管理员登录", topBar);
    loginButton->setObjectName("loginButton");

    QVBoxLayout *brandStack = new QVBoxLayout;
    brandStack->setSpacing(2);
    brandStack->addWidget(brand);
    brandStack->addWidget(title);
    topLayout->addLayout(brandStack);
    topLayout->addStretch();
    topLayout->addWidget(clockLabel);
    topLayout->addSpacing(18);
    topLayout->addWidget(userLabel);
    topLayout->addWidget(loginButton);
    root->addWidget(topBar);

    QWidget *body = new QWidget(center);
    QHBoxLayout *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    QFrame *sidebar = new QFrame(body);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(210);
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(18, 24, 18, 20);
    sideLayout->setSpacing(9);

    QLabel *navTitle = new QLabel("工作区", sidebar);
    navTitle->setObjectName("sectionCaption");
    QPushButton *liveButton = new QPushButton("  实时监控", sidebar);
    liveButton->setObjectName("navActive");
    const QStringList plannedItems = {"  录像回放", "  图片管理", "  移动侦测", "  事件日志"};
    sideLayout->addWidget(navTitle);
    sideLayout->addWidget(liveButton);
    for (const QString &item : plannedItems)
    {
        QPushButton *button = new QPushButton(item, sidebar);
        button->setObjectName("navPlanned");
        button->setEnabled(false);
        button->setToolTip("当前版本暂未开放");
        sideLayout->addWidget(button);
    }
    sideLayout->addStretch();
    QLabel *scopeLabel = new QLabel("本地运行\n数据保存在本机", sidebar);
    scopeLabel->setObjectName("scopeLabel");
    sideLayout->addWidget(scopeLabel);

    QWidget *workspace = new QWidget(body);
    workspace->setObjectName("workspace");
    QVBoxLayout *workLayout = new QVBoxLayout(workspace);
    workLayout->setContentsMargins(24, 22, 24, 18);
    workLayout->setSpacing(14);

    QHBoxLayout *heading = new QHBoxLayout;
    QVBoxLayout *headingText = new QVBoxLayout;
    QLabel *pageTitle = new QLabel("实时监控", workspace);
    pageTitle->setObjectName("pageTitle");
    QLabel *pageHint = new QLabel("本地视频流与录像状态", workspace);
    pageHint->setObjectName("pageHint");
    headingText->addWidget(pageTitle);
    headingText->addWidget(pageHint);
    heading->addLayout(headingText);
    heading->addStretch();

    singleViewButton = new QPushButton("单画面", workspace);
    fourViewButton = new QPushButton("四画面", workspace);
    fourViewButton->setProperty("active", true);
    renameButton = new QPushButton("通道命名", workspace);
    recordButton = new QPushButton("开始录像", workspace);
    recordButton->setObjectName("recordButton");
    recordButton->setCheckable(true);
    heading->addWidget(singleViewButton);
    heading->addWidget(fourViewButton);
    heading->addWidget(renameButton);
    heading->addWidget(recordButton);
    workLayout->addLayout(heading);

    cameraGrid = new QGridLayout;
    cameraGrid->setContentsMargins(0, 0, 0, 0);
    cameraGrid->setHorizontalSpacing(12);
    cameraGrid->setVerticalSpacing(12);
    cameraGrid->setColumnStretch(0, 1);
    cameraGrid->setColumnStretch(1, 1);
    cameraGrid->setRowStretch(0, 1);
    cameraGrid->setRowStretch(1, 1);
    workLayout->addLayout(cameraGrid, 1);

    QFrame *statusBar = new QFrame(workspace);
    statusBar->setObjectName("statusBar");
    QHBoxLayout *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(14, 10, 14, 10);
    statusLabel = new QLabel("系统就绪", statusBar);
    statusLabel->setObjectName("statusMessage");
    recordingSummaryLabel = new QLabel("录像未启动", statusBar);
    recordingSummaryLabel->setObjectName("statusMeta");
    diskLabel = new QLabel(statusBar);
    diskLabel->setObjectName("statusMeta");
    statusLayout->addWidget(statusLabel, 1);
    statusLayout->addWidget(recordingSummaryLabel);
    statusLayout->addSpacing(18);
    statusLayout->addWidget(diskLabel);
    workLayout->addWidget(statusBar);

    bodyLayout->addWidget(sidebar);
    bodyLayout->addWidget(workspace, 1);
    root->addWidget(body, 1);

    connect(loginButton, &QPushButton::clicked, this, &MainView::openLoginView);
    connect(singleViewButton, &QPushButton::clicked, this, &MainView::showSingleView);
    connect(fourViewButton, &QPushButton::clicked, this, &MainView::showFourView);
    connect(recordButton, &QPushButton::toggled, this, &MainView::toggleRecording);
    connect(renameButton, &QPushButton::clicked, this, &MainView::renameSelectedChannel);
}

void MainView::createCameraChannels()
{
    QSettings settings;
    const QString fallbackVideo = settings.value("video/fallbackVideo").toString();
    for (int i = 0; i < 4; ++i)
    {
        const QString channelName = settings.value(
            QString("channels/%1/name").arg(i + 1),
            QString("通道 %1").arg(i + 1)).toString();
        CameraWidget *camera = new CameraWidget(i, channelName, storagePath,
                                                fallbackVideo, this);
        cameras.append(camera);
        cameraGrid->addWidget(camera, i / 2, i % 2);
        connect(camera, &CameraWidget::selected, this, &MainView::selectChannel);
        connect(camera, &CameraWidget::segmentFinished, this, &MainView::saveSegment);
        connect(camera, &CameraWidget::recordingError, this, &MainView::showRecordingError);
        camera->start();
    }
    selectChannel(0);
}

void MainView::applyTheme()
{
    setStyleSheet(
        "* { font-family: 'Microsoft YaHei UI'; font-size: 10pt; }"
        "QWidget#appRoot { background: #0B1421; color: #E7EEF5; }"
        "QFrame#topBar { background: #0F1C2C; border-bottom: 1px solid #21344B; }"
        "QLabel#brandLabel { color: #64D7CA; font: 700 9pt 'Consolas'; letter-spacing: 2px; }"
        "QLabel#titleLabel { color: #F5F8FC; font-size: 17pt; font-weight: 700; }"
        "QLabel#clockLabel { color: #91A6BA; font-family: 'Consolas'; }"
        "QLabel#userLabel { color: #BDD0DE; padding: 8px 12px; background: #15263A; border-radius: 7px; }"
        "QPushButton#loginButton { color: #0A1722; background: #64D7CA; border: 0; border-radius: 7px; padding: 9px 16px; font-weight: 700; }"
        "QPushButton#loginButton:hover { background: #7BE2D7; }"
        "QFrame#sidebar { background: #0C1725; border-right: 1px solid #1C3046; }"
        "QLabel#sectionCaption { color: #70869B; font-size: 9pt; padding: 0 8px 7px 8px; }"
        "QPushButton#navActive { text-align: left; color: #EAF7F5; background: #173B42; border: 1px solid #24575D; border-radius: 8px; padding: 11px 12px; font-weight: 700; }"
        "QPushButton#navPlanned { text-align: left; color: #586B7E; background: transparent; border: 0; padding: 10px 12px; }"
        "QLabel#scopeLabel { color: #6E8295; background: #101E2F; border: 1px solid #1D3046; border-radius: 8px; padding: 11px; }"
        "QWidget#workspace { background: #0B1421; }"
        "QLabel#pageTitle { color: #F4F7FA; font-size: 18pt; font-weight: 700; }"
        "QLabel#pageHint { color: #71869A; }"
        "QPushButton { color: #BFD0DE; background: #15253A; border: 1px solid #294058; border-radius: 7px; padding: 8px 13px; }"
        "QPushButton:hover { color: #F5FAFC; border-color: #4C6B87; }"
        "QPushButton[active='true'] { color: #EAF8F6; background: #1A4549; border-color: #32716F; }"
        "QPushButton:disabled { color: #526579; background: #111D2B; border-color: #1B2A3A; }"
        "QPushButton#recordButton { color: #FFD9D2; background: #3A2027; border-color: #6E3841; font-weight: 700; }"
        "QPushButton#recordButton:checked { color: #FFFFFF; background: #D94D55; border-color: #FF7278; }"
        "QFrame#cameraCard { background: #101D2C; border: 1px solid #22384E; border-radius: 10px; }"
        "QFrame#cameraCard[selected='true'] { border: 2px solid #57CFC2; }"
        "QLabel#cameraName { color: #E7EFF6; font-weight: 700; font-family: 'Consolas'; }"
        "QLabel#cameraState { color: #E1A85A; font-size: 9pt; }"
        "QLabel#cameraState[online='true'] { color: #64D7CA; }"
        "QLabel#recordingBadge { color: white; background: #D94D55; border-radius: 5px; padding: 3px 7px; font: 700 8pt 'Consolas'; }"
        "QLabel#videoSurface { background: #08111D; border: 0; border-bottom-left-radius: 9px; border-bottom-right-radius: 9px; }"
        "QFrame#statusBar { background: #101D2C; border: 1px solid #22374D; border-radius: 8px; }"
        "QLabel#statusMessage { color: #9FB3C5; }"
        "QLabel#statusMessage[warning='true'] { color: #FFAE86; font-weight: 700; }"
        "QLabel#statusMeta { color: #71879B; font-family: 'Consolas'; }"
    );
}

void MainView::openLoginView()
{
    if (loggedIn)
    {
        if (recordButton->isChecked())
            recordButton->setChecked(false);
        setLoggedIn(false);
        showStatusMessage("已退出管理员账号，录像与配置操作已锁定。");
        return;
    }

    if (!loginView)
    {
        loginView = new LoginView(this);
        connect(loginView, &LoginView::loginSucceeded,
                this, &MainView::onLoginSucceeded);
    }
    loginView->show();
    loginView->raise();
    loginView->activateWindow();
}

void MainView::onLoginSucceeded(const QString &username)
{
    setLoggedIn(true, username);
    showStatusMessage(QString("管理员 %1 已登录。").arg(username));
}

void MainView::selectChannel(int index)
{
    selectedChannel = index;
    for (CameraWidget *camera : cameras)
        camera->setSelected(camera->channelIndex() == index);
    if (singleMode)
        showSingleView();
}

void MainView::showSingleView()
{
    singleMode = true;
    for (CameraWidget *camera : cameras)
    {
        cameraGrid->removeWidget(camera);
        camera->setVisible(camera->channelIndex() == selectedChannel);
    }
    cameraGrid->addWidget(cameras.at(selectedChannel), 0, 0, 2, 2);
    singleViewButton->setProperty("active", true);
    fourViewButton->setProperty("active", false);
    singleViewButton->style()->unpolish(singleViewButton);
    singleViewButton->style()->polish(singleViewButton);
    fourViewButton->style()->unpolish(fourViewButton);
    fourViewButton->style()->polish(fourViewButton);
}

void MainView::showFourView()
{
    singleMode = false;
    for (CameraWidget *camera : cameras)
    {
        cameraGrid->removeWidget(camera);
        camera->show();
        cameraGrid->addWidget(camera, camera->channelIndex() / 2,
                              camera->channelIndex() % 2);
    }
    singleViewButton->setProperty("active", false);
    fourViewButton->setProperty("active", true);
    singleViewButton->style()->unpolish(singleViewButton);
    singleViewButton->style()->polish(singleViewButton);
    fourViewButton->style()->unpolish(fourViewButton);
    fourViewButton->style()->polish(fourViewButton);
}

void MainView::toggleRecording(bool enabled)
{
    if (enabled && !loggedIn)
    {
        recordButton->setChecked(false);
        showStatusMessage("请先登录管理员账号，再启动录像。", true);
        return;
    }
    if (enabled && !DbConn::getInstance().database().isOpen())
    {
        recordButton->setChecked(false);
        showStatusMessage("数据库未连接，无法启动需要入库的录像。", true);
        return;
    }

    for (CameraWidget *camera : cameras)
        camera->setRecording(enabled);
    recordButton->setText(enabled ? "停止录像" : "开始录像");
    recordingSummaryLabel->setText(enabled ? "REC · 四路分段录像" : "录像未启动");
    showStatusMessage(enabled ? "已启动四路录像，每 60 秒自动分段。"
                              : "录像已停止，未满 60 秒的视频段已保存。");
}

void MainView::renameSelectedChannel()
{
    if (!loggedIn)
    {
        showStatusMessage("请先登录管理员账号，再修改通道名称。", true);
        return;
    }

    CameraWidget *camera = cameras.at(selectedChannel);
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, "通道命名", QString("设置通道 %1 名称").arg(selectedChannel + 1),
        QLineEdit::Normal, camera->channelName(), &ok).trimmed();
    if (!ok || name.isEmpty())
        return;

    camera->setChannelName(name.left(32));
    QSettings().setValue(QString("channels/%1/name").arg(selectedChannel + 1), name.left(32));
    showStatusMessage(QString("通道 %1 已命名为“%2”。")
                          .arg(selectedChannel + 1).arg(name.left(32)));
}

void MainView::saveSegment(int channel,
                           const QString &channelName,
                           const QDateTime &startTime,
                           const QDateTime &endTime,
                           const QString &filePath,
                           const QString &eventType)
{
    if (!videoModel->saveSegment(channel, channelName, startTime,
                                 endTime, filePath, eventType))
        showStatusMessage(QString("通道 %1 录像已保存，但数据库记录写入失败。").arg(channel), true);
    else
        showStatusMessage(QString("通道 %1 已保存录像段：%2")
                              .arg(channel)
                              .arg(QFileInfo(filePath).fileName()));
}

void MainView::updateClock()
{
    clockLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  HH:mm:ss"));
}

void MainView::updateDiskStatus()
{
    QStorageInfo storage(storagePath);
    if (!storage.isValid() || storage.bytesTotal() <= 0)
    {
        diskLabel->setText("磁盘状态不可用");
        return;
    }

    const double freePercent = storage.bytesAvailable() * 100.0 / storage.bytesTotal();
    const double freeGb = storage.bytesAvailable() / 1024.0 / 1024.0 / 1024.0;
    diskLabel->setText(QString("磁盘可用 %1 GB · %2%")
                           .arg(freeGb, 0, 'f', 1)
                           .arg(freePercent, 0, 'f', 1));

    if (freePercent <= 10.0)
    {
        showStatusMessage(QString("硬盘空间低（剩余 %1%）").arg(freePercent, 0, 'f', 1), true);
        if (!diskWarningLogged && DbConn::getInstance().database().isOpen())
        {
            videoModel->addSystemLog("warning",
                                     QString("硬盘空间低（剩余 %1%）").arg(freePercent, 0, 'f', 1));
            diskWarningLogged = true;
        }
    }
    else
    {
        diskWarningLogged = false;
    }
}

void MainView::showRecordingError(const QString &message)
{
    if (recordButton->isChecked())
        recordButton->setChecked(false);
    showStatusMessage(message, true);
}

void MainView::setLoggedIn(bool enabled, const QString &username)
{
    loggedIn = enabled;
    userLabel->setText(enabled ? QString("管理员 · %1").arg(username) : "访客模式");
    loginButton->setText(enabled ? "退出登录" : "管理员登录");
    renameButton->setEnabled(enabled);
    recordButton->setEnabled(enabled && DbConn::getInstance().database().isOpen());
    recordButton->setToolTip(enabled ? QString() : "登录后可启动录像");
}

void MainView::showStatusMessage(const QString &message, bool warning)
{
    statusLabel->setText(message);
    statusLabel->setProperty("warning", warning);
    statusLabel->style()->unpolish(statusLabel);
    statusLabel->style()->polish(statusLabel);
}
