#include "mainview.h"
#include "loginview.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

MainView::MainView(QWidget *parent)
    : QMainWindow(parent),
    loginView(nullptr)
{
    resize(1200, 800);

    QWidget *center =
        new QWidget(this);

    setCentralWidget(center);

    loginButton =
        new QPushButton("登录", center);

    QVBoxLayout *layout =
        new QVBoxLayout(center);

    layout->addWidget(loginButton);

    connect(
        loginButton,
        &QPushButton::clicked,
        this,
        &MainView::openLoginView
        );
}

void MainView::openLoginView()
{
    if (loginView == nullptr)
    {
        loginView =
            new LoginView(this);
    }

    loginView->show();
}