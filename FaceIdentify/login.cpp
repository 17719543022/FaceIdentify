#include "login.h"
#include "ui_login.h"
#include <QDebug>
#include "messagebox.h"
#include "Include/singleton.h"

LogIn::LogIn(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogIn)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->setupUi(this);
}

LogIn::~LogIn()
{
    delete ui;
}

void LogIn::on_guanBiPushButton_clicked()
{
    close();
}

void LogIn::on_dengLuPushButton_clicked()
{
    QString zhanghao = ui->zhangHaoLineEdit->text();
    QString mima = ui->miMaLineEdit->text();

    // 读ini value时,需要按"/Section Name/Key Name"的格式
    QString passwd = Singleton::getInstance().getConfigQtIni()->value("/accounts/" + zhanghao).toString();

    if (mima == passwd && mima != QString()) {
        close();

        emit logInSuccess();
    } else {
        MessageBox mbx;

        mbx.logInIconLabelShow();
        mbx.logInTextLabelShow();

        mbx.exec();
    }
}
