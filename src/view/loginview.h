#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class LoginView : public QWidget
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parent = nullptr);

private slots:
    void onLoginClicked();
    void onShowPasswordClicked();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;

    QPushButton *loginButton;
    QPushButton *showPasswordButton;
};

#endif