#include "messageboxfx.h"
#include "ui_messageboxfx.h"

MessageBoxFX::MessageBoxFX(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageBoxFX)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->setupUi(this);
}

MessageBoxFX::~MessageBoxFX()
{
    delete ui;
}

void MessageBoxFX::imageShow(const QPixmap& pixmap)
{
    ui->zhaoLabel->setPixmap(pixmap.scaled(ui->zhaoLabel->width()
                                           , ui->zhaoLabel->height()
                                           , Qt::IgnoreAspectRatio
                                           , Qt::SmoothTransformation));
}

void MessageBoxFX::on_closePushButton_clicked()
{
    reject();
}

void MessageBoxFX::on_noPushButton_clicked()
{
    reject();
}

void MessageBoxFX::on_yesPushButton_clicked()
{
    accept();
}
