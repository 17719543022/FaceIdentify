#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include "Include/functions.h"
#include <QDir>
#include <QDebug>
#include "FaceIdentifySDK.h"
#include "ParamDefine.h"
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlQuery>
#include "Include/singleton.h"

namespace {
bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("firesultdatabase");
    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "This example needs SQLite support. Please read "
                        "the Qt SQL driver documentation for information how "
                        "to build it.\n\n"
                        "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }

    QSqlQuery query1;

    query1.exec(QLatin1String("create table person (id varchar(50) primary key"
                              ", passagewayId int"
                              ", idType int"
                              ", name varchar(50)"
                              ", eName varchar(50)"
                              ", gender varchar(50)"
                              ", folk varchar(50)"
                              ", birthDay varchar(50)"
                              ", code varchar(50)"
                              ", address varchar(50)"
                              ", agency varchar(50)"
                              ", expireStart varchar(50)"
                              ", expireEnd varchar(50)"
                              ", isExpire bool"
                              ", prCode varchar(50)"
                              ", nation varchar(50)"
                              ", idVersion varchar(50)"
                              ", agencyCode varchar(50)"
                              ", idImageType int"
                              ", idImage BLOB"
                              ", liveImage BLOB"
                              ", liveImageAge float"
                              ", liveImageGender varchar(50)"
                              ", matchResult int"
                              ", matchScore float"
                              ", matchStatus int"
                              ", matchScoreThreshold float"
                              ", matchTime varchar(50))"));

    QSqlQuery query2;
    query2.exec(QLatin1String("create table flight (id varchar(50) primary key"
                              ", flightNo varchar(50)"
                              ", boardingNum varchar(50)"
                              ", setId varchar(50)"
                              ", startPort varchar(50)"
                              ", terminalPort varchar(50)"
                              ", flightDay varchar(50)"
                              ", kindType int"
                              ", shotImage BLOB"
                              ", shotTime varchar(50)"
                              ", isUpLoaded bool"
                              ", source varchar(50))"));

    return true;
}
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<IDInfos>("IDInfos");
    qRegisterMetaType<IDInfos>("IDInfos&");
    qRegisterMetaType<FaceIdentifyResult>("FaceIdentifyResult");
    qRegisterMetaType<FaceIdentifyResult>("FaceIdentifyResult&");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QItemSelection>("QItemSelection");
    qRegisterMetaType<QVector<QRect>>("QVector<QRect>");
    qRegisterMetaType<FaceAndIdResult>("FaceAndIdResult");
    qRegisterMetaType<FaceAndIdResult>("FaceAndIdResult&");
    qRegisterMetaType<FlightResult>("FlightResult");
    qRegisterMetaType<FlightResult>("FlightResult&");

    if (!createConnection())
        return -1;

#if 0
    Singleton::getInstance().dump();

    qDebug() << "current applicationDirPath: " << QCoreApplication::applicationDirPath();
    qDebug() << "current currentPath: " << QDir::currentPath();
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
