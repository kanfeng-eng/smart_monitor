#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFont>
#include <QFontDatabase>

#include "src/view/mainview.h"
#include "src/model/dbconn.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setOrganizationName("SmartMonitor");
    app.setApplicationName("SmartMonitor");
    app.setStyle("Fusion");
    if (QFile::exists("C:/Windows/Fonts/msyh.ttc"))
        QFontDatabase::addApplicationFont("C:/Windows/Fonts/msyh.ttc");
    app.setFont(QFont("Microsoft YaHei UI", 10));

    if (!DbConn::getInstance().open())
        qWarning() << "数据库不可用，实时预览仍可使用";

    MainView view;

    view.show();

    const int result = app.exec();
    DbConn::getInstance().close();
    return result;
}
