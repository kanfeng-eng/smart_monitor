#include "loginview.h"

#include "../controller/loginctl.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>

LoginView::LoginView(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("管理员登录");
    resize(400, 250);

    QLabel *titleLabel =
        new QLabel("管理员登录", this);

    usernameEdit =
        new QLineEdit(this);

    passwordEdit =
        new QLineEdit(this);

    loginButton =
        new QPushButton("登录", this);

    showPasswordButton =
        new QPushButton("显示密码", this);


    usernameEdit->setPlaceholderText(
        "请输入账号"
        );

    passwordEdit->setPlaceholderText(
        "请输入密码"
        );

    passwordEdit->setEchoMode(
        QLineEdit::Password
        );


    QVBoxLayout *layout =
        new QVBoxLayout(this);

    layout->addWidget(titleLabel);
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(showPasswordButton);
    layout->addWidget(loginButton);


    connect(
        loginButton,
        &QPushButton::clicked,
        this,
        &LoginView::onLoginClicked
        );

    connect(
        showPasswordButton,
        &QPushButton::clicked,
        this,
        &LoginView::onShowPasswordClicked
        );
}

void LoginView::onLoginClicked()
{
    QString username =
        usernameEdit->text();

    QString password =
        passwordEdit->text();

    LoginCtl ctl;

    QString error;

    if (!ctl.checkInput(
            username,
            password,
            error))
    {
        QMessageBox::warning(
            this,
            "错误",
            error
            );

        return;
    }


    if (ctl.login(username, password))
    {
        QMessageBox::information(
            this,
            "登录",
            "登录成功"
            );

        close();
    }
    else
    {
        QMessageBox::warning(
            this,
            "登录",
            "账号或密码错误"
            );
    }
}

void LoginView::onShowPasswordClicked()
{
    if (passwordEdit->echoMode()
        == QLineEdit::Password)
    {
        passwordEdit->setEchoMode(
            QLineEdit::Normal
            );

        showPasswordButton->setText(
            "隐藏密码"
            );
    }
    else
    {
        passwordEdit->setEchoMode(
            QLineEdit::Password
            );

        showPasswordButton->setText(
            "显示密码"
            );
    }
}