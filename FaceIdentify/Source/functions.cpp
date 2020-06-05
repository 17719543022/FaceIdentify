#include "Include/functions.h"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <QString>
#include <QDomDocument>
#include <QMap>
#include "Include/dataanalysis.h"
#include <QDateTime>
#include "ParamDefine.h"
#include "Include/singleton.h"
#include <QBuffer>

void Video1DetectCallback(char *videoData, int dataLen, int videoWidth, int videoHeight, int faceNum, int facePos[][5], void *userData)
{
    if (dataLen == 0 || videoWidth == 0 || videoHeight == 0) {
        qCritical() << QString("dataLen = %1, videoWidth = %2, videoHeight = %3.").arg(dataLen).arg(videoWidth).arg(videoHeight);
        return;
    }

    if (videoWidth != 0 && videoHeight != 0) {
        QImage image(reinterpret_cast<uchar*>(videoData), videoWidth, videoHeight, 3*videoWidth, QImage::Format_RGB888);
        QVector<QRect> faceList;

        int fn = faceNum > Singleton::getInstance().getMaxDetectFace() ?
                    Singleton::getInstance().getMaxDetectFace() : faceNum;
        for (int i = 0; i < fn; i++) {
            QRect rect(QPoint(facePos[i][0], facePos[i][1]), QPoint(facePos[i][2], facePos[i][3]));
            faceList << rect;
        }

        // CoreFISetVideoDetectCB(Video1DetectCallback, this);
        // so: userData -> this
        DataAnalysis *dataAnalysis = static_cast<DataAnalysis*>(userData);
        emit dataAnalysis->video1Detect(image, faceList);
    } else {
        qCritical() << QString("Video1DetectCallback failed, videoWidth %1 videoHeight %2.").arg(videoWidth).arg(videoHeight);
    }
}

void Video2DetectCallback(char *videoData, int dataLen, int videoWidth, int videoHeight, int faceNum, int facePos[][5], void *userData)
{
    if (dataLen == 0 || videoWidth == 0 || videoHeight == 0) {
        qCritical() << QString("dataLen = %1, videoWidth = %2, videoHeight = %3.").arg(dataLen).arg(videoWidth).arg(videoHeight);
        return;
    }

    if (videoWidth != 0 && videoHeight != 0) {
        QImage image(reinterpret_cast<uchar*>(videoData), videoWidth, videoHeight, 3*videoWidth, QImage::Format_RGB888);
        QVector<QRect> faceList;

        int fn = faceNum > Singleton::getInstance().getMaxDetectFace() ?
                    Singleton::getInstance().getMaxDetectFace() : faceNum;
        for (int i = 0; i < fn; i++) {
            QRect rect(QPoint(facePos[i][0], facePos[i][1]), QPoint(facePos[i][2], facePos[i][3]));
            faceList << rect;
        }

        // CoreFISetVideoDetectCB(Video2DetectCallback, this);
        // so: userData -> this
        DataAnalysis *dataAnalysis = static_cast<DataAnalysis*>(userData);
        emit dataAnalysis->video2Detect(image, faceList);
    } else {
        qCritical() << QString("Video2DetectCallback failed, videoWidth %1 videoHeight %2.").arg(videoWidth).arg(videoHeight);
    }
}

void IDCardRequstCBCallBack(IDInfos *pIdInfos, void *userData)
{
    FaceIdentifyResult fiResult;
    CoreFIInputIdInfo(pIdInfos, &fiResult);
    FaceAndIdResult faiResult(pIdInfos, &fiResult);

    DataAnalysis *dataAnalysis = static_cast<DataAnalysis*>(userData);
    emit dataAnalysis->idCardResponse(faiResult);
}

void FlightInfoRequstCallBack(FlightInfo* pFlightInfos, void* userData)
{
    DataAnalysis *dataAnalysis = static_cast<DataAnalysis*>(userData);

    const FlightResult flightResult(*pFlightInfos);

    emit dataAnalysis->flightResponse(flightResult);
}

QByteArray fillWithQImage(const QImage &image)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG");
    buffer.close();

    return byteArray;
}

QByteArray fillWithQPixmap(const QPixmap &pixmap)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "JPG");
    buffer.close();

    return byteArray;
}
