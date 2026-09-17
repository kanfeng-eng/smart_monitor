#include <QApplication>
#include <QDebug>

#include "src/model/dbconn.h"
#include "src/view/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!DbConn::getInstance().open())
    {
        qDebug() << "数据库启动失败";
        return -1;
    }

    MainWindow w;
    w.show();

    return app.exec();
}