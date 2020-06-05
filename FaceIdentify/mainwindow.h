#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Include/dataanalysis.h"
#include <QVector>
#include <qtablewidget.h>
#include <QNetworkReply>
#include "subscreen.h"
#include "querydialog.h"
#include <QSqlQueryModel>
#include <QEvent>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool event(QEvent* ev)
    {
        if (ev->type() == QEvent::KeyPress
                || ev->type() == QEvent::HoverMove
                || ev->type() == QEvent::MouseButtonPress
                || ev->type() == QEvent::WindowUnblocked) {
            timer->stop();
            timer->start(60000);

            connect(timer, SIGNAL(timeout()), this, SLOT(oncedown()));
        }
        if (ev->type() == QEvent::WindowBlocked) {
            timer->stop();
        }

        return QWidget::event(ev);
    }

/*********************************
 * frontPageWidget
 * *******************************/

private slots:
    void on_guanBiPushButton_clicked();

    void on_liShiTableWidget_itemClicked(QTableWidgetItem *item);

    void on_huiChaPushButton_clicked();

    void on_settingPushButton_clicked();

    void on_beiJiangPushButton_clicked();

    void on_fangXingPushButton_clicked();

    void on_freshotPushButton_clicked();

    void on_fletpassPushButton_clicked();

    void letpassResponse(QNetworkReply* reply);

    void nofaceFangXing(QNetworkReply* reply);

    void comparefailFangXing(QNetworkReply* reply);

    void on_fceHuiPushButton_clicked();

    void withdrawResponse(QNetworkReply* reply);

    void on_video2PushButton_clicked();

    void switchToBackPage();

    void switchToDiversion();

    void setFrontPageNotByPassed(int);

    void countdown();

    void oncedown();

private:
    void video1Show(const QImage& image, const QVector<QRect>& facePos);

    void video2Show(const QImage& image, const QVector<QRect>& facePos);

    void idCardResponse(const FaceAndIdResult& faiResult);

    void updateIdCard(const FaceAndIdResult& faiResult);

    void displayIdCard(const FaceAndIdResult& faiResult);

    void databaseIdCard(const FaceAndIdResult& faiResult);

    void flightResponse(const FlightResult& flightResult);

    void updateFlightInfo(const FlightResult& flightResult);

    void displayFlight(const FlightResult& flightResult);

    void databaseFlight(const FlightResult& flightResult);

private:
    Ui::MainWindow* ui;
    DataAnalysis* dataAnalysis;
    QVector<FaceAndIdResult> faiResultVector;
    QPixmap vpixmap;
    QTimer *timer;

    QMetaObject::Connection video1Connection;
    QMetaObject::Connection video2Connection;
    QMetaObject::Connection idCardConnection;
    QMetaObject::Connection flightConnection;
    QMetaObject::Connection tableConnection;
    QMetaObject::Connection withdrawConnection;

    OptimumFace *optimumFace;
    FlightInfo flightInfo;
    QPixmap flightPixmap;
    QString shotTime;
    int faceNum;
    int status;
    SubScreen *pSubScreen;
    QueryDialog *pQueryDialog;
    bool isFaceDetectVideoChanged;
    bool isFrontPageByPassed;

/*********************************
 * beiJiangPushButton
 * *******************************/
private:
    int secondCounts;
    bool isInDiversionMode;

/*********************************
 * backPageWidget
 * *******************************/

private slots:
    void on_bfanHuiButton_clicked();

    void on_bchaXunPushButton_clicked();

    void on_bshuJuTableWidget_itemClicked(QTableWidgetItem *item);

    void on_bpgNextPushButton_clicked();

    void on_bpgPrePushButton_clicked();

    void on_bpgEndPushButton_clicked();

    void on_bpgFirstPushButton_clicked();

    void on_bqieHuanPushButton_clicked();

    void on_bfqieHuanPushButton_clicked();

    void on_bfchaXunPushButton_clicked();

    void on_bfpgFirstPushButton_clicked();

    void on_bfpgPrePushButton_clicked();

    void on_bfpgNextPushButton_clicked();

    void on_bfpgEndPushButton_clicked();

    void on_bfshuJuTableWidget_itemClicked(QTableWidgetItem *item);

private:
    QSqlQueryModel *queryModel;

    int rowCountPerPage;
    int rowCount;
    int pageCount;
    int currentPage;
    int currentRows;

private:
    void updatePageLabel();

    void updatePageRows();

    void fupdatePageLabel();

    void fupdatePageRows();
};

#endif // MAINWINDOW_H
