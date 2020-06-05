//////////////////////////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE  
/// Copyright(c)2018,中科院重庆研究院智能安全技术研究中心  （版权声明）  
/// All rights reserved.  
/// \file VideoProcessSer.h 
/// \brief 参数定义
///
/// 包含相关结构体定义，枚举定义，回调函数定义
///
/// \author plusone_chen 
/// \version V1.0.0(main.min，主版本.分版本格式)  
/// \date 2018/4/19
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARAMDEFINE_H
#define PARAMDEFINE_H

#include <stdio.h>
#include <string.h>
#include "DetectOptimumDef.h" 

#define IN
#define OUT
///<breif 函数返回值对照表
#define FI_RET_SUCCESS				0		// 返回成功
#define FI_RET_FAILURE				3000	// 返回失败
#define	FI_RET_IDCARD_FAIL			3001	// 身份证读卡器初始化错误
#define	FI_RET_CREFEA_FAIL			3002	// 创建特征提取通道错误
#define	FI_RET_CRECOMP_FAIL			3003	// 创建相似度比较通道错误
#define	FI_RET_PARAM_FAIL			3004	// 输入参数错误
#define FI_RET_GET_FEATURE_FAIL		3005	// 提取特征错误
#define FI_RET_GET_AGE_FAIL			3006	// 获取年龄错误
#define FI_RET_COMPARE_FEATURE		3007	// 对比特征错误
#define FI_RET_NET_ERROR			3008	// 网络错误
#define FI_RET_NO_BESTFACE			3009	// 没有最佳人脸 
#define FI_RET_SERVERS_ERROR		3010	// 服务器返回错误
#define FI_RET_NOT_SUPPORT          3011    // 不支持的功能
#define FI_RET_NOT_INIT				3012    // 未初始化

#ifdef _WIN32
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif

//视频检测回调函数
typedef void (CALLBACK *VideoDetectCB)(char *videoData, int dataLen, int videoWidth,int videoHeight,int faceNum,int facePos[][5],void *userData);

// 证件照传入的格式
enum FIIDImageTypeEnum
{
	IDImage_RGB = 0,	// RGB数据
	IDImage_ImgPath = 1,// 图像地址
	IDImage_BMP = 2,	// bmp数据
	IDImage_JPEG = 3	// jpeg数据
};

// 人证比对结果返回
enum FaceIdentifyResultEnum
{
	FI_Res_Success = 0,		// 人证比对成功
	FI_Res_IDNoFace = 1,	// 证件照未检测到人脸
	FI_Res_LiveNoFace = 2,	// 现场照未检测到人脸
	FI_Res_Error = 3		// 其他错误情况
};

typedef struct _VideoInfo
{
	int     index;              //视频源编号号 【保证唯一性即可】
	char	sourceType[64];		//源类型, 字符串为空时不打开视频 
	char	videoParam[128];	//视频参数
	VideoDetectCB cb;           //视频回调
	void*   userData;           //用户参数
	bool    faceDetect;         //人脸检测 【只有一个视频能用于检测】
	_VideoInfo():
		index(0),
		cb(NULL),
		userData(NULL),
		faceDetect(false)
	{
		memset(sourceType, 0, sizeof(sourceType));
		memset(videoParam, 0, sizeof(videoParam));
	}
}VideoInfo;

// 人证比对初始化参数结构体
typedef struct FISDKParam_s
{
	int				nMinFaceSize;		//最小人脸
	int				nMaxFaceSize;		//最大人脸
	char            configPath[260];	//配置文件路径
	int				nIdCardInfoType;	//读卡器信息 0接口传入 1读卡器读入【读卡器目前只支持windows】 2通过HTTP服务获取身份证信息
	VideoInfo       video1;             //视频源1
	VideoInfo       video2;             //视频源2

	FISDKParam_s():
		nMinFaceSize(20),
		nMaxFaceSize(200),
		nIdCardInfoType(1)
	{
		sprintf(configPath, "%s", "config.ini");
		
	}

	FISDKParam_s(int minFace, int maxFace, int isIdCard):
		nMinFaceSize(minFace),
		nMaxFaceSize(maxFace),
		nIdCardInfoType(isIdCard)
	{
		memset(configPath, 0, sizeof(configPath));
	}

}FISDKParam;

/*证件照图像信息
*/
typedef struct IDImageInfo_s
{
	int		idImageType;	//证件照类型
	char*	idImageRGB;		//证件照数据  RGB 格式
	int		idImageRGBLen;	//RGB格式的数据长度
	int		idImageWidth;	//证件照宽度
	int		idImageHeight;	//证件照高度
	char*	idImagePath;	//证件照路径
	int     idImagePathLen;	//路径长度
	char*	idImageBMP;		//证件照数据  BMP格式
	int     idImageBMPLen;	//BMP数据长度
	char*	idImageJpeg;	//JPEG数据
	int		idImageJpegLen;	//JpEG数据长度

	IDImageInfo_s()
	{
		idImageType = 0;
		idImageRGB = 0;
		idImageRGBLen = 0;
		idImageWidth = 0;
		idImageHeight = 0;
		idImagePath = 0;
		idImagePathLen = 0;
		idImageBMP = 0;
		idImageBMPLen = 0;
		idImageJpeg = 0;
		idImageJpegLen = 0;
	};
}IDImageInfo;

/// \breif 证件信息
typedef struct IDInfos_s
{
	int idType;		// 证件类型 0-国人身份证 1-外国人身份证	[必须]
	char name[32];	// 姓名								
	char gender[8];	// 性别	(Female or Male)
	char folk[10];	// 名族								[必须]
	char birthDay[10];	// 出生日期 格式YYYYMMDD			[必须]
	char code[20];		// 证件号码						[必须]
	char address[72];	// 地址
	char agency[32];	// 发证机关
	char expireStart[10];	// 有效期起 格式YYYYMMDD		[必须]
	char expireEnd[10];		// 有效期止 格式YYYYMMDD		[必须]
	char eName[64];			// 英文姓名
	char prCode[16];		// 永久居住代码
	char nation[8];			// 国籍或所在地区代码			[必须]
	char idVersion[4];		// 证件版本号
	char agencyCode[6];		// 受理机关代码
	IDImageInfo imgInfo;

	IDInfos_s()  //数据初始化
	{
		idType = 0;
		memset(name, 0, sizeof(name));
		memset(eName, 0, sizeof(eName));
		memset(gender, 0, sizeof(gender));
		memset(folk, 0, sizeof(folk));
		memset(birthDay, 0, sizeof(birthDay));
		memset(code, 0, sizeof(code));
		memset(address, 0, sizeof(address));
		memset(agency, 0, sizeof(agency));
		memset(expireStart, 0, sizeof(expireStart));
		memset(expireEnd, 0, sizeof(expireEnd));
		memset(prCode, 0, sizeof(prCode));
		memset(nation, 0, sizeof(nation));
		memset(idVersion, 0, sizeof(idVersion));
		memset(agencyCode, 0, sizeof(agencyCode));
	};
}IDInfos;

/// \breif 人证比对结果
typedef struct FaceIdentifyResult_s
{
	char *liveImageRGB;	///<breif 现场照数据 RGB格式
	int liveImageLen;	///<breif 现场照数据长度
	int liveImageWidth;	///<breif 现场照宽度
	int liveImageHeight;///<breif 现场照高度

	char* liveFrameImage;	///<breif 现场照全图 JPG格式
	int   liveFrameLen;		///<breif 现场照全图长度

	int matchResult;		///<breif 人证比对结果 表示人证比对的几种状态  1身份证未检测到人脸 2现场照未未检测到人脸 3其他错误情况 0已获得比对分数
	float matchScore;		///<breif 人证比对得分

	float liveImageAge;		///<breif 现场照年龄
	char liveImageGender[8];///<breif 现场照性别

	FaceIdentifyResult_s()
	{
		liveImageRGB = 0;
		liveImageLen = 0;
		liveImageWidth = 0;
		liveImageHeight = 0;
		liveFrameImage = 0;
		liveFrameLen = 0;
		matchResult = FI_Res_Error;
		matchScore = 0.0;
		liveImageAge = 0;
		liveImageGender[0] = '\0';
	};

}FaceIdentifyResult;

typedef struct FlightInfo_t
{
	char flightNo[16];		//航班号	  [必须]
	char boardingNum[16];	//登机序号 [必须]
	char setId[8];			//座位号	[必须]
	char startPort[8];		//出发港	[必须]
	char terminalPort[8];	//到达港 [非必须]
	char flightDay[8];		//航班日	[必须]
	int  kindType;			//0:刷票 1:刷票放行 2：解除绑定 [必须]

	FlightInfo_t(){
		memset(flightNo, 0, sizeof(flightNo));
		memset(boardingNum, 0, sizeof(boardingNum));
		memset(setId, 0, sizeof(setId));
		memset(startPort, 0, sizeof(startPort));
		memset(terminalPort, 0, sizeof(terminalPort));
		memset(flightDay, 0, sizeof(flightDay));
		kindType = -1;
	}
}FlightInfo;

// 读卡器上线事件
typedef void (CALLBACK *IDCardOnlineCB)(void *userData);

// 读卡器下线事件
typedef void (CALLBACK *IDCardOfflineCB)(void *userData);

/* 读卡器响应回调，设置该回调函数后身份证读卡器读取数据后自动响应，返回人证比对结果,证件信息，现场照信息
** @param pIdInfos,[OUT],证件信息
** @param faceIdentifyResult[out]-人证比对结果返回
** @param userData[out]-用户信息
*/
typedef void (CALLBACK *IDCardResponseCB)(IDInfos* pIdInfos, FaceIdentifyResult* pFIResult,void *userData);

/*身份证信息请求
** @param pIdInfos 身份证信息
*/
typedef void (CALLBACK *IDCardRequstCB)(IDInfos* pIdInfos, void *userData);

/* 机票信息请求处理回调
** @param pFlightInfos 机票信息
*/
typedef void (CALLBACK *FlightInfoRequstCB)(FlightInfo* pFlightInfos, void* userData);

//异步远程人证对比回调
typedef void (CALLBACK *InputIdInfoCB)(FaceIdentifyResult* pFIResult, void *userData);

//异步人工通道回调
typedef void (CALLBACK *InputFligtInfoCB)(int result, int status, void* userData);

//异步人工放行回调
typedef void (CALLBACK *AllowByManualCB)(int result, int status, void* userData);

#endif