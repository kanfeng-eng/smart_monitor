#include "usermodel.h"
#include "dbconn.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool UserModel::checkLogin(const QString &username,
                           const QString &password)
{
    QSqlDatabase db =
        DbConn::getInstance().database();

    QSqlQuery query(db);

    query.prepare(
        "SELECT admin_id "
        "FROM admins "
        "WHERE username = :username "
        "AND password = MD5(:password)"
        );

    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec())
    {
        qDebug() << query.lastError().text();
        return false;
    }

    return query.next();
}