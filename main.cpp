#include <QApplication>

#include "src/view/mainview.h"
#include "src/model/dbconn.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!DbConn::getInstance().open())
        return -1;

    MainView view;

    view.show();

    return app.exec();
}