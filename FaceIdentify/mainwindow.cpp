#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Include/functions.h"
#include "Include/singleton.h"
#include "FaceIdentifySDK.h"
#include "ParamDefine.h"
#include <QPainter>
#include <QUuid>
#include <QSqlQuery>
#include <QtDebug>
#include <QBuffer>
#include <QFile>
#include "login.h"
#include <QSqlQueryModel>
#include <QSqlRecord>
#include "messagebox.h"
#include "messageboxfx.h"
#include "subscreen.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    vpixmap(QPixmap()),
    timer(new QTimer(this)),
    optimumFace(nullptr),
    flightInfo(FlightInfo()),
    shotTime(QString()),
    faceNum(-1),
    status(-1),
    pSubScreen(new SubScreen()),
    isFaceDetectVideoChanged(false),
    isFrontPageByPassed(false),
    isInDiversionMode(false),
    queryModel(new QSqlQueryModel),
    rowCountPerPage(16),
    rowCount(0),
    pageCount(1),
    currentPage(1),
    currentRows(0)
{
    ui->setupUi(this);
    window()->showFullScreen();

    ui->frontPageWidget->show();
    ui->backPageWidget->hide();
    ui->downcount->hide();

    ui->liShiTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->liShiTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->liShiTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->liShiTableWidget->verticalHeader()->setVisible(false);
    ui->liShiTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->liShiTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->liShiTableWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
    ui->liShiTableWidget->setShowGrid(false);
    ui->liShiTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color: rgba(0, 0, 0, 0);"
                                                            " border: 0;"
                                                            " border-bottom: 1px solid #094b57;"
                                                            " color: white; }");

    ui->liShiTableWidget->setColumnWidth(0, 62);
    ui->liShiTableWidget->setColumnWidth(1, 130);
    ui->liShiTableWidget->setColumnWidth(2, 62);
    ui->liShiTableWidget->setColumnWidth(3, 80);
    ui->liShiTableWidget->setColumnWidth(4, 62);

    ui->fdisplayWidget->hide();
    ui->fangXingPushButton->hide();

    dataAnalysis = new DataAnalysis();
    dataAnalysis->RegistCallback();

    video1Connection = connect(dataAnalysis, &DataAnalysis::video1Detect, this, &MainWindow::video1Show);
    video2Connection = connect(dataAnalysis, &DataAnalysis::video2Detect, this, &MainWindow::video2Show);
    idCardConnection = connect(dataAnalysis, &DataAnalysis::idCardResponse, this, &MainWindow::idCardResponse);
    flightConnection = connect(dataAnalysis, &DataAnalysis::flightResponse, this, &MainWindow::flightResponse);
    tableConnection = connect(ui->liShiTableWidget, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(on_liShiTableWidget_itemClicked(QTableWidgetItem *)));

    CoreFIInitBestFaceList(&optimumFace);
}

MainWindow::~MainWindow()
{
    if (dataAnalysis != nullptr) {
        dataAnalysis = nullptr;
    }

    if (video1Connection != nullptr) {
        disconnect(video1Connection);
    }

    if (video2Connection != nullptr) {
        disconnect(video2Connection);
    }

    if (idCardConnection != nullptr) {
        disconnect(idCardConnection);
    }

    if (flightConnection != nullptr) {
        disconnect(flightConnection);
    }

    if (tableConnection != nullptr) {
        disconnect(tableConnection);
    }

    if (optimumFace != nullptr) {
        CoreFIReleaseBestFaceList(&optimumFace);
        optimumFace = nullptr;
    }

    delete ui;
}

void MainWindow::video1Show(const QImage &image, const QVector<QRect> &facePos)
{
    pSubScreen->show();

    QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());
    vpixmap = pixmap;

    for (int i=0; i<facePos.size(); i++) {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen(Qt::green);
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawRect(facePos[i]);
    }
    if (isFaceDetectVideoChanged) {
        ui->video2PushButton->setIcon(QIcon(pixmap.scaled(ui->video2PushButton->width()
                                                          , ui->video2PushButton->height()
                                                          , Qt::IgnoreAspectRatio
                                                          , Qt::SmoothTransformation)));
        ui->video2PushButton->setIconSize(QSize(109, 81));
    } else {
        pSubScreen->updateVideoFrame(pixmap);
        QImage img = pixmap.toImage().scaled(ui->video1Label->width()
                                             , ui->video1Label->height()
                                             , Qt::IgnoreAspectRatio
                                             , Qt::SmoothTransformation);
        ui->video1Label->setPixmap(QPixmap::fromImage(img));
    }
}

void MainWindow::video2Show(const QImage &image, const QVector<QRect> &facePos)
{
    QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());

    for (int i=0; i<facePos.size(); i++) {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen(Qt::green);
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawRect(facePos[i]);
    }
    if (isFaceDetectVideoChanged) {
        pSubScreen->updateVideoFrame(pixmap);
        QImage img = pixmap.toImage().scaled(ui->video1Label->width()
                                             , ui->video1Label->height()
                                             , Qt::IgnoreAspectRatio
                                             , Qt::SmoothTransformation);
        ui->video1Label->setPixmap(QPixmap::fromImage(img));
    } else {
        ui->video2PushButton->setIcon(QIcon(pixmap.scaled(ui->video2PushButton->width()
                                                          , ui->video2PushButton->height()
                                                          , Qt::IgnoreAspectRatio
                                                          , Qt::SmoothTransformation)));
        ui->video2PushButton->setIconSize(QSize(109, 81));
    }
}

void MainWindow::idCardResponse(const FaceAndIdResult& faiResult)
{
    if (!isFrontPageByPassed) {
        ui->displayWidget->show();
        ui->fdisplayWidget->hide();

        if (faiResult.matchResult == 3) {
            MessageBox mbx;

            mbx.softwareIconLabelShow();
            mbx.softwareTextLabelShow();

            mbx.exec();

            return;
        }

        if (faiResult.matchStatus == STATUS_DETECT_NO_FACE || faiResult.matchStatus == STATUS_COMPARE_FAIL)
        {
            ui->fangXingPushButton->show();
        } else {
            ui->fangXingPushButton->hide();
        }

        updateIdCard(faiResult);
        displayIdCard(faiResult);
        databaseIdCard(faiResult);
    }
}

void MainWindow::on_guanBiPushButton_clicked()
{
    isFrontPageByPassed = true;

    MessageBox mbx;

    connect(&mbx, SIGNAL(finished(int)), this, SLOT(setFrontPageNotByPassed(int)));

    mbx.closeSysIconLabelShow();
    mbx.closeSysTextLabelShow();

    if (mbx.exec() == QDialog::Accepted) {
        pSubScreen->hide();
        close();
    }
}

void MainWindow::on_liShiTableWidget_itemClicked(QTableWidgetItem *item)
{
    int row = item->row();

    if ((faiResultVector.at(row).matchStatus == STATUS_DETECT_NO_FACE || faiResultVector.at(row).matchStatus == STATUS_COMPARE_FAIL)
            && row == 0)
    {
        ui->fangXingPushButton->show();
    } else {
        ui->fangXingPushButton->hide();
    }

    displayIdCard(faiResultVector.at(row));
}

void MainWindow::on_huiChaPushButton_clicked()
{
    isFrontPageByPassed = true;

    QueryDialog dialog;

    connect(&dialog, SIGNAL(finished(int)), this, SLOT(setFrontPageNotByPassed(int)));
    connect(dataAnalysis, SIGNAL(idCardResponse(const FaceAndIdResult&)), &dialog, SLOT(queryByIdCardSwipe(const FaceAndIdResult&)));
    connect(dataAnalysis, SIGNAL(flightResponse(const FlightResult&)), &dialog, SLOT(queryByFlightSwipe(const FlightResult&)));

    dialog.exec();
}

void MainWindow::switchToBackPage()
{
    isFrontPageByPassed = true;

    updatePageLabel();

    ui->bshuJuTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bshuJuTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bshuJuTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->bshuJuTableWidget->verticalHeader()->setVisible(false);
    ui->bshuJuTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->bshuJuTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->bshuJuTableWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
    ui->bshuJuTableWidget->setShowGrid(false);
    ui->bshuJuTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color: rgba(0, 0, 0, 0); border: 0; border-bottom: 1px solid #094b57; color: white; }");

    ui->bshuJuTableWidget->setColumnWidth(0, 62);
    ui->bshuJuTableWidget->setColumnWidth(1, 130);
    ui->bshuJuTableWidget->setColumnWidth(2, 62);
    ui->bshuJuTableWidget->setColumnWidth(3, 79);
    ui->bshuJuTableWidget->setColumnWidth(4, 58);

    ui->frontPageWidget->hide();
    ui->backPageWidget->show();
    ui->bfdisplayWidget->hide();
    ui->bfoperateWidget->hide();
    ui->bdisplayWidget->show();
    ui->boperateWidget->show();
}

void MainWindow::switchToDiversion()
{
    isFrontPageByPassed = true;

    if (!isInDiversionMode) {
        MessageBox mbx;

        connect(&mbx, SIGNAL(finished(int)), this, SLOT(setFrontPageNotByPassed(int)));

        mbx.openDivIconLabelShow();
        mbx.openDivTextLabelShow();

        if (mbx.exec() == QDialog::Accepted) {
            isInDiversionMode = true;

            QTimer *timer = new QTimer(this);
            timer->start(1000);
            secondCounts = 3600;

            QString hour = (secondCounts/3600 < 10) ? tr("0") + QString::number(secondCounts/3600) : QString::number(secondCounts/3600);
            QString minute = ((secondCounts/60)%60 < 10) ? tr("0") + QString::number((secondCounts/60)%60) : QString::number((secondCounts/60)%60);
            QString second = (secondCounts%60 < 10) ? tr("0") + QString::number(secondCounts%60) : QString::number(secondCounts%60);
            ui->downcount->show();
            ui->downcount->setText(hour + tr(":") + minute + tr(":") + second);

            connect(timer, SIGNAL(timeout()), this, SLOT(countdown()));
        }
    } else {
        MessageBox mbx;

        connect(&mbx, SIGNAL(finished(int)), this, SLOT(setFrontPageNotByPassed(int)));

        mbx.closeDivIconLabelShow();
        mbx.closeDivTextLabelShow();

        if (mbx.exec() == QDialog::Accepted) {
            isInDiversionMode = false;

            ui->downcount->hide();
        }
    }
}

void MainWindow::setFrontPageNotByPassed(int)
{
    isFrontPageByPassed = false;
}

void MainWindow::on_settingPushButton_clicked()
{
    isFrontPageByPassed = true;

    LogIn logIn;

    connect(&logIn, SIGNAL(finished(int)), this, SLOT(setFrontPageNotByPassed(int)));
    connect(&logIn, SIGNAL(logInSuccess()), this, SLOT(switchToBackPage()));

    logIn.exec();
}

void MainWindow::on_beiJiangPushButton_clicked()
{
    isFrontPageByPassed = true;

    LogIn logIn;

    connect(&logIn, SIGNAL(finished(int)), this, SLOT(setFrontPageNotByPassed(int)));
    connect(&logIn, SIGNAL(logInSuccess()), this, SLOT(switchToDiversion()));

    logIn.exec();
}

void MainWindow::countdown()
{
    secondCounts -= 1;

    if (secondCounts <= 0) {
        ui->downcount->hide();
    } else {
        QString hour = (secondCounts/3600 < 10) ? tr("0") + QString::number(secondCounts/3600) : QString::number(secondCounts/3600);
        QString minute = ((secondCounts/60)%60 < 10) ? tr("0") + QString::number((secondCounts/60)%60) : QString::number((secondCounts/60)%60);
        QString second = (secondCounts%60 < 10) ? tr("0") + QString::number(secondCounts%60) : QString::number(secondCounts%60);
        ui->downcount->setText(hour + tr(":") + minute + tr(":") + second);
    }
}

void MainWindow::oncedown()
{
    ui->fdisplayWidget->hide();
    ui->displayWidget->show();
}

//////////////////////////////////////////////////////
/// \brief FaceAndIdResult.stampResult     mainwindow.ui->liShiTableWidget     backstage.ui->tiaoJianComboBox
/// \param STATUS_COMPARE_PASS             比对通过                             系统验证通过
/// \param STATUS_COMPARE_FAIL             比对不通过                            系统验证不通过
/// \param STATUS_DETECT_NO_FACE           无人脸信息                            未检测到人脸
/// \param STATUS_DETECT_NO_FACE           无人脸信息                            未检测到人脸
/// \param STATUS_ARTIFICIAL_PASS          已人工放行                            人工验证通过
/// \param STATUS_IDCARD_EXPIRE            无效                                 证件失效
/// \param STATUS_DEFAULT                  默认值                               默认值
//////////////////////////////////////////////////////

void MainWindow::updateIdCard(const FaceAndIdResult& faiResult)
{
    if (faiResultVector.count() > 200) {
        faiResultVector.remove(199);
    }

    faiResultVector.insert(faiResultVector.begin(), faiResult);

    QFont font;
    font.setPointSize(9);

    ui->liShiTableWidget->setRowHeight(0, 18);
    ui->liShiTableWidget->insertRow(0);
    ui->liShiTableWidget->setRowHeight(0, 18);
    QTableWidgetItem *item0 = new QTableWidgetItem(faiResult.name);
    item0->setFont(font);
    item0->setForeground(QBrush(Qt::white));
    item0->setTextAlignment(Qt::AlignCenter);
    ui->liShiTableWidget->setItem(0, 0, item0);

    QTableWidgetItem *item1 = new QTableWidgetItem(faiResult.matchTime);
    item1->setFont(font);
    item1->setForeground(QBrush(Qt::white));
    item1->setTextAlignment(Qt::AlignCenter);
    ui->liShiTableWidget->setItem(0, 1, item1);

    QTableWidgetItem *item2 = new QTableWidgetItem(QString("%1").arg(double(faiResult.matchScore)));
    item2->setFont(font);
    item2->setForeground(QBrush(Qt::white));
    item2->setTextAlignment(Qt::AlignCenter);
    ui->liShiTableWidget->setItem(0, 2, item2);

    if ((faiResult.matchStatus & 0xf) == STATUS_COMPARE_PASS) {
        QTableWidgetItem *item3 = new QTableWidgetItem(tr("比对通过"));
        item3->setFont(font);
        item3->setForeground(QBrush(Qt::green));
        item3->setTextAlignment(Qt::AlignCenter);
        ui->liShiTableWidget->setItem(0, 3,item3);
    } else if ((faiResult.matchStatus & 0xf) == STATUS_COMPARE_FAIL) {
        QTableWidgetItem *item3 = new QTableWidgetItem(tr("比对不通过"));
        item3->setFont(font);
        item3->setForeground(QBrush(Qt::red));
        item3->setTextAlignment(Qt::AlignCenter);
        ui->liShiTableWidget->setItem(0, 3, item3);
    } else if ((faiResult.matchStatus & 0xf) == STATUS_DETECT_NO_FACE) {
        QTableWidgetItem *item3 = new QTableWidgetItem(tr("无人脸信息"));
        item3->setFont(font);
        item3->setForeground(QBrush(Qt::red));
        item3->setTextAlignment(Qt::AlignCenter);
        ui->liShiTableWidget->setItem(0, 3, item3);
    } else if ((faiResult.matchStatus & 0xf) == STATUS_ARTIFICIAL_PASS) {
        QTableWidgetItem *item3 = new QTableWidgetItem(tr("已人工放行"));
        item3->setFont(font);
        item3->setForeground(QBrush(Qt::red));
        item3->setTextAlignment(Qt::AlignCenter);
        ui->liShiTableWidget->setItem(0, 3, item3);
    } else if ((faiResult.matchStatus & 0xf) == STATUS_DEFAULT) {
        qCritical() << "Error! faiResult Not Set.";
    }

    if (faiResult.matchStatus & STATUS_IDCARD_EXPIRE) {
        QTableWidgetItem *item4 = new QTableWidgetItem(tr("无效"));
        item4->setFont(font);
        item4->setForeground(QBrush(Qt::red));
        item4->setTextAlignment(Qt::AlignCenter);
        ui->liShiTableWidget->setItem(0, 4, item4);
    } else {
        QTableWidgetItem *item4 = new QTableWidgetItem(tr("有效"));
        item4->setFont(font);
        item4->setForeground(QBrush(Qt::green));
        item4->setTextAlignment(Qt::AlignCenter);
        ui->liShiTableWidget->setItem(0, 4, item4);
    }
    ui->liShiTableWidget->setRowHeight(0, 18);
}

void MainWindow::displayIdCard(const FaceAndIdResult& faiResult)
{
    ui->nameLabel->setText(tr("姓名：") + faiResult.name);
    ui->nationalityLabel->setText(tr("民族：") + faiResult.folk);
    ui->idCardLabel->setText(tr("身份证号：") + faiResult.code);
    ui->expireTimeLabel->setText(tr("有效期限：") + faiResult.expireStart+tr("-")+ faiResult.expireEnd);
    ui->compareLabel->setText(tr("比对分值：") + QString::number(double(faiResult.matchScore)));
    if (faiResult.matchStatus == STATUS_COMPARE_PASS) {
        ui->compareLabel->setStyleSheet("border: 0; background: 0; color: green; image: 0;");
    } else {
        ui->compareLabel->setStyleSheet("border: 0; background: 0; color: red; image: 0;");
    }

    if (!faiResult.idImage.isNull()) {
        QImage img = faiResult.idImage.scaled(ui->zhengLabel->width()
                                             , ui->zhengLabel->height()
                                             , Qt::IgnoreAspectRatio
                                             , Qt::SmoothTransformation);
        ui->zhengLabel->setPixmap(QPixmap::fromImage(img));
    } else {
        ui->zhengLabel->clear();
    }

    if (!faiResult.liveImage.isNull()) {
        QImage img = faiResult.liveImage.scaled(ui->zhaoLabel->width()
                                               , ui->zhaoLabel->height()
                                               , Qt::IgnoreAspectRatio
                                               , Qt::SmoothTransformation);
        ui->zhaoLabel->setPixmap(QPixmap::fromImage(img));
    } else {
        ui->zhaoLabel->clear();
    }

    if (faiResult.matchStatus & STATUS_IDCARD_EXPIRE) {
        QImage img;
        img.load(":/Images/RenZhengBiDui/seal_invalid.png");
        img = img.scaled(ui->stampLabel->width()
                         , ui->stampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->stampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((faiResult.matchStatus & 0xf) == STATUS_COMPARE_PASS) {
        QImage img;
        img.load(":/Images/RenZhengBiDui/YanZhengTongGuo.png");
        img = img.scaled(ui->stampLabel->width()
                         , ui->stampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->stampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((faiResult.matchStatus & 0xf) == STATUS_COMPARE_FAIL) {
        QImage img;
        img.load(":/Images/RenZhengBiDui/BiDuiBuTongGuo.png");
        img = img.scaled(ui->stampLabel->width()
                         , ui->stampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->stampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((faiResult.matchStatus & 0xf) == STATUS_ARTIFICIAL_PASS) {
        QImage img;
        img.load(":/Images/LiShiJiLu/FangXingTongGuo.png");
        img = img.scaled(ui->stampLabel->width()
                         , ui->stampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->stampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((faiResult.matchStatus & 0xf) == STATUS_DETECT_NO_FACE) {
        QImage img;
        img.load(":/Images/LiShiJiLu/seal_noface.png");
        img = img.scaled(ui->stampLabel->width()
                         , ui->stampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->stampLabel->setPixmap(QPixmap::fromImage(img));

        // 同时需要将zhaoLabel的照片更新为WeiJianCeDaoRenNian.png
        QImage zhao;
        zhao.load(":/Images/LiShiJiLu/WeiJianCeDaoRenNian.png");
        zhao = zhao.scaled(ui->zhaoLabel->width()
                           , ui->zhaoLabel->height()
                           , Qt::IgnoreAspectRatio
                           , Qt::SmoothTransformation);
        ui->zhaoLabel->setPixmap(QPixmap::fromImage(zhao));
    } else if ((faiResult.matchStatus & 0xf) == STATUS_DEFAULT) {
        ui->stampLabel->clear();
        qCritical() << "Error! faiResult Not Set.";
    }
}

void MainWindow::databaseIdCard(const FaceAndIdResult& faiResult)
{
    QSqlQuery query;

    query.prepare("INSERT INTO person (id"
                  ", passagewayId"
                  ", idType"
                  ", name"
                  ", eName"
                  ", gender"
                  ", folk"
                  ", birthDay"
                  ", code"
                  ", address"
                  ", agency"
                  ", expireStart"
                  ", expireEnd"
                  ", prCode"
                  ", nation"
                  ", idVersion"
                  ", agencyCode"
                  ", idImageType"
                  ", idImage"
                  ", liveImage"
                  ", liveImageAge"
                  ", liveImageGender"
                  ", matchResult"
                  ", matchScore"
                  ", matchStatus"
                  ", matchScoreThreshold"
                  ", matchTime) "
                  "VALUES (:id"
                  ", :passagewayId"
                  ", :idType"
                  ", :name"
                  ", :eName"
                  ", :gender"
                  ", :folk"
                  ", :birthDay"
                  ", :code"
                  ", :address"
                  ", :agency"
                  ", :expireStart"
                  ", :expireEnd"
                  ", :prCode"
                  ", :nation"
                  ", :idVersion"
                  ", :agencyCode"
                  ", :idImageType"
                  ", :idImage"
                  ", :liveImage"
                  ", :liveImageAge"
                  ", :liveImageGender"
                  ", :matchResult"
                  ", :matchScore"
                  ", :matchStatus"
                  ", :matchScoreThreshold"
                  ", :matchTime)");

    QString uuid = QUuid::createUuid().toString();
    uuid.remove("{").remove("}").remove("-");
    query.bindValue(":id", uuid);
    query.bindValue(":passagewayId", Singleton::getInstance().getSystemParamXml()["RecogLocationID"].toInt());
    query.bindValue(":idType", faiResult.idType);
    query.bindValue(":name", faiResult.name);
    query.bindValue(":eName", faiResult.eName);
    query.bindValue(":gender", faiResult.gender);
    query.bindValue(":folk", faiResult.folk);
    query.bindValue(":birthDay", faiResult.birthDay);
    query.bindValue(":code", faiResult.code);
    query.bindValue(":address", faiResult.address);
    query.bindValue(":agency", faiResult.agency);
    query.bindValue(":expireStart", faiResult.expireStart);
    query.bindValue(":expireEnd", faiResult.expireEnd);
    query.bindValue(":isExpire", faiResult.isExpire);
    query.bindValue(":prCode", faiResult.prCode);
    query.bindValue(":nation", faiResult.nation);
    query.bindValue(":idVersion", faiResult.idVersion);
    query.bindValue(":agencyCode", faiResult.agencyCode);
    query.bindValue(":idImageType", faiResult.idImageType);
    QByteArray idImageByteArray = fillWithQImage(faiResult.idImage);
    query.bindValue(":idImage", idImageByteArray);
    QByteArray liveImageByteArray = fillWithQImage(faiResult.liveImage);
    query.bindValue(":liveImage", liveImageByteArray);
    query.bindValue(":liveImageAge", faiResult.liveImageAge);
    query.bindValue(":liveImageGender", faiResult.liveImageGender);
    query.bindValue(":matchResult", faiResult.matchResult);
    query.bindValue(":matchScore", faiResult.matchScore);
    query.bindValue(":matchStatus", faiResult.matchStatus);
    query.bindValue(":matchScoreThreshold", Singleton::getInstance().getSystemParamXml()["RecogThreshold"].toFloat());
    query.bindValue(":matchTime", faiResult.matchTime);

    query.exec();
}

void MainWindow::on_fangXingPushButton_clicked()
{
    if (faiResultVector[0].matchStatus == STATUS_DETECT_NO_FACE) {
        MessageBoxFX fx;

        fx.imageShow(vpixmap);

        if (fx.exec() == QDialog::Accepted) {
            QNetworkRequest request;
            QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

            connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(nofaceFangXing(QNetworkReply*)));

            // Header
            QString securityManualOptCheck = Singleton::getInstance().getConfigQtIni()->value("/interfaces/securityManualOptCheck").toString();
            request.setUrl(QUrl(securityManualOptCheck));
            QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
            request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
            QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
            request.setRawHeader("apiId", apiId.toLatin1());
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
            request.setRawHeader("timestamp", timestamp.toLatin1());
            QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
            QString temp = securityManualOptCheck.mid(securityManualOptCheck.indexOf("/api/v1")) + timestamp + apiKey;
            QByteArray bb = QCryptographicHash::hash(temp.toLatin1(), QCryptographicHash::Md5);
            QString sign = QString().append(bb.toHex());
            request.setRawHeader("sign", sign.toLatin1());

            // Body
            QJsonObject json;
            QJsonDocument doc;
            QString uuid = QUuid::createUuid().toString();
            uuid.remove("{").remove("}").remove("-");
            json.insert("reqId", uuid);
            QString gateNo = Singleton::getInstance().getConfigIni()->value("/device/gateNo").toString();
            json.insert("gateNo", gateNo);
            QString deviceId = Singleton::getInstance().getConfigIni()->value("/device/deviceId").toString();
            json.insert("deviceId", deviceId);
            json.insert("cardType", 0);
            json.insert("idCard", faiResultVector[0].code);
            json.insert("birthDate", faiResultVector[0].birthDay);
            json.insert("certificateValidity", faiResultVector[0].expireStart + "-" + faiResultVector[0].expireEnd);
            json.insert("nationality", faiResultVector[0].nation);
            json.insert("ethnic", faiResultVector[0].folk);
            QByteArray byteArray = fillWithQPixmap(vpixmap);
            json.insert("scenePhoto", QString(byteArray.toBase64()));
            byteArray = fillWithQImage(faiResultVector[0].idImage);
            json.insert("cardPhoto", QString(byteArray.toBase64()));
            json.insert("cardFeature", faiResultVector[0].cardFeature);

            doc.setObject(json);
            QByteArray array = doc.toJson(QJsonDocument::Compact);

            naManager->post(request, array);
        }
    } else if (faiResultVector[0].matchStatus == STATUS_COMPARE_FAIL) {
        MessageBox mbx;

        mbx.letGoIconLabelShow();
        mbx.letGoTextLabelShow();

        if (mbx.exec() == QDialog::Accepted) {
            QNetworkRequest request;
            QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

            connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(comparefailFangXing(QNetworkReply*)));

            // Header
            QString securityManualOptCheck = Singleton::getInstance().getConfigQtIni()->value("/interfaces/securityManualOptCheck").toString();
            request.setUrl(QUrl(securityManualOptCheck));
            QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
            request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
            QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
            request.setRawHeader("apiId", apiId.toLatin1());
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
            request.setRawHeader("timestamp", timestamp.toLatin1());
            QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
            QString temp = securityManualOptCheck.mid(securityManualOptCheck.indexOf("/api/v1")) + timestamp + apiKey;
            QByteArray bb = QCryptographicHash::hash(temp.toLatin1(), QCryptographicHash::Md5);
            QString sign = QString().append(bb.toHex());
            request.setRawHeader("sign", sign.toLatin1());

            // Body
            QJsonObject json;
            QJsonDocument doc;
            QString uuid = QUuid::createUuid().toString();
            uuid.remove("{").remove("}").remove("-");
            json.insert("reqId", uuid);
            QString gateNo = Singleton::getInstance().getConfigIni()->value("/device/gateNo").toString();
            json.insert("gateNo", gateNo);
            QString deviceId = Singleton::getInstance().getConfigIni()->value("/device/deviceId").toString();
            json.insert("deviceId", deviceId);
            json.insert("cardType", 0);
            json.insert("idCard", faiResultVector[0].code);
            json.insert("birthDate", faiResultVector[0].birthDay);
            json.insert("certificateValidity", faiResultVector[0].expireStart + "-" + faiResultVector[0].expireEnd);
            json.insert("nationality", faiResultVector[0].nation);
            json.insert("ethnic", faiResultVector[0].folk);
            QByteArray byteArray = fillWithQImage(faiResultVector[0].liveImage);
            json.insert("scenePhoto", QString(byteArray.toBase64()));
            json.insert("sceneFeature", faiResultVector[0].sceneFeature);
            byteArray = fillWithQImage(faiResultVector[0].idImage);
            json.insert("cardPhoto", QString(byteArray.toBase64()));
            json.insert("cardFeature", faiResultVector[0].cardFeature);

            doc.setObject(json);
            QByteArray array = doc.toJson(QJsonDocument::Compact);

            naManager->post(request, array);
        }
    } else {
        qCritical() << "matchStatus Error: " << faiResultVector[0].matchStatus;
    }
}

/////////////////////////////////////////////////////////////////
/// \brief MainWindow::flightResponse
/// \param flightResult
/////////////////////////////////////////////////////////////////

void MainWindow::flightResponse(const FlightResult& flightResult)
{
    if (!isFrontPageByPassed) {
        // 超时后返回到刷身份安检界面
        timer->stop();
        timer->start(60000);

        connect(timer, SIGNAL(timeout()), this, SLOT(oncedown()));

        if (isInDiversionMode && flightResult.startPort == QString(tr("CKG"))) {
            MessageBox mbx;

            mbx.portErrorIconLabelShow();
            mbx.portErrorTextLabelShow();

            mbx.exec();

            return;
        }

        ui->displayWidget->hide();
        ui->fdisplayWidget->show();

        int res = CoreFIGetBestFace(optimumFace, &faceNum);
        if (res != 0) {
            qCritical() << "CoreFIGetBestFace Invokes Error.";
            return;
        }

        updateFlightInfo(flightResult);

        if (faceNum > 0) {
            ui->fstatuslabel->setStyleSheet("background: 0; border: 0; image: 0; color: rgb(171, 172, 176);");
            ui->fstatuslabel->setText(tr("正在上报．．"));
            flightInfo.kindType = 0;
            CoreFIInputFligtInfo(&flightInfo, optimumFace[0].faceData, optimumFace[0].imgLen, &status);
        }

        displayFlight(flightResult);
        databaseFlight(flightResult);
    }
}

void MainWindow::updateFlightInfo(const FlightResult& flightResult)
{
    strcpy(flightInfo.flightNo, flightResult.flightNo.toLatin1().data());
    strcpy(flightInfo.boardingNum, flightResult.boardingNum.toLatin1().data());
    strcpy(flightInfo.setId, flightResult.setId.toLatin1().data());
    strcpy(flightInfo.startPort, flightResult.startPort.toLatin1().data());
    strcpy(flightInfo.terminalPort, flightResult.terminalPort.toLatin1().data());
    strcpy(flightInfo.flightDay, flightResult.flightDay.toLatin1().data());
    flightInfo.kindType = 0;
}

void MainWindow::displayFlight(const FlightResult& flightResult)
{
    ui->fhangBanHaoLabel->setText(tr("航班号：") + flightResult.flightNo);
    ui->fxuLieHaoLabel->setText(tr("序列号：") + flightResult.boardingNum);
    ui->fqiFeiShiJianLabel->setText(tr("起飞时间：") + flightResult.flightDay);
    ui->fzuoWeiHaolabel->setText(tr("座位号：") + flightResult.setId);

    QImage img;
    if (faceNum > 0) {
        flightPixmap.loadFromData(reinterpret_cast<uchar*>(optimumFace[0].faceData)
                , static_cast<uint>(optimumFace[0].imgLen));

        img.load(":/Images/DengJiPai/YanZhengTongGuo.png");
        img = img.scaled(ui->stampLabel->width()
                         , ui->stampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->fstampLabel->setPixmap(QPixmap::fromImage(img));
        ui->fstatuslabel->clear();

        ui->freshotPushButton->hide();
        ui->fletpassPushButton->hide();
    } else {
        QImage img;
        img.load(":/Images/LiShiJiLu/WeiJianCeDaoRenNian.png");
        flightPixmap = QPixmap::fromImage(img);

        ui->fstampLabel->clear();
        ui->fstatuslabel->setStyleSheet("background: 0; border: 0; image: 0; color: red;");
        ui->fstatuslabel->setText(tr("未检测到人脸"));

        ui->freshotPushButton->show();
        ui->fletpassPushButton->show();
    }

    ui->fzhaoLabel->setPixmap(flightPixmap.scaled(ui->fzhaoLabel->width()
                                                   , ui->fzhaoLabel->height()
                                                   , Qt::IgnoreAspectRatio
                                                   , Qt::SmoothTransformation));

}

void MainWindow::databaseFlight(const FlightResult& flightResult)
{
    QSqlQuery query;

    query.prepare("INSERT INTO flight (id"
                  ", flightNo"
                  ", boardingNum"
                  ", setId"
                  ", startPort"
                  ", terminalPort"
                  ", flightDay"
                  ", kindType"
                  ", shotImage"
                  ", shotTime"
                  ", isUpLoaded"
                  ", source) "
                  "VALUES (:id"
                  ", :flightNo"
                  ", :boardingNum"
                  ", :setId"
                  ", :startPort"
                  ", :terminalPort"
                  ", :flightDay"
                  ", :kindType"
                  ", :shotImage"
                  ", :shotTime"
                  ", :isUpLoaded"
                  ", :source)");

    QString uuid = QUuid::createUuid().toString();
    uuid.remove("{").remove("}").remove("-");
    query.bindValue(":id", uuid);
    query.bindValue(":flightNo", flightResult.flightNo);
    query.bindValue(":boardingNum", flightResult.boardingNum);
    query.bindValue(":setId", flightResult.setId);
    query.bindValue(":startPort", flightResult.startPort);
    query.bindValue(":terminalPort", flightResult.terminalPort);
    query.bindValue(":flightDay", flightResult.flightDay);
    query.bindValue(":kindType", flightResult.kindType);
    QByteArray shotImageByteArray = fillWithQPixmap(flightPixmap);
    query.bindValue(":shotImage", shotImageByteArray);
    shotTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    query.bindValue(":shotTime", shotTime);
    if (faceNum > 0) {
        query.bindValue(":isUpLoaded", true);
        ui->fceHuiPushButton->show();
    } else {
        query.bindValue(":isUpLoaded", false);
        ui->fceHuiPushButton->hide();
    }
    query.bindValue(":source", tr("刷票"));

    query.exec();
}

void MainWindow::on_freshotPushButton_clicked()
{
    int res = CoreFIGetBestFace(optimumFace, &faceNum);
    if (res != 0) {
        qCritical() << "CoreFIGetBestFace Invokes Error.";
        return;
    }

    if (faceNum > 0) {
        ui->freshotPushButton->hide();
        ui->fletpassPushButton->hide();

        ui->fstatuslabel->setStyleSheet("background: 0; border: 0; image: 0; color: rgb(171, 172, 176);");
        ui->fstatuslabel->setText(tr("正在上报．．"));

        flightInfo.kindType = 0;
        CoreFIInputFligtInfo(&flightInfo, optimumFace[0].faceData, optimumFace[0].imgLen, &status);

        ui->fstatuslabel->clear();
        ui->fceHuiPushButton->show();
        QImage img;
        img.load(":/Images/DengJiPai/YanZhengTongGuo.png");
        img = img.scaled(ui->stampLabel->width()
                         , ui->stampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->fstampLabel->setPixmap(QPixmap::fromImage(img));

        // 更新本地抓拍人脸的显示
        flightPixmap.loadFromData(reinterpret_cast<uchar*>(optimumFace[0].faceData), static_cast<uint>(optimumFace[0].imgLen));
        ui->fzhaoLabel->setPixmap(flightPixmap.scaled(ui->fzhaoLabel->width()
                                                       , ui->fzhaoLabel->height()
                                                       , Qt::IgnoreAspectRatio
                                                       , Qt::SmoothTransformation));

        // 更新本地数据shotImage字段，shotTime字段
        QSqlQuery query;

        char sql[200];
        sprintf(sql, "UPDATE flight SET shotImage = ? WHERE shotTime='%s'"
                , shotTime.toStdString().data());

        query.prepare(sql);
        QByteArray shotImageByteArray = fillWithQPixmap(flightPixmap);
        query.bindValue(0, shotImageByteArray);
        query.exec();

        sprintf(sql, "UPDATE flight SET isUpLoaded = ? WHERE shotTime='%s'"
                , shotTime.toStdString().data());
        query.prepare(sql);
        query.bindValue(0, true);
        query.exec();

        sprintf(sql, "UPDATE flight SET shotTime = ? WHERE shotTime='%s'"
                , shotTime.toStdString().data());
        query.prepare(sql);
        shotTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        query.bindValue(0, shotTime);
        query.exec();
    } else {
        ui->fstatuslabel->setStyleSheet("background: 0; border: 0; image: 0; color: red;");
        ui->fstatuslabel->setText(tr("未检测到人脸"));

        ui->freshotPushButton->show();
        ui->fletpassPushButton->show();
        ui->fceHuiPushButton->hide();
    }
}

void MainWindow::on_fletpassPushButton_clicked()
{
    MessageBoxFX fx;

    flightPixmap = vpixmap;
    fx.imageShow(flightPixmap);

    if (fx.exec() == QDialog::Accepted) {
        QNetworkRequest request;
        QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

        connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(letpassResponse(QNetworkReply*)));

        // Header
        QString securityManualCheck = Singleton::getInstance().getConfigQtIni()->value("/interfaces/securityManualCheck").toString();
        request.setUrl(QUrl(securityManualCheck));
        QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
        QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
        request.setRawHeader("apiId", apiId.toLatin1());
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        request.setRawHeader("timestamp", timestamp.toLatin1());
        QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
        QString temp = securityManualCheck.mid(securityManualCheck.indexOf("/api/v1")) + timestamp + apiKey;
        QByteArray bb = QCryptographicHash::hash(temp.toLatin1(), QCryptographicHash::Md5);
        QString sign = QString().append(bb.toHex());
        request.setRawHeader("sign", sign.toLatin1());

        // Body
        QJsonObject json;
        QJsonDocument doc;
        QString uuid = QUuid::createUuid().toString();
        uuid.remove("{").remove("}").remove("-");
        json.insert("reqId", uuid);
        json.insert("flightNo", flightInfo.flightNo);
        QString deviceId = Singleton::getInstance().getConfigIni()->value("/device/deviceId").toString();
        json.insert("deviceCode", deviceId);
        json.insert("boardingNumber", flightInfo.boardingNum);
        json.insert("seatId", flightInfo.setId);
        json.insert("startPort", flightInfo.startPort);
        json.insert("flightDay", flightInfo.flightDay);
        QString gateNo = Singleton::getInstance().getConfigIni()->value("/device/gateNo").toString();
        json.insert("gateNo", gateNo);
        json.insert("kindType", 1); //
        json.insert("endPort", flightInfo.terminalPort);
        QByteArray byteArray = fillWithQPixmap(flightPixmap);
        json.insert("faceImage", QString(byteArray.toBase64()));

        doc.setObject(json);
        QByteArray array = doc.toJson(QJsonDocument::Compact);

        naManager->post(request, array);
    }
}

void MainWindow::nofaceFangXing(QNetworkReply* reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (statusCode.isValid()) {
        qDebug() << "status code = " << statusCode.toInt();
    }

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

    if (reason.isValid()) {
        qDebug() << "reason = " << reason.toString();
    }

    QNetworkReply::NetworkError err = reply->error();

    if (err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    } else {
        QByteArray all = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(all, &err);
        if (err.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("status") && obj.value("status") == QJsonValue(0)) {
                    // 隐藏放行按钮
                    ui->fangXingPushButton->hide();

                    // 更新表格“通行结果”内容
                    // 翻页后要根据matchStatus字段获取通行结果，因此vector中此字段也需要更新
                    faiResultVector[0].matchStatus = STATUS_ARTIFICIAL_PASS;
                    QFont font;
                    font.setPointSize(9);
                    QTableWidgetItem *item3 = new QTableWidgetItem(tr("已人工放行"));
                    item3->setFont(font);
                    item3->setForeground(QBrush(Qt::red));
                    item3->setTextAlignment(Qt::AlignCenter);
                    ui->liShiTableWidget->setItem(0, 3, item3);

                    // 把放行弹窗上的这张照片更新到zhaoLabel
                    // 翻页或者增加行后点击已经放行通过的行要根据liveImage字段显示zhaoLabel，因此vector中此字段也需要更新
                    faiResultVector[0].liveImage = vpixmap.toImage();
                    QPixmap scaled = vpixmap.scaled(ui->zhaoLabel->width()
                                                    , ui->zhaoLabel->height()
                                                    , Qt::IgnoreAspectRatio
                                                    , Qt::SmoothTransformation);
                    ui->zhaoLabel->setPixmap(scaled);

                    // 更新表格行展开内容中的检测结果盖章
                    QImage img;
                    img.load(":/Images/LiShiJiLu/FangXingTongGuo.png");
                    img = img.scaled(ui->stampLabel->width()
                                     , ui->stampLabel->height()
                                     , Qt::IgnoreAspectRatio
                                     , Qt::SmoothTransformation);
                    ui->stampLabel->setPixmap(QPixmap::fromImage(img));

                    // 更新本地数据库中matchStatus字段，liveImage字段
                    QSqlQuery query;

                    char sql[200];
                    sprintf(sql, "UPDATE person SET matchStatus = ? WHERE matchTime='%s'"
                            , faiResultVector[0].matchTime.toStdString().data());

                    query.prepare(sql);
                    query.bindValue(0, STATUS_ARTIFICIAL_PASS);
                    query.exec();

                    sprintf(sql, "UPDATE person SET liveImage = ? WHERE matchTime='%s'"
                            , faiResultVector[0].matchTime.toStdString().data());

                    query.prepare(sql);
                    QByteArray liveImageByteArray = fillWithQPixmap(vpixmap);
                    query.bindValue(0, liveImageByteArray);
                    query.exec();
                }
            }
        } else {
            qDebug() << "err.error: " << err.error;
        }
    }
}

void MainWindow::comparefailFangXing(QNetworkReply* reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (statusCode.isValid()) {
        qDebug() << "status code = " << statusCode.toInt();
    }

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

    if (reason.isValid()) {
        qDebug() << "reason = " << reason.toString();
    }

    QNetworkReply::NetworkError err = reply->error();

    if (err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    } else {
        QByteArray all = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(all, &err);
        if (err.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("status") && obj.value("status") == QJsonValue(0)) {
                    // 隐藏放行按钮
                    ui->fangXingPushButton->hide();

                    // 更新表格“通行结果”内容
                    // 翻页后要根据matchStatus字段获取通行结果，因此vector中此字段也需要更新
                    faiResultVector[0].matchStatus = STATUS_ARTIFICIAL_PASS;
                    QFont font;
                    font.setPointSize(9);
                    QTableWidgetItem *item3 = new QTableWidgetItem(tr("已人工放行"));
                    item3->setFont(font);
                    item3->setForeground(QBrush(Qt::red));
                    item3->setTextAlignment(Qt::AlignCenter);
                    ui->liShiTableWidget->setItem(0, 3, item3);

                    // 更新表格行展开内容中的检测结果盖章
                    QImage img;
                    img.load(":/Images/LiShiJiLu/FangXingTongGuo.png");
                    img = img.scaled(ui->stampLabel->width()
                                     , ui->stampLabel->height()
                                     , Qt::IgnoreAspectRatio
                                     , Qt::SmoothTransformation);
                    ui->stampLabel->setPixmap(QPixmap::fromImage(img));

                    // 更新本地数据库中matchStatus字段
                    QSqlQuery query;

                    char sql[200];
                    sprintf(sql, "UPDATE person SET matchStatus = ? WHERE matchTime='%s'"
                            , faiResultVector[0].matchTime.toStdString().data());

                    query.prepare(sql);
                    query.bindValue(0, STATUS_ARTIFICIAL_PASS);
                    query.exec();
                }
            }
        } else {
            qDebug() << "err.error: " << err.error;
        }
    }
}

void MainWindow::letpassResponse(QNetworkReply* reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (statusCode.isValid()) {
        qDebug() << "status code = " << statusCode.toInt();
    }

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

    if (reason.isValid()) {
        qDebug() << "reason = " << reason.toString();
    }

    QNetworkReply::NetworkError err = reply->error();

    if (err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    } else {
        QByteArray all = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(all, &err);
        if (err.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("status") && obj.value("status") == QJsonValue(0)) {
                    // 隐藏放行按钮
                    ui->fletpassPushButton->hide();
                    ui->freshotPushButton->hide();
                    ui->fstatuslabel->clear();

                    QImage img;
                    img.load(":/Images/LiShiJiLu/FangXingTongGuo.png");
                    img = img.scaled(ui->stampLabel->width()
                                     , ui->stampLabel->height()
                                     , Qt::IgnoreAspectRatio
                                     , Qt::SmoothTransformation);
                    ui->fstampLabel->setPixmap(QPixmap::fromImage(img));

                    ui->fzhaoLabel->setPixmap(flightPixmap.scaled(ui->fzhaoLabel->width()
                                                                   , ui->fzhaoLabel->height()
                                                                   , Qt::IgnoreAspectRatio
                                                                   , Qt::SmoothTransformation));

                    // 更新本地数据库中shotImage字段，shotTime字段，kindType字段，source字段
                    QSqlQuery query;

                    char sql[200];
                    sprintf(sql, "UPDATE flight SET shotImage = ? WHERE shotTime='%s'"
                            , shotTime.toStdString().data());

                    query.prepare(sql);
                    QByteArray shotImageByteArray = fillWithQPixmap(flightPixmap);
                    query.bindValue(0, shotImageByteArray);
                    query.exec();

                    sprintf(sql, "UPDATE flight SET kindType = ? WHERE shotTime='%s'"
                            , shotTime.toStdString().data());

                    query.prepare(sql);
                    query.bindValue(0, 1);
                    query.exec();

                    sprintf(sql, "UPDATE flight SET source = ? WHERE shotTime='%s'"
                            , shotTime.toStdString().data());

                    query.prepare(sql);
                    query.bindValue(0, tr("放行"));
                    query.exec();

                    sprintf(sql, "UPDATE flight SET shotTime = ? WHERE shotTime='%s'"
                            , shotTime.toStdString().data());

                    query.prepare(sql);
                    shotTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
                    query.bindValue(0, shotTime);
                    query.exec();
                }
            }
        } else {
            qDebug() << "err.error: " << err.error;
        }
    }
}

void MainWindow::on_fceHuiPushButton_clicked()
{
    MessageBox mbx;

    mbx.cheHuiIconLabelShow();
    mbx.cheHuiTextLabelShow();

    if (mbx.exec() == QDialog::Accepted) {
        QNetworkRequest request;
        QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

        withdrawConnection = connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(withdrawResponse(QNetworkReply*)));
        Q_ASSERT(withdrawConnection);

        // Header
        QString securityManualCheck = Singleton::getInstance().getConfigQtIni()->value("/interfaces/securityManualCheck").toString();
        request.setUrl(QUrl(securityManualCheck));
        QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
        QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
        request.setRawHeader("apiId", apiId.toLatin1());
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        request.setRawHeader("timestamp", timestamp.toLatin1());
        QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
        QString temp = securityManualCheck.mid(securityManualCheck.indexOf("/api/v1")) + timestamp + apiKey;
        QByteArray bb = QCryptographicHash::hash(temp.toLatin1(), QCryptographicHash::Md5);
        QString sign = QString().append(bb.toHex());
        request.setRawHeader("sign", sign.toLatin1());

        // Body
        QJsonObject json;
        QJsonDocument doc;
        QString uuid = QUuid::createUuid().toString();
        uuid.remove("{").remove("}").remove("-");
        json.insert("reqId", uuid);
        json.insert("flightNo", flightInfo.flightNo);
        QString deviceId = Singleton::getInstance().getConfigIni()->value("/device/deviceId").toString();
        json.insert("deviceCode", deviceId);
        json.insert("boardingNumber", flightInfo.boardingNum);
        json.insert("seatId", flightInfo.setId);
        json.insert("startPort", flightInfo.startPort);
        json.insert("flightDay", flightInfo.flightDay);
        QString gateNo = Singleton::getInstance().getConfigIni()->value("/device/gateNo").toString();
        json.insert("gateNo", gateNo);
        json.insert("kindType", 2); //
        json.insert("endPort", flightInfo.terminalPort);
        QByteArray byteArray = fillWithQPixmap(flightPixmap);
        json.insert("faceImage", QString(byteArray.toBase64()));

        doc.setObject(json);
        QByteArray array = doc.toJson(QJsonDocument::Compact);

        naManager->post(request, array);
    }
}

void MainWindow::withdrawResponse(QNetworkReply* reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (statusCode.isValid()) {
        qDebug() << "status code = " << statusCode.toInt();
    }

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

    if (reason.isValid()) {
        qDebug() << "reason = " << reason.toString();
    }

    QNetworkReply::NetworkError err = reply->error();

    if (err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    } else {
        QByteArray all = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(all, &err);
        if (err.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("status") && obj.value("status") == QJsonValue(0)) {
                    // 隐藏撤回按钮
                    ui->fletpassPushButton->hide();
                    ui->freshotPushButton->hide();
                    ui->fceHuiPushButton->hide();
                    ui->fstampLabel->clear();
                    ui->fstatuslabel->setStyleSheet("background: 0; border: 0; image: 0; color: rgb(171, 172, 176);");
                    ui->fstatuslabel->setText(tr("特征已撤回"));

                    // 更新本地数据库中kindType字段，source字段，isUpLoaded字段
                    QSqlQuery query;

                    char sql[200];
                    sprintf(sql, "UPDATE flight SET kindType = ? WHERE shotTime='%s'"
                            , shotTime.toStdString().data());

                    query.prepare(sql);
                    query.bindValue(0, 2);
                    query.exec();

                    sprintf(sql, "UPDATE flight SET source = ? WHERE shotTime='%s'"
                            , shotTime.toStdString().data());

                    query.prepare(sql);
                    query.bindValue(0, tr("撤回"));
                    query.exec();

                    sprintf(sql, "UPDATE flight SET isUpLoaded = ? WHERE shotTime='%s'"
                            , shotTime.toStdString().data());

                    query.prepare(sql);
                    query.bindValue(0, false);
                    query.exec();
                }
            }
        } else {
            qDebug() << "err.error: " << err.error;
        }
    }
}

void MainWindow::on_video2PushButton_clicked()
{
    isFaceDetectVideoChanged = !isFaceDetectVideoChanged;

    CoreFISetFaceDetectVideo(isFaceDetectVideoChanged);
}

/*********************************
 * backPageWidget
 * *******************************/

void MainWindow::updatePageLabel()
{
    QString pageLabel = "当前第" + QString::number(currentPage) + "页，共" + QString::number(pageCount) + "页";
    ui->bpageLabel->setText(pageLabel);
}

void MainWindow::updatePageRows()
{
    QFont font;
    font.setPointSize(9);

    int startRow = rowCount - (currentPage-1)*rowCountPerPage - currentRows;
    startRow = (startRow > 0) ? startRow : 0;
    int endRow = rowCount - (currentPage-1)*rowCountPerPage;

    ui->bshuJuTableWidget->clearContents();
    ui->bshuJuTableWidget->setRowCount(0);
    for (int index = startRow; index < endRow; index++) {
        ui->bshuJuTableWidget->insertRow(0);
        ui->bshuJuTableWidget->setRowHeight(0, 18);

        QTableWidgetItem *item0 = new QTableWidgetItem(queryModel->record(index).value("name").toString());
        item0->setFont(font);
        item0->setForeground(QBrush(Qt::white));
        item0->setTextAlignment(Qt::AlignCenter);
        ui->bshuJuTableWidget->setItem(0, 0, item0);

        QTableWidgetItem *item1 = new QTableWidgetItem(queryModel->record(index).value("matchTime").toString());
        item1->setFont(font);
        item1->setForeground(QBrush(Qt::white));
        item1->setTextAlignment(Qt::AlignCenter);
        ui->bshuJuTableWidget->setItem(0, 1, item1);

        QTableWidgetItem *item2 = new QTableWidgetItem(QString("%1").arg(queryModel->record(index).value("matchScore").toDouble()));
        item2->setFont(font);
        item2->setForeground(QBrush(Qt::white));
        item2->setTextAlignment(Qt::AlignCenter);
        ui->bshuJuTableWidget->setItem(0, 2, item2);

        if ((queryModel->record(index).value("matchStatus").toUInt() & 0xf) == STATUS_COMPARE_FAIL) {
            QTableWidgetItem *item3 = new QTableWidgetItem(tr("比对不通过"));
            item3->setFont(font);
            item3->setForeground(QBrush(Qt::red));
            item3->setTextAlignment(Qt::AlignCenter);
            ui->bshuJuTableWidget->setItem(0, 3,item3);
        } else if ((queryModel->record(index).value("matchStatus").toUInt() & 0xf) == STATUS_COMPARE_PASS) {
            QTableWidgetItem *item3 = new QTableWidgetItem(tr("比对通过"));
            item3->setFont(font);
            item3->setForeground(QBrush(Qt::green));
            item3->setTextAlignment(Qt::AlignCenter);
            ui->bshuJuTableWidget->setItem(0, 3, item3);
        } else if ((queryModel->record(index).value("matchStatus").toUInt() & 0xf) == STATUS_DETECT_NO_FACE) {
            QTableWidgetItem *item3 = new QTableWidgetItem(tr("无人脸信息"));
            item3->setFont(font);
            item3->setForeground(QBrush(Qt::red));
            item3->setTextAlignment(Qt::AlignCenter);
            ui->bshuJuTableWidget->setItem(0, 3, item3);
        } else if ((queryModel->record(index).value("matchStatus").toUInt() & 0xf) == STATUS_ARTIFICIAL_PASS) {
            QTableWidgetItem *item3 = new QTableWidgetItem(tr("已人工放行"));
            item3->setFont(font);
            item3->setForeground(QBrush(Qt::red));
            item3->setTextAlignment(Qt::AlignCenter);
            ui->bshuJuTableWidget->setItem(0, 3, item3);
        } else if ((queryModel->record(index).value("matchStatus").toUInt() & 0xf) == STATUS_DEFAULT) {
            qCritical() << "Error! faiResult Not Set.";
        }

        if (queryModel->record(index).value("matchStatus").toUInt() & STATUS_IDCARD_EXPIRE)
        {
            QTableWidgetItem *item4 = new QTableWidgetItem(tr("无效"));
            item4->setFont(font);
            item4->setForeground(QBrush(Qt::red));
            item4->setTextAlignment(Qt::AlignCenter);
            ui->bshuJuTableWidget->setItem(0, 4, item4);
        } else {
            QTableWidgetItem *item4 = new QTableWidgetItem(tr("有效"));
            item4->setFont(font);
            item4->setForeground(QBrush(Qt::green));
            item4->setTextAlignment(Qt::AlignCenter);
            ui->bshuJuTableWidget->setItem(0, 4, item4);
        }
    }
}

void MainWindow::fupdatePageLabel()
{
    QString pageLabel = "当前第" + QString::number(currentPage) + "页，共" + QString::number(pageCount) + "页";
    ui->bfpageLabel->setText(pageLabel);
}

void MainWindow::fupdatePageRows()
{
    QFont font;
    font.setPointSize(9);

    int startRow = rowCount - (currentPage-1)*rowCountPerPage - currentRows;
    startRow = (startRow > 0) ? startRow : 0;
    int endRow = rowCount - (currentPage-1)*rowCountPerPage;

    ui->bfshuJuTableWidget->clearContents();
    ui->bfshuJuTableWidget->setRowCount(0);
    for (int index = startRow; index < endRow; index++) {
        ui->bfshuJuTableWidget->insertRow(0);
        ui->bfshuJuTableWidget->setRowHeight(0, 18);

        QTableWidgetItem *item0 = new QTableWidgetItem(queryModel->record(index).value("flightNo").toString());
        item0->setFont(font);
        item0->setForeground(QBrush(Qt::white));
        item0->setTextAlignment(Qt::AlignCenter);
        ui->bfshuJuTableWidget->setItem(0, 0, item0);

        QTableWidgetItem *item1 = new QTableWidgetItem(queryModel->record(index).value("shotTime").toString());
        item1->setFont(font);
        item1->setForeground(QBrush(Qt::white));
        item1->setTextAlignment(Qt::AlignCenter);
        ui->bfshuJuTableWidget->setItem(0, 1, item1);

        QTableWidgetItem *item2 = new QTableWidgetItem(queryModel->record(index).value("boardingNum").toString());
        item2->setFont(font);
        item2->setForeground(QBrush(Qt::white));
        item2->setTextAlignment(Qt::AlignCenter);
        ui->bfshuJuTableWidget->setItem(0, 2, item2);

        QTableWidgetItem *item3 = new QTableWidgetItem(queryModel->record(index).value("setId").toString());
        item3->setFont(font);
        item3->setForeground(QBrush(Qt::white));
        item3->setTextAlignment(Qt::AlignCenter);
        ui->bfshuJuTableWidget->setItem(0, 3, item3);

        QTableWidgetItem *item4;
        if (queryModel->record(index).value("isUpLoaded") == true) {
            item4 = new QTableWidgetItem(tr("是"));
            item4->setForeground(QBrush(Qt::green));
        } else {
            item4 = new QTableWidgetItem(tr("否"));
            item4->setForeground(QBrush(Qt::red));
        }
        item4->setFont(font);
        item4->setTextAlignment(Qt::AlignCenter);
        ui->bfshuJuTableWidget->setItem(0, 4, item4);

        QTableWidgetItem *item5 = new QTableWidgetItem(queryModel->record(index).value("source").toString());
        item5->setFont(font);
        item5->setForeground(QBrush(Qt::white));
        item5->setTextAlignment(Qt::AlignCenter);
        ui->bfshuJuTableWidget->setItem(0, 5, item5);
    }
}

void MainWindow::on_bfanHuiButton_clicked()
{
    isFrontPageByPassed = false;

    ui->backPageWidget->hide();
    ui->frontPageWidget->show();
}

void MainWindow::on_bfchaXunPushButton_clicked()
{
    QString begin = ui->bftiaoJianLineEdit_1->text();
    QString end = ui->bftiaoJianLineEdit_2->text();
    QString upLoad = ui->bftiaoJianComboBox->currentText();
    QString flightNo = ui->bftiaoJianLineEdit_3->text();
    char sql[200];

    if (begin == QString())
        begin = "20180101";

    if (end == QString())
        end = "21000101";

    QString begin2 = begin.left(4) + "/" + begin.mid(4, 2) + "/" + begin.mid(6, 2) + " " + "00:00:00";
    QString end2 = end.left(4) + "/" + end.mid(4, 2) + "/" + end.mid(6, 2) + " " + "23:59:59";

    if (flightNo == QString()) {
        if (upLoad == QString("全部")) {
            sprintf(sql, "SELECT * FROM flight where shotTime > '%s' and shotTime < '%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
        if (upLoad == QString("是")) {
            sprintf(sql, "SELECT * FROM flight where shotTime > '%s' and shotTime < '%s' and isUpLoaded=true"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
        if (upLoad == QString("否")) {
            sprintf(sql, "SELECT * FROM flight where shotTime > '%s' and shotTime < '%s' and isUpLoaded=false"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
    } else {
        if (upLoad == QString("全部")) {
            sprintf(sql, "SELECT * FROM flight where shotTime > '%s' and shotTime < '%s' and flightNo='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , flightNo.toStdString().data());
        }
        if (upLoad == QString("是")) {
            sprintf(sql, "SELECT * FROM flight where shotTime > '%s' and shotTime < '%s' and isUpLoaded=true and flightNo='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , flightNo.toStdString().data());
        }
        if (upLoad == QString("否")) {
            sprintf(sql, "SELECT * FROM flight where shotTime > '%s' and shotTime < '%s' and isUpLoaded=false and flightNo='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , flightNo.toStdString().data());
        }
    }

    QSqlQuery query;
    query.exec(sql);
    queryModel->setQuery(query);

    rowCount = queryModel->rowCount();
    pageCount = queryModel->rowCount()/rowCountPerPage + 1;
    currentPage = 1;
    currentRows = (currentPage == pageCount) ? rowCount%rowCountPerPage: rowCountPerPage;

    fupdatePageLabel();

    fupdatePageRows();
}

void MainWindow::on_bshuJuTableWidget_itemClicked(QTableWidgetItem *item)
{
    int row = item->row();
    row = rowCount - 1 - row - (currentPage-1)*rowCountPerPage;

    ui->bnameLabel->setText(tr("姓名：") + queryModel->record(row).value("name").toString());
    ui->bnationalityLabel->setText(tr("民族：") + queryModel->record(row).value("folk").toString());
    ui->bidCardLabel->setText(tr("身份证号：") + queryModel->record(row).value("code").toString());
    ui->bexpireTimeLabel->setText(tr("有效期限：") + queryModel->record(row).value("expireStart").toString() + tr("-") + queryModel->record(row).value("expireEnd").toString());
    ui->bcompareLabel->setText(tr("比对分值：") + QString("%1").arg(queryModel->record(row).value("matchScore").toDouble()));
    if (queryModel->record(row).value("matchStatus").toUInt() == STATUS_COMPARE_PASS) {
        ui->bcompareLabel->setStyleSheet("border: 0; background: 0; color: green;");
    } else {
        ui->bcompareLabel->setStyleSheet("border: 0; background: 0; color: red;");
    }

    if (!queryModel->record(row).value("idImage").isNull()) {
        QByteArray idImageByteArray = queryModel->record(row).value("idImage").toByteArray();
        QImage img;
        img.loadFromData(idImageByteArray);
        QImage imgScaled = img.scaled(ui->bzhengLabel->width()
                                      , ui->bzhengLabel->height()
                                      , Qt::IgnoreAspectRatio
                                      , Qt::SmoothTransformation);
        ui->bzhengLabel->setPixmap(QPixmap::fromImage(imgScaled));
    } else {
        ui->bzhengLabel->clear();
    }

    if(!queryModel->record(row).value("liveImage").isNull()) {
        QByteArray liveImageByteArray = queryModel->record(row).value("liveImage").toByteArray();
        QImage img;
        img.loadFromData(liveImageByteArray);
        QImage imgScaled = img.scaled(ui->bzhaoLabel->width()
                                      , ui->bzhaoLabel->height()
                                      , Qt::IgnoreAspectRatio
                                      , Qt::SmoothTransformation);
        ui->bzhaoLabel->setPixmap(QPixmap::fromImage(imgScaled));
    } else {
        ui->bzhaoLabel->clear();
    }

    if (queryModel->record(row).value("matchStatus").toUInt() & STATUS_IDCARD_EXPIRE) {
        QImage img;
        img.load(":/Images/RenZhengBiDui/seal_invalid.png");
        img = img.scaled(ui->bstampLabel->width()
                         , ui->bstampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->bstampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((queryModel->record(row).value("matchStatus").toUInt() & 0xf) == STATUS_COMPARE_PASS) {
        QImage img;
        img.load(":/Images/RenZhengBiDui/YanZhengTongGuo.png");
        img = img.scaled(ui->bstampLabel->width()
                         , ui->bstampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);

        ui->bstampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((queryModel->record(row).value("matchStatus").toUInt() & 0xf) == STATUS_COMPARE_FAIL) {
        QImage img;
        img.load(":/Images/RenZhengBiDui/BiDuiBuTongGuo.png");
        img = img.scaled(ui->bstampLabel->width()
                         , ui->bstampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->bstampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((queryModel->record(row).value("matchStatus").toUInt() & 0xf) == STATUS_DETECT_NO_FACE) {
        QImage img;
        img.load(":/Images/LiShiJiLu/seal_noface.png");
        img = img.scaled(ui->bstampLabel->width()
                         , ui->bstampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->bstampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((queryModel->record(row).value("matchStatus").toUInt() & 0xf) == STATUS_ARTIFICIAL_PASS) {
        QImage img;
        img.load(":/Images/LiShiJiLu/FangXingTongGuo.png");
        img = img.scaled(ui->bstampLabel->width()
                         , ui->bstampLabel->height()
                         , Qt::IgnoreAspectRatio
                         , Qt::SmoothTransformation);
        ui->bstampLabel->setPixmap(QPixmap::fromImage(img));
    } else if ((queryModel->record(row).value("matchStatus").toUInt() & 0xf) == STATUS_DEFAULT) {
        ui->bstampLabel->clear();
        qCritical() << "Error! faiResult Not Set.";
    }
}

void MainWindow::on_bpgNextPushButton_clicked()
{
    if (currentPage < pageCount) {
        currentPage += 1;
    }

    updatePageLabel();

    updatePageRows();
}

void MainWindow::on_bpgPrePushButton_clicked()
{
    if (currentPage > 1) {
        currentPage -= 1;
    }

    updatePageLabel();

    updatePageRows();
}

void MainWindow::on_bpgEndPushButton_clicked()
{
    currentPage = pageCount;

    updatePageLabel();

    updatePageRows();
}

void MainWindow::on_bpgFirstPushButton_clicked()
{
    currentPage = 1;

    updatePageLabel();

    updatePageRows();
}

void MainWindow::on_bqieHuanPushButton_clicked()
{
    fupdatePageLabel();

    ui->bfshuJuTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bfshuJuTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bfshuJuTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->bfshuJuTableWidget->verticalHeader()->setVisible(false);
    ui->bfshuJuTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->bfshuJuTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->bfshuJuTableWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
    ui->bfshuJuTableWidget->setShowGrid(false);
    ui->bfshuJuTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color: rgba(0, 0, 0, 0); border: 0; border-bottom: 1px solid #094b57; color: white; }");

    ui->bfshuJuTableWidget->setColumnWidth(0, 65);
    ui->bfshuJuTableWidget->setColumnWidth(1, 130);
    ui->bfshuJuTableWidget->setColumnWidth(2, 46);
    ui->bfshuJuTableWidget->setColumnWidth(3, 58);
    ui->bfshuJuTableWidget->setColumnWidth(4, 46);
    ui->bfshuJuTableWidget->setColumnWidth(5, 46);

    ui->bdisplayWidget->hide();
    ui->boperateWidget->hide();
    ui->bfdisplayWidget->show();
    ui->bfoperateWidget->show();
}

void MainWindow::on_bfqieHuanPushButton_clicked()
{
    updatePageLabel();

    ui->bshuJuTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bshuJuTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bshuJuTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->bshuJuTableWidget->verticalHeader()->setVisible(false);
    ui->bshuJuTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->bshuJuTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->bshuJuTableWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
    ui->bshuJuTableWidget->setShowGrid(false);
    ui->bshuJuTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color: rgba(0, 0, 0, 0); border: 0; border-bottom: 1px solid #094b57; color: white; }");

    ui->bshuJuTableWidget->setColumnWidth(0, 62);
    ui->bshuJuTableWidget->setColumnWidth(1, 130);
    ui->bshuJuTableWidget->setColumnWidth(2, 62);
    ui->bshuJuTableWidget->setColumnWidth(3, 79);
    ui->bshuJuTableWidget->setColumnWidth(4, 58);

    ui->bfdisplayWidget->hide();
    ui->bfoperateWidget->hide();
    ui->bdisplayWidget->show();
    ui->boperateWidget->show();
}

void MainWindow::on_bchaXunPushButton_clicked()
{
    QString begin = ui->btiaoJianLineEdit_1->text();
    QString end = ui->btiaoJianLineEdit_2->text();
    QString code = ui->btiaoJianLineEdit_3->text();
    QString result = ui->btiaoJianComboBox->currentText();
    char sql[200];

    if (begin == QString())
        begin = "20180101";

    if (end == QString())
        end = "21000101";

    QString begin2 = begin.left(4) + "/" + begin.mid(4, 2) + "/" + begin.mid(6, 2) + " " + "00:00:00";
    QString end2 = end.left(4) + "/" + end.mid(4, 2) + "/" + end.mid(6, 2) + " " + "23:59:59";

    if (code == QString()) {
        if (result == QString("全部")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
        if (result == QString("系统验证通过")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=1"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
        if (result == QString("系统验证不通过")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=2"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
        if (result == QString("未检测到人脸")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=3"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
        if (result == QString("人工验证通过")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=4"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
        if (result == QString("证件失效")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus>15"
                    , begin2.toStdString().data()
                    , end2.toStdString().data());
        }
    } else {
        if (result == QString("全部")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and code='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , code.toStdString().data());
        }
        if (result == QString("系统验证通过")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=1 and code='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , code.toStdString().data());
        }
        if (result == QString("系统验证不通过")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=2 and code='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , code.toStdString().data());
        }
        if (result == QString("未检测到人脸")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=3 and code='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , code.toStdString().data());
        }
        if (result == QString("人工验证通过")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus=4 and code='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , code.toStdString().data());
        }
        if (result == QString("证件失效")) {
            sprintf(sql, "SELECT * FROM person where matchTime > '%s' and matchTime < '%s' and matchStatus>15 and code='%s'"
                    , begin2.toStdString().data()
                    , end2.toStdString().data()
                    , code.toStdString().data());
        }
    }

    QSqlQuery query;
    query.exec(sql);
    queryModel->setQuery(query);

    rowCount = queryModel->rowCount();
    pageCount = queryModel->rowCount()/rowCountPerPage + 1;
    currentPage = 1;
    currentRows = (currentPage == pageCount) ? rowCount%rowCountPerPage: rowCountPerPage;

    updatePageLabel();

    updatePageRows();
}

void MainWindow::on_bfpgFirstPushButton_clicked()
{
    currentPage = 1;

    fupdatePageLabel();

    fupdatePageRows();
}

void MainWindow::on_bfpgPrePushButton_clicked()
{
    if (currentPage > 1)
    {
        currentPage -= 1;
    }

    fupdatePageLabel();

    fupdatePageRows();
}

void MainWindow::on_bfpgNextPushButton_clicked()
{
    if (currentPage < pageCount)
    {
        currentPage += 1;
    }

    fupdatePageLabel();

    fupdatePageRows();
}

void MainWindow::on_bfpgEndPushButton_clicked()
{
    currentPage = pageCount;

    fupdatePageLabel();

    fupdatePageRows();
}

void MainWindow::on_bfshuJuTableWidget_itemClicked(QTableWidgetItem *item)
{
    int row = item->row();
    row = rowCount - 1 - row - (currentPage-1)*rowCountPerPage;

    ui->bfhangBanHaoLabel->setText(tr("航班号：") + queryModel->record(row).value("flightNo").toString());
    ui->bfxuLieHaoLabel->setText(tr("序列号：") + queryModel->record(row).value("boardingNum").toString());
    ui->bfqiFeiShiJianLabel->setText(tr("起飞时间：") + queryModel->record(row).value("flightDay").toString());
    ui->bfzuoWeiHaolabel->setText(tr("座位号：") + queryModel->record(row).value("setId").toString());
    ui->bfchuFaDiLabel->setText(tr("出发地：") + queryModel->record(row).value("startPort").toString());
    if (!queryModel->record(row).value("shotImage").isNull()) {
        QByteArray shotImageByteArray = queryModel->record(row).value("shotImage").toByteArray();
        QImage img;
        img.loadFromData(shotImageByteArray);
        QImage imgScaled = img.scaled(ui->bzhengLabel->width()
                                      , ui->bzhengLabel->height()
                                      , Qt::IgnoreAspectRatio
                                      , Qt::SmoothTransformation);
        ui->bfzhaoLabel->setPixmap(QPixmap::fromImage(imgScaled));
    } else {
        ui->bfzhaoLabel->clear();
    }
}
