#include "Include/dataanalysis.h"
#include "Include/functions.h"
#include "Include/paramdef.h"
#include <QDebug>
#include <algorithm>
#include <QString>
#include <QDateTime>
#include <QTextCodec>
#include <QByteArray>
#include <QDateTime>
#include <QPixmap>
#include <QPointer>
#include <QThread>
#include "Include/functions.h"
#include "Include/singleton.h"

DataAnalysis::DataAnalysis() : QObject (nullptr)
{
    m_FISDKParam.nIdCardInfoType = 2;
    m_FISDKParam.video1.index = 0;
    m_FISDKParam.video1.userData = this;
    m_FISDKParam.video1.cb = Video1DetectCallback;
    m_FISDKParam.video1.faceDetect = true;
    if ("USB" == Singleton::getInstance().getSystemParamXml()["VideoType"]) {
        QString videoParam = QString("USB-Camera,%1,640,480,%2")
                .arg(Singleton::getInstance().getSystemParamXml()["VideoPort0"])
                .arg(Singleton::getInstance().getSystemParamXml()["VideoFrame"]);
        strcpy(m_FISDKParam.video1.videoParam, videoParam.toLocal8Bit());
        strcpy(m_FISDKParam.video1.sourceType, "usb");
    }

    m_FISDKParam.video2.index = 1;
    m_FISDKParam.video2.userData = this;
    m_FISDKParam.video2.cb = Video2DetectCallback;
    m_FISDKParam.video2.faceDetect = false;
    if ("USB" == Singleton::getInstance().getSystemParamXml()["VideoType"]) {
        QString videoParam = QString("USB-Camera,%1,640,480,%2")
                .arg(Singleton::getInstance().getSystemParamXml()["VideoPort1"])
                .arg(Singleton::getInstance().getSystemParamXml()["VideoFrame"]);
        strcpy(m_FISDKParam.video2.videoParam, videoParam.toLocal8Bit());
        strcpy(m_FISDKParam.video2.sourceType, "usb");
    }

    m_OptimumParam.nMaxTrackFaceNum = 1;
    m_OptimumParam.nOptTimeLength = Singleton::getInstance().getSystemParamXml()["FaceOptTimeLength"].toInt();
    m_OptimumParam.nTrigTimeInterval = 1000;
    m_OptimumParam.nDetFrmInterval = Singleton::getInstance().getSystemParamXml()["DetFrmInterval"].toInt();
    m_OptimumParam.nMaxDetInterval = Singleton::getInstance().getSystemParamXml()["MaxDetInterval"].toInt();
    m_OptimumParam.nEvalFrmFactor = Singleton::getInstance().getSystemParamXml()["EvalFrmFactor"].toInt();
    m_OptimumParam.nPicType = PicRGB;
    m_OptimumParam.nTriggerType = TrigByEvent;
    m_OptimumParam.fDetRatio = Singleton::getInstance().getSystemParamXml()["ImageSacle"].toFloat();
    m_OptimumParam.fQualityLimit = Singleton::getInstance().getSystemParamXml()["FaceQualityLimit"].toFloat();
    m_OptimumParam.fKptCfdLimit = Singleton::getInstance().getSystemParamXml()["FaceKptCfdLimit"].toFloat();
    m_OptimumParam.fAngleXLimit = Singleton::getInstance().getSystemParamXml()["FaceAngleXLimit"].toFloat();
    m_OptimumParam.fAngleYLimit = Singleton::getInstance().getSystemParamXml()["FaceAngleYLimit"].toFloat();
    m_OptimumParam.fAngleZLimit = Singleton::getInstance().getSystemParamXml()["FaceAngleZLimit"].toFloat();

    int res = CoreFIInit(&m_FISDKParam, &m_OptimumParam);

    if (res != 0) {
        qDebug() << "CoreFIInit failed: " << res;
        throw std::runtime_error("CoreFIInit failed.");
    }
}

DataAnalysis::~DataAnalysis()
{
    CoreFIRelease();
}

void DataAnalysis::RegistCallback()
{
    CoreFISetHTTPRequstCB(IDCardRequstCBCallBack, FlightInfoRequstCallBack, this);
}
