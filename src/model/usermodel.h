#ifndef USERMODEL_H
#define USERMODEL_H

#include <QString>

class UserModel
{
public:
    bool checkLogin(const QString &username,
                    const QString &password);
};

#endif