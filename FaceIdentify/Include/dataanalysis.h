#ifndef DATAANALYSIS_H
#define DATAANALYSIS_H

#include "Include/paramdef.h"
#include "FaceIdentifySDK.h"
#include "ParamDefine.h"
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <cstdint>
#include <QImage>

class DataAnalysis: public QObject
{
    Q_OBJECT
public:
     explicit DataAnalysis();
    ~DataAnalysis();
    void RegistCallback();

signals:
    void video1Detect(const QImage& image, const QVector<QRect>& faceList);
    void video2Detect(const QImage& image, const QVector<QRect>& faceList);
    void idCardResponse(const FaceAndIdResult& faiResult);
    void flightResponse(const FlightResult& flightResult);

private:
    FISDKParam   m_FISDKParam;
    OptimumParam m_OptimumParam;
};

#endif // DATAANALYSIS_H
