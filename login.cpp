#include "login.h"
#include "ui_login.h"
#include "vajegangui.h"

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);

//    Qt::WindowFlags flags = windowFlags();
//    Qt::WindowFlags closeFlag = Qt::WindowCloseButtonHint;
//    flags = flags & (~closeFlag);
//    setWindowFlags(flags);

    setModal(false);

    // Initial database
    const QString DRIVER("QSQLITE");
    db = QSqlDatabase::addDatabase(DRIVER);

    QPixmap pix(":/loginimg/img.png");
    int w = ui->imageLabel->width();
    int h = ui->imageLabel->height();
    qDebug() << w << h;
    ui->imageLabel->setPixmap(pix);

    QPixmap icon(":/icon/voice.png");
    this->setWindowIcon(QIcon(icon));

    ui->loginButton->setEnabled(false);

    ui->passLineEdit->setEchoMode(QLineEdit::Password);

    connect(ui->loginButton, SIGNAL(clicked(bool)), this, SLOT(onLoginButtonClicked()));
    connect(ui->userLineEdit, SIGNAL(textChanged(QString)), this, SLOT(alternateLoginButton()));
    connect(ui->passLineEdit, SIGNAL(textChanged(QString)), this, SLOT(alternateLoginButton()));
}

login::~login()
{
    delete ui;
}

void login::onLoginButtonClicked()
{
    QString username = ui->userLineEdit->text();
    QString password = ui->passLineEdit->text();

    const QString DRIVER("QSQLITE");

    // connect to quotes
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db.setDatabaseName("libinit");

        if(!db.open())
            qWarning() << "DatabaseConnect - ERROR: " << db.lastError().text();
    }
    else
        qWarning() << "DatabaseConnect - ERROR: no driver " << DRIVER << " available";

    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = ?");
    query.addBindValue(username);

    if(!query.exec())
        qDebug() << "Login - ERROR: " << query.lastError().text();
    else
    {
        if(query.last())
        {
            if( query.value(1).toString() == username)
            {
                if(query.value(2).toString() == password)
                {
                    qDebug() << "Ehsan";
//                    VajeganGUI *w = new VajeganGUI(this);
                    this->hide();
                    successfulLogin();
//                    this->setHidden(true);
//                    w->showMaximized();
                }
                else
                {
                    QMessageBox::information(
                        this,
                        tr("خطای ورود"),
                        tr("کلمه عبور اشتباه است"));
                }
            }
        }
        else
        {
            QMessageBox::information(
                this,
                tr("خطای ورود"),
                tr("نام کاربری یافت نشد") );
        }
    }

    //    if(usernme == "admin" && password == "admin")
    //    {

    //        VajeganGUI *w = new VajeganGUI(this);
    //        //            w->show();
    //        this->hide();
    //        w->showMaximized();
    //    }
    //    else
    //    {
    //        //        QMessageBox::warning(this,"Login", "Username and password is not correct");
    //    }
}

void login::alternateLoginButton()
{
    if(ui->userLineEdit->text() != "" && ui->passLineEdit->text() != "")
        ui->loginButton->setEnabled(true);
    else
        ui->loginButton->setEnabled(false);
}

void login::closeEvent(QCloseEvent *e)
{
    emit closing(true);
    QDialog::closeEvent(e);
}

void login::successfulLogin()
{
    emit success(true);
}

//void login::closeEvent (QCloseEvent *event)
//{
//    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Vzheyab",
//                                                                tr("Are you sure?\n"),
//                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
//                                                                QMessageBox::Yes);
//    if (resBtn != QMessageBox::Yes) {
//        event->ignore();
//    } else {
//        event->accept();
//        emit closing(true);
//        QDialog::closeEvent(event);
//    }
//}
