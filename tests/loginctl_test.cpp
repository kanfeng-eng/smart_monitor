#include "../src/controller/loginctl.h"

#include <QCoreApplication>
#include <QString>

#include <cassert>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    LoginCtl controller;
    QString error;

    assert(!controller.checkInput("", "", error));
    assert(error == "账号或密码不能为空");
    assert(!controller.checkInput("abc", "123456", error));
    assert(!controller.checkInput("admin01", "12345", error));
    assert(controller.checkInput("admin01", "abc123", error));
    return 0;
}
