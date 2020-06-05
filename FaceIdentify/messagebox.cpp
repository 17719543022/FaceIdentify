#include "messagebox.h"
#include "ui_messagebox.h"
#include <QMessageBox>

MessageBox::MessageBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageBox)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->setupUi(this);

    ui->letGoIconLabel->hide();
    ui->letGoTextLabel->hide();
    ui->letStayIconLabel->hide();
    ui->letStayTextLabel->hide();
    ui->logInIconLabel->hide();
    ui->logInTextLabel->hide();
    ui->closeSysIconLabel->hide();
    ui->closeSysTextLabel->hide();
    ui->softwareIconLabel->hide();
    ui->softwareTextLabel->hide();
    ui->cheHuiIconLabel->hide();
    ui->cheHuiTextLabel->hide();
    ui->hitNullIconLabel->hide();
    ui->hitNullTextLabel->hide();
    ui->closeDivIconLabel->hide();
    ui->closeDivTextLabel->hide();
    ui->openDivIconLabel->hide();
    ui->openDivTextLabel->hide();
    ui->portErrorIconLabel->hide();
    ui->portErrorTextLabel->hide();
}

MessageBox::~MessageBox()
{
    delete ui;
}

void MessageBox::on_closePushButton_clicked()
{
    reject();
}

void MessageBox::on_noPushButton_clicked()
{
    reject();
}

void MessageBox::on_yesPushButton_clicked()
{
    accept();
}

void MessageBox::letGoTextLabelShow()
{
    ui->letGoTextLabel->show();
}

void MessageBox::letGoIconLabelShow()
{
    ui->letGoIconLabel->show();
}

void MessageBox::letGoTextLabelHide()
{
    ui->letGoTextLabel->hide();
}

void MessageBox::letGoIconLabelHide()
{
    ui->letGoIconLabel->hide();
}

void MessageBox::letStayTextLabelShow()
{
    ui->letStayTextLabel->show();
}

void MessageBox::letStayIconLabelShow()
{
    ui->letStayIconLabel->show();
}

void MessageBox::letStayTextLabelHide()
{
    ui->letStayTextLabel->hide();
}

void MessageBox::letStayIconLabelHide()
{
    ui->letStayIconLabel->hide();
}

void MessageBox::logInTextLabelShow()
{
    ui->logInTextLabel->show();
}

void MessageBox::logInIconLabelShow()
{
    ui->logInIconLabel->show();
}

void MessageBox::logInTextLabelHide()
{
    ui->logInTextLabel->hide();
}

void MessageBox::logInIconLabelHide()
{
    ui->logInIconLabel->hide();
}

void MessageBox::closeSysTextLabelShow()
{
    ui->closeSysTextLabel->show();
}

void MessageBox::closeSysIconLabelShow()
{
    ui->closeSysIconLabel->show();
}

void MessageBox::closeSysTextLabelHide()
{
    ui->closeSysTextLabel->hide();
}

void MessageBox::closeSysIconLabelHide()
{
    ui->closeSysIconLabel->hide();
}

void MessageBox::softwareTextLabelShow()
{
    ui->softwareTextLabel->show();
}

void MessageBox::softwareIconLabelShow()
{
    ui->softwareIconLabel->show();
}

void MessageBox::softwareTextLabelHide()
{
    ui->softwareTextLabel->hide();
}

void MessageBox::softwareIconLabelHide()
{
    ui->softwareIconLabel->hide();
}

void MessageBox::cheHuiTextLabelShow()
{
    ui->cheHuiTextLabel->show();
}

void MessageBox::cheHuiIconLabelShow()
{
    ui->cheHuiIconLabel->show();
}

void MessageBox::cheHuiTextLabelHide()
{
    ui->cheHuiTextLabel->hide();
}

void MessageBox::cheHuiIconLabelHide()
{
    ui->cheHuiIconLabel->hide();
}

void MessageBox::hitNullIconLabelShow()
{
    ui->hitNullIconLabel->show();
}

void MessageBox::hitNullTextLabelShow()
{
    ui->hitNullTextLabel->show();
}

void MessageBox::hitNullIconLabelHide()
{
    ui->hitNullIconLabel->hide();
}

void MessageBox::hitNullTextLabelHide()
{
    ui->hitNullTextLabel->hide();
}

void MessageBox::closeDivIconLabelShow()
{
    ui->closeDivIconLabel->show();
}

void MessageBox::closeDivTextLabelShow()
{
    ui->closeDivTextLabel->show();
}

void MessageBox::closeDivIconLabelHide()
{
    ui->closeDivIconLabel->hide();
}

void MessageBox::closeDivTextLabelHide()
{
    ui->closeDivTextLabel->hide();
}

void MessageBox::openDivIconLabelShow()
{
    ui->openDivIconLabel->show();
}

void MessageBox::openDivTextLabelShow()
{
    ui->openDivTextLabel->show();
}

void MessageBox::openDivIconLabelHide()
{
    ui->openDivIconLabel->hide();
}

void MessageBox::openDivTextLabelHide()
{
    ui->openDivTextLabel->hide();
}

void MessageBox::portErrorIconLabelShow()
{
    ui->portErrorIconLabel->show();
}

void MessageBox::portErrorTextLabelShow()
{
    ui->portErrorTextLabel->show();
}

void MessageBox::portErrorIconLabelHide()
{
    ui->portErrorIconLabel->hide();
}

void MessageBox::portErrorTextLabelHide()
{
    ui->portErrorTextLabel->hide();
}
