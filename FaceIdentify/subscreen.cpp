#include "subscreen.h"
#include "ui_subscreen.h"
#include <QTimer>
#include <QPainter>

SubScreen::SubScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubScreen)
{
    ui->setupUi(this);
    window()->showFullScreen();

    QTimer *timer = new QTimer(this);
    timer->start(5000);

    animationConnection = connect(timer, SIGNAL(timeout()), this, SLOT(animate()));

    QImage img;
    img.load(":/Images/SecondWindow/TuCeng-8.png");
    img = img.scaled(ui->animationLabel->width()
                     , ui->animationLabel->height()
                     , Qt::IgnoreAspectRatio
                     , Qt::SmoothTransformation);
    ui->animationLabel->setPixmap(QPixmap::fromImage(img));
}

SubScreen::~SubScreen()
{
    if (animationConnection != nullptr) {
        disconnect(animationConnection);
    }

    delete ui;
}

void SubScreen::updateVideoFrame(const QPixmap& pixmap)
{
    ui->videoFrame->setPixmap(pixmap.scaled(ui->videoFrame->width()
                                            , ui->videoFrame->height()
                                            , Qt::IgnoreAspectRatio
                                            , Qt::SmoothTransformation));
}

void SubScreen::animate()
{
    QImage img;
    static int count = 0;

    count += 1;
    switch (count%8) {
    case 0:
        img.load(":/Images/SecondWindow/TuCeng-1.png");
        break;
    case 1:
        img.load(":/Images/SecondWindow/TuCeng-2.png");
        break;
    case 2:
        img.load(":/Images/SecondWindow/TuCeng-3.png");
        break;
    case 3:
        img.load(":/Images/SecondWindow/TuCeng-4.png");
        break;
    case 4:
        img.load(":/Images/SecondWindow/TuCeng-5.png");
        break;
    case 5:
        img.load(":/Images/SecondWindow/TuCeng-6.png");
        break;
    case 6:
        img.load(":/Images/SecondWindow/TuCeng-7.png");
        break;
    case 7:
        img.load(":/Images/SecondWindow/TuCeng-8.png");
        break;
    default:
        break;
    }

    img = img.scaled(ui->animationLabel->width()
                     , ui->animationLabel->height()
                     , Qt::IgnoreAspectRatio
                     , Qt::SmoothTransformation);
    ui->animationLabel->setPixmap(QPixmap::fromImage(img));
}
