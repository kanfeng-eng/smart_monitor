#include "loginview.h"

#include "../controller/loginctl.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QUuid>
#include <QVBoxLayout>

LoginView::LoginView(QWidget *parent)
    : QWidget(parent),
      failedAttempts(0),
      lockUntilMs(0)
{
    setWindowTitle("管理员登录");
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFixedSize(440, 430);
    setObjectName("loginPanel");

    QLabel *eyebrowLabel = new QLabel("SMART MONITOR  ·  LOCAL ACCESS", this);
    eyebrowLabel->setObjectName("eyebrowLabel");
    QLabel *titleLabel = new QLabel("管理员登录", this);
    titleLabel->setObjectName("loginTitle");
    QLabel *hintLabel = new QLabel("登录后可启用录像、通道配置与数据管理。", this);
    hintLabel->setObjectName("loginHint");

    usernameEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    captchaEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("请输入 6~10 位账号");
    passwordEdit->setPlaceholderText("请输入 6~8 位密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    captchaEdit->setPlaceholderText("请输入验证码");

    showPasswordButton = new QPushButton("显示", this);
    showPasswordButton->setObjectName("textButton");
    captchaButton = new QPushButton(this);
    captchaButton->setToolTip("点击刷新验证码");
    captchaButton->setObjectName("captchaButton");
    loginButton = new QPushButton("登录", this);
    loginButton->setObjectName("primaryButton");

    errorLabel = new QLabel(this);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setWordWrap(true);
    errorLabel->hide();

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->setSpacing(8);
    passwordLayout->addWidget(passwordEdit, 1);
    passwordLayout->addWidget(showPasswordButton);

    QHBoxLayout *captchaLayout = new QHBoxLayout;
    captchaLayout->setSpacing(8);
    captchaLayout->addWidget(captchaEdit, 1);
    captchaLayout->addWidget(captchaButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(42, 36, 42, 36);
    layout->setSpacing(14);
    layout->addWidget(eyebrowLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(hintLabel);
    layout->addSpacing(10);
    layout->addWidget(usernameEdit);
    layout->addLayout(passwordLayout);
    layout->addLayout(captchaLayout);
    layout->addWidget(errorLabel);
    layout->addStretch();
    layout->addWidget(loginButton);

    connect(loginButton, &QPushButton::clicked, this, &LoginView::onLoginClicked);
    connect(showPasswordButton, &QPushButton::clicked,
            this, &LoginView::onShowPasswordClicked);
    connect(captchaButton, &QPushButton::clicked, this, &LoginView::refreshCaptcha);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginView::onLoginClicked);
    connect(captchaEdit, &QLineEdit::returnPressed, this, &LoginView::onLoginClicked);

    setStyleSheet(
        "QWidget#loginPanel { background: #0D1726; color: #E8F0F7; }"
        "QLabel#eyebrowLabel { color: #62D5C9; font: 700 10pt 'Microsoft YaHei UI'; letter-spacing: 1px; }"
        "QLabel#loginTitle { color: #F5F8FC; font: 700 24pt 'Microsoft YaHei UI'; }"
        "QLabel#loginHint { color: #8FA3B8; font: 10pt 'Microsoft YaHei UI'; }"
        "QLabel#errorLabel { color: #FF9B88; background: #3A2025; border: 1px solid #6B3338; border-radius: 7px; padding: 8px; }"
        "QLineEdit { min-height: 42px; padding: 0 13px; color: #F5F8FC; background: #142238; border: 1px solid #263C56; border-radius: 8px; selection-background-color: #2FB7A9; }"
        "QLineEdit:focus { border: 1px solid #53CFC2; }"
        "QPushButton#primaryButton { min-height: 44px; color: #07131E; background: #65D8CC; border: 0; border-radius: 8px; font-weight: 700; }"
        "QPushButton#primaryButton:hover { background: #7CE4D9; }"
        "QPushButton#textButton { min-width: 58px; min-height: 42px; color: #BFD0DE; background: #182A42; border: 1px solid #2B415B; border-radius: 8px; }"
        "QPushButton#captchaButton { min-width: 112px; min-height: 42px; color: #FFD486; background: #263044; border: 1px dashed #D89B3C; border-radius: 8px; font: 700 14pt 'Consolas'; letter-spacing: 3px; }"
    );

    refreshCaptcha();
}

void LoginView::onLoginClicked()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now < lockUntilMs)
    {
        const int seconds = static_cast<int>((lockUntilMs - now + 999) / 1000);
        showInlineError(QString("连续登录失败，账号已临时锁定。请在 %1 秒后重试。").arg(seconds));
        return;
    }

    const QString username = usernameEdit->text().trimmed();
    const QString password = passwordEdit->text();
    LoginCtl ctl;
    QString error;

    if (!ctl.checkInput(username, password, error))
    {
        showInlineError(error);
        refreshCaptcha();
        return;
    }

    if (captchaEdit->text().trimmed().compare(captchaText, Qt::CaseInsensitive) != 0)
    {
        showInlineError("验证码错误，请重新输入。");
        captchaEdit->clear();
        refreshCaptcha();
        return;
    }

    if (ctl.login(username, password))
    {
        failedAttempts = 0;
        errorLabel->hide();
        passwordEdit->clear();
        captchaEdit->clear();
        refreshCaptcha();
        emit loginSucceeded(username);
        close();
        return;
    }

    ++failedAttempts;
    if (failedAttempts >= 3)
    {
        lockUntilMs = now + 10 * 60 * 1000;
        failedAttempts = 0;
        showInlineError("连续 3 次登录失败，账号已锁定 10 分钟。");
    }
    else
    {
        showInlineError(QString("账号或密码错误，还可尝试 %1 次。").arg(3 - failedAttempts));
    }
    passwordEdit->clear();
    captchaEdit->clear();
    refreshCaptcha();
}

void LoginView::onShowPasswordClicked()
{
    const bool hidden = passwordEdit->echoMode() == QLineEdit::Password;
    passwordEdit->setEchoMode(hidden ? QLineEdit::Normal : QLineEdit::Password);
    showPasswordButton->setText(hidden ? "隐藏" : "显示");
}

void LoginView::refreshCaptcha()
{
    captchaText = QUuid::createUuid().toString()
                      .remove('-').remove('{').remove('}')
                      .left(4).toUpper();
    captchaButton->setText(captchaText);
}

void LoginView::showInlineError(const QString &message)
{
    errorLabel->setText(message);
    errorLabel->show();
}
