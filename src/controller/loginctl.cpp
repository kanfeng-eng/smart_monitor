#include "loginctl.h"
#include "../model/usermodel.h"

#include <QRegularExpression>

bool LoginCtl::checkInput(const QString &username,
                          const QString &password,
                          QString &error)
{
    if (username.isEmpty() ||
        password.isEmpty())
    {
        error = "账号或密码不能为空";
        return false;
    }

    QRegularExpression usernameRegex(
        "^[A-Za-z0-9]{6,10}$"
        );

    QRegularExpression passwordRegex(
        "^[A-Za-z0-9]{6,8}$"
        );

    if (!usernameRegex.match(username).hasMatch())
    {
        error = "账号必须为6~10位数字或字母";
        return false;
    }

    if (!passwordRegex.match(password).hasMatch())
    {
        error = "密码必须为6~8位数字或字母";
        return false;
    }

    return true;
}

bool LoginCtl::login(const QString &username,
                     const QString &password)
{
    UserModel model;

    return model.checkLogin(
        username,
        password
        );
}