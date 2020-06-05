#ifndef PARAMDEF_H
#define PARAMDEF_H

#include <QString>
#include <QImage>
#include "ParamDefine.h"
#include <QDateTime>
#include "Include/functions.h"
#include <QtDebug>
#include "Include/singleton.h"
#include "FaceIdentifySDK.h"

namespace {
void idImageJpg(const IDInfos *pIdInfos, QImage &idImage)
{
    if (pIdInfos->imgInfo.idImageType == IDImage_JPEG) {
        idImage = QImage::fromData(reinterpret_cast<uchar *>(pIdInfos->imgInfo.idImageJpeg), pIdInfos->imgInfo.idImageJpegLen, "JPG");
    } else {
        idImage = QImage();
    }
}

void liveImageJpg(const FaceIdentifyResult *pFIResult, QImage &liveImage)
{
    if (pFIResult->liveImageLen != 0) {
        liveImage = QImage(reinterpret_cast<uchar*>(pFIResult->liveImageRGB)
                           , pFIResult->liveImageWidth
                           , pFIResult->liveImageHeight
                           , 3 * pFIResult->liveImageWidth
                           , QImage::Format_RGB888).rgbSwapped();
    }
}

bool isExpired(QString expireStart, QString expireEnd, QString currTime)
{
    QString temp = currTime.remove(":").remove(" ").remove("/").left(8);

    if ((expireStart <= temp) && (temp <= expireEnd)) {
        return false;
    }

    return true;
}
}
enum E_STATUS
{
    STATUS_DEFAULT = 0x00,         // 默认值
    STATUS_COMPARE_PASS = 0x01,    // 比对通过
    STATUS_COMPARE_FAIL = 0x02,    // 比对不通过
    STATUS_DETECT_NO_FACE = 0x03,  // 无人脸信息
    STATUS_ARTIFICIAL_PASS = 0x04, // 人工放行
    STATUS_IDCARD_EXPIRE = 0x10,   // 证件失效
};

struct FaceAndIdResult
{
    int      idType; // 身份证类型 0-国人身份证 1-外国人身份证
    QString  name; // brief 姓名
    QString  eName; // 英文姓名
    QString  gender; // 性别
    QString  folk; // 名族
    QString  birthDay; // 出生日期
    QString  code; // 身份证号
    QString  address; // 地址
    QString  agency; // 发证机关
    QString  expireStart; // 有效期起
    QString  expireEnd; // 有效期止
    bool     isExpire; // 证件是否失效
    QString  prCode; // 永久居住代码
    QString  nation; // 国籍或所在地区代码
    QString  idVersion; // 证件版本号
    QString  agencyCode; // 受理机关代码
    int		 idImageType; // 证件照类型
    QImage	 idImage; // 证件照
    QString  cardFeature;
    QImage   liveImage; // 现场照
    QString  sceneFeature;
    int      matchResult; // 人证比对结果 表示人证比对的几种状态  1身份证未检测到人脸 2现场照未未检测到人脸 3其他错误情况 0已获得比对分数
    float    matchScore; // 人证比对得分
    int      matchStatus; //
    float    liveImageAge; // 现场照年龄
    QString  liveImageGender; // 现场照性别
    QString  matchTime; // 比对时间

    FaceAndIdResult () {
        idType = 0;
        name = "";
        eName = "";
        gender = "";
        folk = "";
        birthDay = "";
        code = "";
        address = "";
        agency = "";
        expireStart = "";
        expireEnd = "";
        isExpire = false;
        prCode = "";
        nation = "";
        idVersion = "";
        agencyCode = "";
        idImageType = 0;
        idImage = QImage();
        cardFeature = "";
        liveImage = QImage();
        sceneFeature = "";
        matchResult = -1;
        matchScore = 0.0f;
        matchStatus = STATUS_DEFAULT;
        liveImageAge = 0;
        liveImageGender = "";
        matchTime = "";
    }

    FaceAndIdResult (const IDInfos *pIdInfos, const FaceIdentifyResult *pFIResult) {
        QString currTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");

        idType = pIdInfos->idType;
        name = pIdInfos->name;
        eName = pIdInfos->eName;
        gender = pIdInfos->gender;
        folk = pIdInfos->folk;
        birthDay = pIdInfos->birthDay;
        code = pIdInfos->code;
        address = pIdInfos->address;
        agency = pIdInfos->agency;
        expireStart = pIdInfos->expireStart;
        expireEnd = pIdInfos->expireEnd;
        isExpire = isExpired(pIdInfos->expireStart, pIdInfos->expireEnd, currTime);
        prCode = pIdInfos->prCode;
        nation = pIdInfos->nation;
        idVersion = pIdInfos->idVersion;
        agencyCode = pIdInfos->agencyCode;
        idImageType = pIdInfos->imgInfo.idImageType;
        idImageJpg(pIdInfos, idImage);
        char feature[8192];
        CoreFIGetFeature(pIdInfos->imgInfo.idImageJpeg, pIdInfos->imgInfo.idImageJpegLen, feature);
        cardFeature = QString(QByteArray(feature, 8192).toBase64());
        liveImageJpg(pFIResult, liveImage);
        CoreFIGetFeature(pFIResult->liveImageRGB, pFIResult->liveImageLen, feature);
        sceneFeature = QString(QByteArray(feature, 8192).toBase64());
        matchResult = pFIResult->matchResult;
        matchScore = pFIResult->matchScore;
        switch (pFIResult->matchResult) {
        case 0:
            matchStatus = (pFIResult->matchScore >= Singleton::getInstance().getSystemParamXml()["RecogThreshold"].toFloat())
                    ? STATUS_COMPARE_PASS
                    : STATUS_COMPARE_FAIL;
            break;
        case 1:
            matchStatus = STATUS_DETECT_NO_FACE;
            break;
        case 2:
            matchStatus = STATUS_DETECT_NO_FACE;
            break;
        case 3:
        default:
            matchStatus = STATUS_DEFAULT;
            break;
        }
        if (isExpire) {
            matchStatus = matchStatus | STATUS_IDCARD_EXPIRE;
        }
        liveImageAge = pFIResult->liveImageAge;
        liveImageGender = pFIResult->liveImageGender;
        matchTime = currTime;
    }
};

struct FlightResult {
    QString flightNo;
    QString boardingNum;
    QString setId;
    QString startPort;
    QString terminalPort;
    QString flightDay;
    int     kindType;

    FlightResult()
    {
        flightNo = QString();
        boardingNum = QString();
        setId = QString();
        startPort = QString();
        terminalPort = QString();
        flightDay = QString();
        kindType = -1;
    }

    FlightResult(const FlightInfo& flightInfo)
    {
        flightNo = QString(QLatin1String(flightInfo.flightNo));
        boardingNum = QString(QLatin1String(flightInfo.boardingNum));
        setId = QString(QLatin1String(flightInfo.setId));
        startPort = QString(QLatin1String(flightInfo.startPort));
        terminalPort = QString(QLatin1String(flightInfo.terminalPort));
        flightDay = QString(QLatin1String(flightInfo.flightDay));
        kindType = flightInfo.kindType;
    }
};

#endif // PARAMDEF_H
