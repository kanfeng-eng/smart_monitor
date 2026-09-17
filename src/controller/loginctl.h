#ifndef LOGINCTL_H
#define LOGINCTL_H

#include <QString>

class LoginCtl
{
public:
    bool login(const QString &username,
               const QString &password);

    bool checkInput(const QString &username,
                    const QString &password,
                    QString &error);
};

#endif