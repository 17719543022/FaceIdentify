#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>
#include <QMap>
#include <QImage>
#include "DetectOptimumDef.h"
#include "ParamDefine.h"
#include <QPixmap>

void STDCALL Video1DetectCallback(char *videoData, int dataLen, int videoWidth, int videoHeight, int faceNum, int facePos[][5], void *userData);
void STDCALL Video2DetectCallback(char *videoData, int dataLen, int videoWidth, int videoHeight, int faceNum, int facePos[][5], void *userData);
void STDCALL IDCardRequstCBCallBack(IDInfos *pIdInfos, void *userData);
void STDCALL FlightInfoRequstCallBack(FlightInfo* pFlightInfos, void* userData);

QByteArray fillWithQImage(const QImage &image);
QByteArray fillWithQPixmap(const QPixmap &pixmap);

#endif // FUNCTIONS_H
