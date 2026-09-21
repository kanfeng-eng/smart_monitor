#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QDialog>

#include "../utils/captchagenerator.h"

class QLineEdit;
class QPushButton;
class QLabel;

class LoginView : public QDialog
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parent = nullptr);

signals:
    void loginSucceeded(const QString &username);

private slots:
    void onLoginClicked();
    void onShowPasswordClicked();
    void refreshCaptcha();

private:
    void showInlineError(const QString &message);

    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *captchaEdit;

    QPushButton *loginButton;
    QPushButton *showPasswordButton;
    QPushButton *captchaButton;
    QLabel *errorLabel;

    CaptchaGenerator captchaGenerator;
    int failedAttempts;
    qint64 lockUntilMs;
};

#endif
