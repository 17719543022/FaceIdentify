#ifndef DETECT_OPTIMUM_DEF_H
#define DETECT_OPTIMUM_DEF_H

typedef struct FacePosition_s
{
	int			id;					/*人脸ID*/
	int			left;			    /* 左边界值 */
	int			top;				/* 上边界值 */
	int			right;				/* 右边界值 */
	int			bottom;				/* 下边界值 */
}FacePosition;

/*整个视频帧图像信息*/
typedef struct FrameImage_s
{
	int			frmIndex;			/*帧号信息*/
	int			imgLen;				/*整帧图像（JPEG格式）的长度*/
	char*		imgData;			/*整帧图像数据（JPEG格式），通过解码JPEG数据可以得到图像的宽高等信息*/
	int			memLen;				/*图像数据区申请的总内存大小*/
}FrameImage;

/*人脸寻优结果结构体*/
typedef struct OptimumFace_s
{
	FacePosition	facePst;		/*人脸在剪切后图像中的位置信息*/

	float			quality;		/*人脸质量*/
	int				width;			/*剪切后图像的宽度*/
	int				height;			/*剪切后图像的高度*/
	int				imgLen;			/*剪切后图像所占内存大小*/
	char*			faceData;		/*剪切后图像内存数据*/
	int				featureLen;		/*人脸特征长度，不提特征时为0*/
	char*			featureData;	/*人脸特征，不提特征时为NULL*/
	long			curTime;		/*人脸当前时间，为当前日期的毫秒数计时*/

	FrameImage		frmImg;			/*人脸所在的整张视频帧图片的数据信息*/
	FacePosition	frmFacePst;		/*人脸在全景图像中的位置信息*/

	float			reserved[6];	/*保留，用于输出一些中间数据*/

	/*默认构造函数*/
	OptimumFace_s()
	{
		facePst.id = -1;
		facePst.left = -1;
		facePst.top = -1;
		facePst.right = -1;
		facePst.bottom = -1;
		quality = 0;
		width = 0;
		height = 0;
		imgLen = 0;
		faceData = 0;
		featureLen = 0;
		featureData = 0;
		curTime = 0;
		frmImg.frmIndex = -1;
		frmImg.imgLen = 0;
		frmImg.imgData = 0;
		frmImg.memLen = 0;
		frmFacePst.id = -1;
		frmFacePst.left = -1;
		frmFacePst.top = -1;
		frmFacePst.right = -1;
		frmFacePst.bottom= -1;
	};
}OptimumFace;

enum TrigType
{
	TrigByTime = 0,
	TrigByEvent
};

enum PicType
{
	PicRGB = 0,
	PicJPEG
};

enum DetectModel
{
	Model_Normal = 0,
	Model_Light,
	Model_Tiny
};

/*关于寻优的参数结构体*/
typedef struct OptimumParam_s
{
	/*最大跟踪寻优的人脸数目*/
	int nMaxTrackFaceNum;

	/*人脸寻优的时间跨度，为寻优的滑动窗口时间长度，单位 ms*/
	int nOptTimeLength;

	/*定时自动触发寻优事件的间隔时间，触发类型为定时触发时有效，单位 ms*/
	int nTrigTimeInterval;

	/*进行检测的帧间隔，=1 每帧检测，>1 跳帧检测*/
	int nDetFrmInterval;

	/*检测最大帧间隔，当连续时间内无人脸，为CPU消耗，降低检测频率，用最大帧间隔代替普通帧间隔*/
	int nMaxDetInterval;

	/*质量评估相对于检测的帧倍数，代表每多少次检测才进行一次质量评估，
	**即质量评估帧间隔为：nDetFrmInterval*nEvalFrmFactor，质量评估耗时，不是所有检测都进行评估
	*/
	int nEvalFrmFactor;

	/*输出的图片类型，OptimumFace结构中的faceData类型，网络传输时建议JPEG，本地调用时建议RGB*/
	PicType nPicType;

	/*触发类型，事件触发（外部调用，例如刷卡）或者定时触发（回调函数自动触发）*/
	TrigType nTriggerType;

	/*检测时图像的缩小比例，范围0-1，主要为了避免时间消耗，但输出的人脸坐标为原始图片坐标*/
	float fDetRatio;

	/*人脸质量阈值，低于此阈值的人脸不参与寻优*/
	float fQualityLimit;

	/*关键点置信度阈值，低于此阈值不参与寻优，置信度越低角度估计越不准确，一般人脸转角较大是会导致置信度低*/
	float fKptCfdLimit;

	/*人脸X轴旋转角度阈值（人脸俯仰角，抬头低头），超过此角度的不参与寻优*/
	float fAngleXLimit;

	/*人脸Y轴旋转角度阈值（人脸左右转角，头顶不动转头），超过此角度的不参与寻优*/
	float fAngleYLimit;

	/*人脸Z轴旋转角度阈值（人脸水平偏转角，头往肩部下压），超过此角度的不参与寻优*/
	float fAngleZLimit;

	/*默认构造函数*/
	OptimumParam_s()
	{
        nMaxTrackFaceNum = 10;
		nOptTimeLength = 3000;
		nTrigTimeInterval = 1000;
		nDetFrmInterval = 1;
		nMaxDetInterval = 4;
		nEvalFrmFactor = 2;
		nPicType = PicJPEG;
		nTriggerType = TrigByTime;
		fDetRatio = 0.5f;
		fQualityLimit = 0.1f;
		fKptCfdLimit = 0.6f;
		fAngleXLimit = 40.0f;
		fAngleYLimit = 40.0f;
		fAngleZLimit = 40.0f;
	};
}OptimumParam;


//函数返回错误码
#define		DO_SUCC					0	//成功
#define		DO_CREATE_DET_ERR		101	//创建检测通道出错
#define		DO_CREATE_FEA_ERR		102	//创建特征通道出错
#define		DO_DET_NOT_CREATE		103	//检测通道未创建
#define		DO_FEA_NOT_CREATE		104	//特征通道未创建
#define		DO_PARAM_SET_ERR		105	//参数设置错误
#define		DO_DET_FAILURE			106	//人脸检测失败
#define		DO_KPT_FAILURE			107	//关键点计算失败
#define		DO_FEA_FAILURE			108	//特征提取失败
#define		DO_OTHER_ERR			109	//其他错误，包括去初始化失败，释放内存错误等

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

//人脸寻优回调函数，寻到到最优人脸时调用.
typedef void (STDCALL *FaceOptimumCallBack)(int mCamId, OptimumFace* pFaceList, int faceNum, void *userData);

#endif 
