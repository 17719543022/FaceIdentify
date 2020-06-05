//////////////////////////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE  
/// Copyright(c)2018,中科院重庆研究院智能安全技术研究中心  （版权声明）  
/// All rights reserved.  
/// \file IDCardSDK.h 
/// \brief 人证比对SDK
///
/// \author plusone_chen 
/// \version V1.0(main.min，主版本.分版本格式)  
/// \date 2018/6/10 
//////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FACEIDENTIFYSDK_H
#define FACEIDENTIFYSDK_H

#include "ParamDefine.h"

#ifdef FACEIDENTIFYSDK_EXPORTS
	#define FCFIAPI  __declspec(dllexport)
#else
	#define FCFIAPI
#endif

#ifdef __cplusplus
extern "C"{
#endif	

	/* 功能： 初始化参数设置
	** 参数：
	**		pSDKParam ，初始化参数，[IN]
	**		pOptParam , 人脸寻优参数结构体, [IN]
	** 返回值：
	**		0， 成功；其他，错误码 
	*/
	FCFIAPI int CoreFIInit(IN const FISDKParam* pSDKParam = NULL, IN const OptimumParam* pOptParam = NULL);
	
	/* 功能： 释放对象内存
	*/
	FCFIAPI void CoreFIRelease();

	/*功能：设置身份证信息识别回调和读卡器状态回调(有读卡器设备使用)
	*/
	FCFIAPI void CoreFISetIDCardResponseCB(IDCardResponseCB idCardResponse, void* userData, IDCardOnlineCB idCardOnline = 0,IDCardOfflineCB idCardOffline = 0);

	/**功能：设置本地HTTP服务处理回调
	*/
	FCFIAPI void CoreFISetHTTPRequstCB(IDCardRequstCB idCardRequstCB, FlightInfoRequstCB flightInfoCB, void* userData);

	/*功能：设置服务器异步调用回调
	*/
	FCFIAPI void CoreFISetRPCCB(InputIdInfoCB inputIdInfoCB, InputFligtInfoCB inputFligtInfoCB, AllowByManualCB allowByManualCB, void* userData);

	/*功能： 传入证件照做人证比对[同步]
	**参数： pIdInfo ，传入的证件照片信息， [IN]
	**		faceIdentifyResult， 人证比对结果， [OUT]
	**返回值：
	**		0，成功；其他，失败
	**说明： 不使用读卡器功能
	**		配置文件config.ini ----compareMode配置为0时做本地对比,1时调用服务器对比,2先做服务器比对若失败做本地比对
	*/
	FCFIAPI int CoreFIInputIdInfo(IN const IDInfos *pIdInfo, OUT FaceIdentifyResult *faceIdentifyResult);

	/*功能： 传入证件照做人证比对[异步远程调用]
	**参数： pIdInfo ，传入的证件照片信息， [IN]
	**返回值：
	**		0，成功；其他，失败
	**说明： 不使用读卡器功能
	**		调用服务器对比 ----compareMode配置为2先做服务器比对若失败做本地比对
	*/
	FCFIAPI int CoreFIInputIdInfoAsyn(IN const IDInfos *pIdInfo);

	/*功能： 传入现场照视频流
	**参数： 
	**	frameIndex， 传入图像的帧序号， [IN]
	**  liveImage, 传入视频流图片数据RGB格式, [IN]
	**  liveImageLen, liveImageWidth, liveImageHeight， 传入视频流图片的字节长度、宽度、高度, [IN]
	**  faceNum,检测到的人脸个数, [OUT]
	**  facePos,人脸对应的位置信息,[OUT]
	** 返回值：
	**		 0，成功；其他，失败
	** 说明：
	**		外部申请faceNum数组大小不小于20
	*/
	FCFIAPI int CoreFIInputVideoFrame(IN int frameIndex, IN const char *pImgData, IN int imgLen, IN int imgWidth, IN int imgHeight, OUT int *faceNum, OUT int facePos[][5]);

	/*功能： 设置人脸检测视频源
	**参数：
	**   nVideoId, 视频源序号和初始化函数CoreFIInit设置的视频源序号对应，[IN]
	**返回值：
	**   0，成功；其他，失败
	**说明：
	**   只有一个视频能参与检测，当前设置将覆盖以前设置
	*/
	FCFIAPI int CoreFISetFaceDetectVideo(IN int nVideoId);

	/*功能： 刷机票[同步]
	**参数：
	**	pFlightInfo， 传入航班信息，[IN]
	**  pFaceImg, jpg格式人脸图片数据，[IN]
	**  nImgLen, 图片数据长度，[IN]
	**  pStatus, 服务返回结果，[OUT]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	**      该接口支持【刷票、解除绑定】 不支持【刷票放行】 
	**      解除绑定不需要 pFaceImg 
	*/
	FCFIAPI int CoreFIInputFligtInfo(IN const FlightInfo* pFlightInfo, IN const char* pFaceImg, IN int nImgLen, OUT int* pStatus);

	/*功能： 刷机票[异步]
	**参数：
	**	pFlightInfo， 传入航班信息，[IN]
	**  pFaceImg, jpg格式人脸图片数据，[IN]
	**  nImgLen, 图片数据长度，[IN]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	**      该接口支持【刷票、解除绑定】 不支持【刷票放行】 
	**      解除绑定不需要 pFaceImg 
	**
	*/
	FCFIAPI int CoreFIInputFligtInfoAsyn(IN const FlightInfo* pFlightInfo, IN const char* pFaceImg, IN int nImgLen);

	/*功能：人工放行[同步]
	**参数：
	**  pFlightInfo，传入的证件照片信息， [IN]
	**  pFrameImg， jpg格式全景照，[IN]
	**  nImglen, 全景照数据长度，[IN]
	**  flightInfo， 传入航班信息，[IN]
	**  pStatus，服务返回结果，[OUT]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	**    
	*/
	FCFIAPI int CoreFIAllowByManual(IN const FlightInfo* pFlightInfo, IN const char* pFrameImg, IN int nImglen, OUT int* pStatus, IN const IDInfos *pIdInfo = NULL);

	/*功能：人工放行[异步]
	**参数：
	**  pFlightInfo， 传入航班信息，[IN]
	**  pFrameImg， jpg格式全景照，[IN]
	**  nImglen, 全景照数据长度，[IN]
	**  pIdInfo，传入的证件照片信息， [IN]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	**    
	*/
	FCFIAPI int CoreFIAllowByManualAsyn(IN const FlightInfo* pFlightInfo, IN const char* pFrameImg, IN int nImglen, IN const IDInfos *pIdInfo = NULL);

	/*功能：初始化最优人脸缓存
	**参数：
	** pBestFace, 最优人脸缓存 [OUT]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	*/
	FCFIAPI int CoreFIInitBestFaceList(OUT OptimumFace** pBestFace);

	/*功能：释放最优人脸缓存
	**参数：
	** pBestFace, 最优人脸缓存 [OUT]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	*/
	FCFIAPI int CoreFIReleaseBestFaceList(OUT OptimumFace** pBestFace);

	/*功能：获取最优人脸
	**参数：
	** pBestFace, 最优人脸列表指针，输出最优人脸信息，事先通过@CoreFIInitBestFaceList申请内存 [OUT]
	** pFaceNum, 最优人脸数量 [OUT]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	*/
	FCFIAPI int CoreFIGetBestFace(OUT OptimumFace* pBestFace, OUT int* pFaceNum);

	/*功能：提取图片中间人脸特征
	**参数：
	** pImgData, 图片数据
	** nDataLen, 数据长度
	** pOutFeature, 特征缓存大小8*1024
	**返回值：
	**		0，成功；其他，失败
	**说明：
	*/
	FCFIAPI int CoreFIGetFeature(IN const char* pImgData, IN int nDataLen, OUT char* pOutFeature);

	/*功能： 获取内部结构版本信息
	**参数： fVersion，字符串数组，[OUT]
	**返回值：
	**		0，成功；其他，失败
	**说明：
	**	外部申请不小于468个字节,其中 人脸寻优版本:32字节，人脸特征版本:200字节，人脸匹配版本：200字节，身份证阅读版本:36字节，视频解码模块版本:,视频编码模块版本:,当前SDK版本：
	*/
	FCFIAPI int CoreFIGetVersion(char *fVersion);


#ifdef __cplusplus
}
#endif

#endif




