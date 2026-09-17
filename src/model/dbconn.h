#ifndef DBCONN_H
#define DBCONN_H

#include <QSqlDatabase>

class DbConn
{
public:
    static DbConn& getInstance();

    bool open();
    void close();

    QSqlDatabase database();

private:
    DbConn();

    DbConn(const DbConn&) = delete;
    DbConn& operator=(const DbConn&) = delete;

private:
    QSqlDatabase db;
};

#endif