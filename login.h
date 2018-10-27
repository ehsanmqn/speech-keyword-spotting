#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();

private:
    Ui::login *ui;

    QSqlDatabase db;
    void dialogIsFinished(int result);

private slots:
    void onLoginButtonClicked();
    void alternateLoginButton();

protected:
    void closeEvent(QCloseEvent *e);
    void successfulLogin();

signals:
    void closing(bool visible);
    void success(bool visible);
};
#endif // LOGIN_H
