#include "querydialog.h"
#include "ui_querydialog.h"
#include <QPainter>
#include "messagebox.h"
#include <QtDebug>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QFile>
#include <QUuid>
#include <QVBoxLayout>
#include <QBuffer>

QueryDialog::QueryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryDialog)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->setupUi(this);
    ui->cardCodeLineEdit->setPlaceholderText(tr("身份证号或者航班号/序列号"));
    ui->headerWidget->show();
    ui->bodyWidget_1->hide();
    ui->bodyWidget_2->hide();
    ui->bodyWidget_3->hide();
}

QueryDialog::~QueryDialog()
{
    delete ui;
}

QPixmap QueryDialog::getQPixmapSync(QString str)
{
    const QUrl url = QUrl::fromUserInput(str);

    QNetworkRequest request(url);
    QNetworkAccessManager *naManager = new QNetworkAccessManager(this);
    QNetworkReply* reply = naManager->get(request);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    QByteArray byteArray = reply->readAll();

    QPixmap pixmap;
    pixmap.loadFromData(byteArray);

    return pixmap;
}

void QueryDialog::on_closePushButton_clicked()
{
    close();
}

void QueryDialog::on_queryPushButton_clicked()
{
    document = QJsonDocument();
    array = QJsonArray();

    QNetworkRequest request;
    QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

    connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(postResponse(QNetworkReply*)));

    // Header
    QString reviewFlowbackQuery = Singleton::getInstance().getConfigQtIni()->value("/interfaces/reviewFlowbackQuery").toString();
    request.setUrl(QUrl(reviewFlowbackQuery));
    QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
    request.setRawHeader("apiId", apiId.toLatin1());
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    request.setRawHeader("timestamp", timestamp.toLatin1());
    QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
    QString temp = reviewFlowbackQuery.mid(reviewFlowbackQuery.indexOf("/api/v1")) + timestamp + apiKey;
    QByteArray bb = QCryptographicHash::hash(temp.toLatin1(), QCryptographicHash::Md5);
    QString sign = QString().append(bb.toHex());
    request.setRawHeader("sign", sign.toLatin1());

    // Body
    QJsonObject json;
    QJsonDocument doc;
    QString uuid = QUuid::createUuid().toString();
    uuid.remove("{").remove("}").remove("-");
    json.insert("reqId", uuid);
    // isFuzzyQuery的填写规则是,如果是通过身份证查询则填0,通过机票加航班号查询时,手动输入填1,刷票输入填0
    // flightDay的填写规则是,如果用扫码枪扫入,则将机票年月日的日填入,如果手动输入则为-1,查询历史所有
    if (ui->cardCodeLineEdit->text().contains("/", Qt::CaseSensitive)) {
        json.insert("cardId", "");
        json.insert("flightNo", ui->cardCodeLineEdit->text().section("/", 0, 0));
        json.insert("boardingNumber", ui->cardCodeLineEdit->text().section("/", 1, 1));
        json.insert("isFuzzyQuery", 1);
    } else {
        json.insert("cardId", ui->cardCodeLineEdit->text());
        json.insert("flightNo", "");
        json.insert("boardingNumber", "");
        json.insert("isFuzzyQuery", 0);
    }
    json.insert("flightDay", "-1");

    doc.setObject(json);
    QByteArray array = doc.toJson(QJsonDocument::Compact);

    naManager->post(request, array);
}

void QueryDialog::on_fangXingPushButton_clicked()
{
    MessageBox mbx;

    mbx.letGoIconLabelShow();
    mbx.letGoTextLabelShow();

    if (mbx.exec() == QDialog::Accepted) {
        QNetworkRequest request;
        QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

        connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(letgoResponse(QNetworkReply*)));

        // Header
        QString reviewFlowbackQuery = Singleton::getInstance().getConfigQtIni()->value("/interfaces/reviewFlowbackQuery").toString();
        request.setUrl(QUrl(reviewFlowbackQuery));
        QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
        QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
        request.setRawHeader("apiId", apiId.toLatin1());
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        request.setRawHeader("timestamp", timestamp.toLatin1());
        QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
        QString temp = reviewFlowbackQuery.mid(reviewFlowbackQuery.indexOf("/api/v1")) + timestamp + apiKey;
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
        if(document.isObject()){
            QJsonObject obj = document.object();
            if(obj.contains("result")){
                QJsonValue result = obj.value("result");
                if(result.isArray()){
                    QJsonArray array = result.toArray();
                    QJsonValue securityInfo = array.at(0).toObject().value("securityInfo");
                    QString passType = securityInfo.toObject().value("passType").toString();
                    json.insert("securityStatus", passType);
                    QString passTime = securityInfo.toObject().value("passTime").toString();
                    json.insert("securityPassTime", passTime);

                    QJsonValue userInfo = array.at(0).toObject().value("userInfo");
                    QString flightDay = userInfo.toObject().value("flightDay").toString();
                    json.insert("flightDay", flightDay);
                    QString seatId = userInfo.toObject().value("seatId").toString();
                    json.insert("seatId", seatId);
                }
            }
        }
        json.insert("sourceType", 0); // 放行

        doc.setObject(json);
        QByteArray array = doc.toJson(QJsonDocument::Compact);

        naManager->post(request, array);
    }
}

void QueryDialog::on_baoJingPushButton_clicked()
{
    MessageBox mbx;

    mbx.letStayIconLabelShow();
    mbx.letStayTextLabelShow();

    if (mbx.exec() == QDialog::Accepted) {
        QNetworkRequest request;
        QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

        connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(letgoResponse(QNetworkReply*)));

        // Header
        QString reviewManualCheck = Singleton::getInstance().getConfigQtIni()->value("/interfaces/reviewManualCheck").toString();
        request.setUrl(QUrl(reviewManualCheck));
        QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
        QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
        request.setRawHeader("apiId", apiId.toLatin1());
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        request.setRawHeader("timestamp", timestamp.toLatin1());
        QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
        QString temp = reviewManualCheck.mid(reviewManualCheck.indexOf("/api/v1")) + timestamp + apiKey;
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
        if(document.isObject()){
            QJsonObject obj = document.object();
            if(obj.contains("result")){
                QJsonValue result = obj.value("result");
                if(result.isArray()){
                    QJsonArray array = result.toArray();
                    QJsonValue securityInfo = array.at(0).toObject().value("securityInfo");
                    QString passType = securityInfo.toObject().value("passType").toString();
                    json.insert("securityStatus", passType);
                    QString passTime = securityInfo.toObject().value("passTime").toString();
                    json.insert("securityPassTime", passTime);

                    QJsonValue userInfo = array.at(0).toObject().value("userInfo");
                    QString flightDay = userInfo.toObject().value("flightDay").toString();
                    json.insert("flightDay", flightDay);
                    QString seatId = userInfo.toObject().value("seatId").toString();
                    json.insert("seatId", seatId);
                }
            }
        }
        json.insert("sourceType", 1); // 报警

        doc.setObject(json);
        QByteArray array = doc.toJson(QJsonDocument::Compact);

        naManager->post(request, array);
    }
}

void QueryDialog::postResponse(QNetworkReply* reply)
{
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (statusCode.isValid()) {
        qDebug() << "status code = " << statusCode.toInt();
    }

    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);

    if (reason.isValid()) {
        qDebug() << "reason = " << reason;
    }

    QNetworkReply::NetworkError err = reply->error();

    if (err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << reply->errorString();
    } else {
        QByteArray all = reply->readAll();

        QJsonParseError err;
        document = QJsonDocument::fromJson(all, &err);

        if(err.error == QJsonParseError::NoError)
        {
            QJsonObject obj = document.object();

            if (obj.contains("status")) {
                QJsonValue status = obj.value("status");
                qDebug() << "reply success. status: " << status.toDouble();
            }

            if (obj.contains("msg")) {
                QJsonValue msg = obj.value("msg");
                qDebug() << "reply success. msg: " << msg.toString();
            }

            if (obj.contains("result"))
            {
                QJsonValue result = obj.value("result");
                if (result.isNull()) {
                    ui->bodyWidget_1->hide();
                    ui->bodyWidget_3->hide();

                    MessageBox mbx;
                    mbx.hitNullIconLabelShow();
                    mbx.hitNullTextLabelShow();
                    mbx.exec();

                    return;
                }
                if (result.isArray()) {
                    array = result.toArray();

                    if (array.size() == 1) {
                        ui->bodyWidget_3->hide();
                        ui->bodyWidget_1->show();

                        QJsonValue userInfo = array.at(0).toObject().value("userInfo");
                        QJsonValue passengerName = userInfo.toObject().value("passengerName");
                        ui->personalLabel_1->setText("姓名：" + passengerName.toString());
                        QJsonValue certificateNumber = userInfo.toObject().value("certificateNumber");
                        ui->personalLabel_2->setText("证件号码：" + certificateNumber.toString());
                        QJsonValue flightNo = userInfo.toObject().value("flightNo");
                        ui->personalLabel_3->setText("航班号：" + flightNo.toString());
                        QJsonValue boardingNumber = userInfo.toObject().value("boardingNumber");
                        ui->personalLabel_4->setText("序列号：" + boardingNumber.toString());

                        QJsonValue securityInfo = array.at(0).toObject().value("securityInfo");
                        QJsonValue channelName = securityInfo.toObject().value("channelName");
                        QJsonValue passTime = securityInfo.toObject().value("passTime");
                        QString resultLabel = passTime.toString().mid(8, 2) + ":" + passTime.toString().mid(10, 2) + ":" + passTime.toString().right(2);
                        ui->resultLabel_2->setText(channelName.toString() + "  " + resultLabel);

                        QJsonValue passType = securityInfo.toObject().value("passType");
                        QJsonValue passStatus = securityInfo.toObject().value("passStatus");
                        QString labelText;
                        if (passType.toInt() == 0) {
                            labelText = "人证1:1";
                        } else if (passType.toInt() == 1) {
                            labelText = "人工放行";
                        } else if (passType.toInt() == 2) {
                            labelText = "闸机B门通过";
                        } else if (passType.toInt() == 3) {
                            labelText = "验票通过";
                        } else if (passType.toInt() == 4) {
                            labelText = "未知";
                        } else {
                            labelText = "程序错误";
                        }
                        if (passStatus.toInt() == 0) {
                            labelText += " 通过";
                        } else if (passStatus.toInt() == 1) {
                            labelText += " 未通过";
                        } else if (passStatus.toInt() == 2) {
                            labelText += " 未知";
                        } else if (passStatus.toInt() == 3) {
                            labelText += " 身份证过期";
                        } else {
                            labelText += " 程序错误";
                        }
                        ui->resultLabel_1->setText(labelText);

                        QString photoPath = securityInfo.toObject().value("photoPath").toString();
                        QPixmap pixmap = getQPixmapSync(photoPath);

                        pixmap = pixmap.scaled(ui->portraitLabel->width()
                                               , ui->portraitLabel->height()
                                               , Qt::IgnoreAspectRatio
                                               , Qt::SmoothTransformation);
                        ui->portraitLabel->setPixmap(pixmap);
                    } else {
                        ui->bodyWidget_1->hide();

                        ui->flightTableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
                        ui->flightTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
                        ui->flightTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
                        ui->flightTableWidget->verticalHeader()->setVisible(false);
                        ui->flightTableWidget->horizontalHeader()->setVisible(false);
                        ui->flightTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                        ui->flightTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                        ui->flightTableWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
                        ui->flightTableWidget->setShowGrid(false);
                        ui->flightTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color: rgba(0, 0, 0, 0);"
                                                                                " border: 0;"
                                                                                " border-bottom: 1px solid #094b57;"
                                                                                " color: white; }");
                        ui->flightTableWidget->setRowCount(0);
                        ui->flightTableWidget->setColumnCount(0);
                        ui->flightTableWidget->setRowHeight(0, 226);
                        ui->flightTableWidget->insertRow(0);
                        ui->flightTableWidget->setRowHeight(0, 226);

                        for (int i = 0; i < array.size(); i++) {
                            ui->flightTableWidget->setColumnWidth(i, 146);
                            ui->flightTableWidget->insertColumn(i);
                            ui->flightTableWidget->setColumnWidth(i, 146);

                            QJsonValue securityInfo = array.at(i).toObject().value("securityInfo");
                            QJsonValue userInfo = array.at(i).toObject().value("userInfo");

                            QString photoPath = securityInfo.toObject().value("photoPath").toString();
                            QPixmap pixmap = getQPixmapSync(photoPath);
                            pixmap = pixmap.scaled(93
                                                   , 127
                                                   , Qt::IgnoreAspectRatio
                                                   , Qt::SmoothTransformation);

                            QLabel *picLabel = new QLabel();
                            picLabel->setPixmap(pixmap);
                            picLabel->setAlignment(Qt::AlignCenter);
                            QFont font;
                            QLabel *textLabel = new QLabel();
                            font.setPointSize(11);
                            textLabel->setFont(font);
                            textLabel->setStyleSheet("color: rgb(2, 255, 157);");
                            textLabel->setText("航班号/序列号");
                            textLabel->setAlignment(Qt::AlignCenter);
                            QLabel *noLabel = new QLabel();
                            font.setPointSize(13);
                            noLabel->setFont(font);
                            noLabel->setStyleSheet("color: white");
                            noLabel->setText(userInfo.toObject().value("flightNo").toString() + "/" + userInfo.toObject().value("boardingNumber").toString());
                            noLabel->setAlignment(Qt::AlignCenter);
                            QWidget *widget = new QWidget();
                            QVBoxLayout *itemLayout = new QVBoxLayout(widget);
                            itemLayout->addWidget(picLabel);
                            itemLayout->addWidget(textLabel);
                            itemLayout->addWidget(noLabel);
                            ui->flightTableWidget->setCellWidget(0, i, widget);
                        }

                        ui->bodyWidget_3->show();
                    }
                }
            }
        } else {
            qDebug() << "err.error: " << err.error;
        }
    }
}

void QueryDialog::letgoResponse(QNetworkReply* reply)
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
        if(err.error != QJsonParseError::NoError) {
            qDebug() << "err.error: " << err.error;
        }
    }
}

void QueryDialog::letstayResponse(QNetworkReply* reply)
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
        if(err.error != QJsonParseError::NoError) {
            qDebug() << "err.error: " << err.error;
        }
    }
}

void QueryDialog::queryByIdCardSwipe(const FaceAndIdResult& faiResult)
{
    ui->cardCodeLineEdit->clear();
    ui->cardCodeLineEdit->insert(faiResult.code);

    QNetworkRequest request;
    QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

    connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(postResponse(QNetworkReply*)));

    // Header
    QString reviewFlowbackQuery = Singleton::getInstance().getConfigQtIni()->value("/interfaces/reviewFlowbackQuery").toString();
    request.setUrl(QUrl(reviewFlowbackQuery));
    QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
    request.setRawHeader("apiId", apiId.toLatin1());
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    request.setRawHeader("timestamp", timestamp.toLatin1());
    QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
    QString temp = reviewFlowbackQuery.mid(reviewFlowbackQuery.indexOf("/api/v1")) + timestamp + apiKey;
    QByteArray bb = QCryptographicHash::hash(temp.toLatin1(), QCryptographicHash::Md5);
    QString sign = QString().append(bb.toHex());
    request.setRawHeader("sign", sign.toLatin1());

    // Body
    QJsonObject json;
    QJsonDocument doc;
    QString uuid = QUuid::createUuid().toString();
    uuid.remove("{").remove("}").remove("-");
    json.insert("reqId", uuid);
    // isFuzzyQuery的填写规则是,如果是通过身份证查询则填0,通过机票加航班号查询时,手动输入填1,刷票输入填0
    // flightDay的填写规则是,如果用扫码枪扫入,则将机票年月日的日填入,如果手动输入则为-1,查询历史所有
    json.insert("cardId", faiResult.code);
    json.insert("flightNo", "");
    json.insert("boardingNumber", "");
    json.insert("isFuzzyQuery", 0);
    json.insert("flightDay", "-1");

    doc.setObject(json);
    QByteArray array = doc.toJson(QJsonDocument::Compact);

    naManager->post(request, array);
}

void QueryDialog::queryByFlightSwipe(const FlightResult& flight)
{
    ui->cardCodeLineEdit->clear();
    ui->cardCodeLineEdit->insert(flight.flightNo + "/" + flight.boardingNum);

    QNetworkRequest request;
    QNetworkAccessManager* naManager = new QNetworkAccessManager(this);

    connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(postResponse(QNetworkReply*)));

    // Header
    QString reviewFlowbackQuery = Singleton::getInstance().getConfigQtIni()->value("/interfaces/reviewFlowbackQuery").toString();
    request.setUrl(QUrl(reviewFlowbackQuery));
    QString contentType = Singleton::getInstance().getConfigQtIni()->value("/interfaces/contentType").toString();
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    QString apiId = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiId").toString();
    request.setRawHeader("apiId", apiId.toLatin1());
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    request.setRawHeader("timestamp", timestamp.toLatin1());
    QString apiKey = Singleton::getInstance().getConfigQtIni()->value("/interfaces/apiKey").toString();
    QString temp = reviewFlowbackQuery.mid(reviewFlowbackQuery.indexOf("/api/v1")) + timestamp + apiKey;
    QByteArray bb = QCryptographicHash::hash(temp.toLatin1(), QCryptographicHash::Md5);
    QString sign = QString().append(bb.toHex());
    request.setRawHeader("sign", sign.toLatin1());

    // Body
    QJsonObject json;
    QJsonDocument doc;
    QString uuid = QUuid::createUuid().toString();
    uuid.remove("{").remove("}").remove("-");
    json.insert("reqId", uuid);
    // isFuzzyQuery的填写规则是,如果是通过身份证查询则填0,通过机票加航班号查询时,手动输入填1,刷票输入填0
    // flightDay的填写规则是,如果用扫码枪扫入,则将机票年月日的日填入,如果手动输入则为-1,查询历史所有
    json.insert("cardId", "");
    json.insert("flightNo", flight.flightNo);
    json.insert("boardingNumber", flight.boardingNum);
    json.insert("isFuzzyQuery", 0);
    json.insert("flightDay", flight.flightDay.right(2));

    doc.setObject(json);
    qDebug() << "request: " << doc;
    QByteArray array = doc.toJson(QJsonDocument::Compact);

    naManager->post(request, array);
}

void QueryDialog::on_flightTableWidget_cellClicked(int row, int column)
{
    Q_UNUSED(row)

    ui->bodyWidget_3->hide();
    ui->bodyWidget_1->show();

    QJsonValue userInfo = array.at(column).toObject().value("userInfo");
    QJsonValue passengerName = userInfo.toObject().value("passengerName");
    ui->personalLabel_1->setText("姓名：" + passengerName.toString());
    QJsonValue certificateNumber = userInfo.toObject().value("certificateNumber");
    ui->personalLabel_2->setText("证件号码：" + certificateNumber.toString());
    QJsonValue flightNo = userInfo.toObject().value("flightNo");
    ui->personalLabel_3->setText("航班号：" + flightNo.toString());
    QJsonValue boardingNumber = userInfo.toObject().value("boardingNumber");
    ui->personalLabel_4->setText("序列号：" + boardingNumber.toString());

    QJsonValue securityInfo = array.at(column).toObject().value("securityInfo");
    QJsonValue channelName = securityInfo.toObject().value("channelName");
    QJsonValue passTime = securityInfo.toObject().value("passTime");
    QString resultLabel = passTime.toString().mid(8, 2) + ":" + passTime.toString().mid(10, 2) + ":" + passTime.toString().right(2);
    ui->resultLabel_2->setText(channelName.toString() + "  " + resultLabel);

    QJsonValue passType = securityInfo.toObject().value("passType");
    if (passType.toInt() == 0) {
        ui->resultLabel_1->setText(tr("人证1:1"));
    } else if (passType.toInt() == 1) {
        ui->resultLabel_1->setText(tr("人工放行"));
    } else if (passType.toInt() == 2) {
        ui->resultLabel_1->setText(tr("闸机B门通过"));
    } else if (passType.toInt() == 3) {
        ui->resultLabel_1->setText(tr("验票通过"));
    } else if (passType.toInt() == 4) {
        ui->resultLabel_1->setText(tr("未知"));
    } else {
        ui->resultLabel_1->setText(tr("程序错误"));
    }

    QString photoPath = securityInfo.toObject().value("photoPath").toString();
    QPixmap pixmap = getQPixmapSync(photoPath);

    pixmap = pixmap.scaled(ui->portraitLabel->width()
                           , ui->portraitLabel->height()
                           , Qt::IgnoreAspectRatio
                           , Qt::SmoothTransformation);
    ui->portraitLabel->setPixmap(pixmap);
}
