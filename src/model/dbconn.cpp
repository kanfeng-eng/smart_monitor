#include "dbconn.h"

#include <QDebug>
#include <QSqlError>

DbConn::DbConn()
{
    db = QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName("localhost");
    db.setPort(3306);

    db.setDatabaseName("smart_monitor");

    db.setUserName("root");
    db.setPassword("123456");
}

DbConn& DbConn::getInstance()
{
    static DbConn instance;
    return instance;
}

bool DbConn::open()
{
    if (db.isOpen())
        return true;

    if (!db.open())
    {
        qDebug() << "数据库连接失败:";
        qDebug() << db.lastError().text();

        return false;
    }

    qDebug() << "数据库连接成功";

    return true;
}

void DbConn::close()
{
    db.close();
}

QSqlDatabase DbConn::database()
{
    return db;
}