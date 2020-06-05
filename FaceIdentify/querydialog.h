#ifndef QUERYDIALOG_H
#define QUERYDIALOG_H

#include <QDialog>
#include "Include/paramdef.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>

namespace Ui {
class QueryDialog;
}

class QueryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueryDialog(QWidget *parent = nullptr);
    ~QueryDialog();

private:
    QPixmap getQPixmapSync(QString str);

private slots:
    void on_closePushButton_clicked();

    void on_queryPushButton_clicked();

    void on_fangXingPushButton_clicked();

    void on_baoJingPushButton_clicked();

    void postResponse(QNetworkReply* reply);

    void letgoResponse(QNetworkReply* reply);

    void letstayResponse(QNetworkReply* reply);

    void queryByIdCardSwipe(const FaceAndIdResult& faiResult);

    void queryByFlightSwipe(const FlightResult& flight);

    void on_flightTableWidget_cellClicked(int row, int column);

private:
    Ui::QueryDialog *ui;
    QJsonDocument document;
    QJsonArray array;
};

#endif // QUERYDIALOG_H
