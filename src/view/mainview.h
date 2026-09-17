#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QMainWindow>

class QPushButton;
class LoginView;

class MainView : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainView(QWidget *parent = nullptr);

private slots:
    void openLoginView();

private:
    QPushButton *loginButton;

    LoginView *loginView;
};

#endif