#ifndef  _SYPROTOCOL_H_
#define  _SYPROTOCOL_H_
 
#define HANDLE int


//////////////////////////////////////
#define ZAZ_OK                0x00
#define ZAZ_COMM_ERR          0x01
#define ZAZ_NO_FINGER         0x02
#define ZAZ_GET_IMG_ERR       0x03
#define ZAZ_FP_TOO_DRY        0x04
#define ZAZ_FP_TOO_WET        0x05
#define ZAZ_FP_DISORDER       0x06
#define ZAZ_LITTLE_FEATURE    0x07
#define ZAZ_NOT_MATCH         0x08
#define ZAZ_NOT_SEARCHED      0x09
#define ZAZ_MERGE_ERR         0x0a
#define ZAZ_ADDRESS_OVER      0x0b
#define ZAZ_READ_ERR          0x0c
#define ZAZ_UP_TEMP_ERR       0x0d
#define ZAZ_RECV_ERR          0x0e
#define ZAZ_UP_IMG_ERR        0x0f
#define ZAZ_DEL_TEMP_ERR      0x10
#define ZAZ_CLEAR_TEMP_ERR    0x11
#define ZAZ_SLEEP_ERR         0x12
#define ZAZ_INVALID_PASSWORD  0x13
#define ZAZ_RESET_ERR         0x14
#define ZAZ_INVALID_IMAGE     0x15
#define ZAZ_HANGOVER_UNREMOVE 0X17


/////////////////////////////////////////////
#define CHAR_BUFFER_A          0x01
#define CHAR_BUFFER_B          0x02
#define MODEL_BUFFER           0x03

/////////////////
#define COM1                   0x01
#define COM2                   0x02
#define COM3                   0x03

/////////////////////////////////////////
#define BAUD_RATE_9600         0x00
#define BAUD_RATE_19200        0x01
#define BAUD_RATE_38400        0x02
#define BAUD_RATE_57600        0x03   //default
#define BAUD_RATE_115200       0x04

 
typedef unsigned char BYTE;

/////////////////////////////////////////
 



//参数fptype
#define fp_602 1 
#define fp_606 2  
#define fp_608 3
//参数path
#define path_proc 0 //表示usb路径：/proc/bus/usb
#define path_dev  1 //表示usb路径：/dev/bus/usb

//
#ifdef __cplusplus
extern "C" {
#endif

//1	设置设备工作模式  //255开日志
int ZAZ_MODE(int mode);
//2	设置设备驱动方式
int ZAZ_SETPATH(int fptype,int path);	
//3	设置通讯时间间隔
void ZAZ_Delaytime(int times);
//4	打开设备 
int ZAZOpenDeviceEx(HANDLE *hHandle,int nDeviceType,const char * nPortNum,int nPortPara,int nPackageSize);
//5 	关闭设备
int ZAZCloseDeviceEx(HANDLE hHandle);
//6	校验密码
int ZAZVfyPwd(HANDLE hHandle,int nAddr,unsigned char* pPassword);
//7	获取图像	 
int ZAZGetImage(HANDLE hHandle,int nAddr);
//8	生成特征
int ZAZGenChar(HANDLE hHandle,int nAddr,int iBufferID);
//9	设备内部比对指纹(1:1)
int ZAZMatch(HANDLE hHandle,int nAddr,int* iScore);
//10 设备内部搜索指纹(1:N)
int ZAZSearch(HANDLE hHandle,int nAddr,int iBufferID, int iStartPage, int iPageNum, int *iMbAddress,int *iscore);
//11	合成模板	
int ZAZRegModule(HANDLE hHandle,int nAddr);
//12 存储指纹模板
int ZAZStoreChar(HANDLE hHandle,int nAddr,int iBufferID, int iPageID);
//13 删除指纹
int ZAZDelChar(HANDLE hHandle,int nAddr,int iStartPageID,int nDelPageNum);
//14 清空指纹
int ZAZEmpty(HANDLE hHandle,int nAddr);
//15 读取系统参数
int ZAZReadParTable(HANDLE hHandle,int nAddr,unsigned char* pParTable);
//16 获取已注册指纹数量
int ZAZTemplateNum(HANDLE hHandle,int nAddr,int *iMbNum);
//17 获取随机数
int ZAZGetRandomData(HANDLE hHandle,int nAddr,unsigned char* pRandom);
//18 读取指纹列表
int 	ZAZReadIndexTable(HANDLE hHandle,int nAddr,int nPage,unsigned char* UserContent);
//19 灯声控制接口
int ZAZDoUserDefine(HANDLE hHandle,int nAddr,int GPIO,int STATE);
//20	 设置特征长度
int ZAZSetCharLen( int nLen);
int  ZAZSetImgLen( int w,int h);
//21	 获取特征长度
int 	ZAZGetCharLen(int *pnLen);
//22 加载指纹模板到缓冲区iBuffer
int ZAZLoadChar(HANDLE hHandle,int nAddr,int iBufferID,int iPageID);
//23	 上传指纹特征
int ZAZUpChar(HANDLE hHandle,int nAddr,int iBufferID, unsigned char* pTemplet, int* iTempletLength);
//24	 下传指纹特征
int ZAZDownChar(HANDLE hHandle,int nAddr,int iBufferID, unsigned char* pTemplet, int iTempletLength);
//25 上传图像
int ZAZUpImage(HANDLE hHandle,int nAddr,unsigned char* pImageData, int* iImageLength);
//26 下传图像
int ZAZDownImage(HANDLE hHandle,int nAddr,unsigned char *pImageData, int iLength);
//27 保存Bmp图像
int ZAZImgData2BMP(unsigned char* pImgData,const char* pImageFile);
//28 读取Bmp图像
int ZAZGetImgDataFromBMP(const char *pImageFile,unsigned char *pImageData,int *pnImageLen);
//29 读取记事本
int 	ZAZReadInfo(HANDLE hHandle,int nAddr,int nPage,unsigned char* UserContent);
//30 写入记事本
int 	ZAZWriteInfo(HANDLE hHandle,int nAddr,int nPage,unsigned char* UserContent); 
//31 设置密码
int  ZAZSetPwd(HANDLE hHandle,int nAddr,unsigned char* pPassword);
//32 读取参数页
int ZAZReadInfPage(HANDLE hHandle,int nAddr, unsigned char* pInf);
//33 获取图像质量
int  ZAZGetImgQuality(int width, int height, unsigned char*	p_pbImage );
//34  RAW TO BMP 
int  ZAZRaw2BMP(unsigned char* prawData,int w,int h,unsigned char* pbmpData);

void ZAZSetlog(int val);
int ZAZAutoStore(int *fingerid);
int ZAZGetFpList(int  imbnum,int *fplist);

int ZAZSetBaud(HANDLE hHandle,int nAddr,int nBaudNum);
int ZAZSetSecurLevel(HANDLE hHandle,int nAddr,int nLevel);
int ZAZSetPacketSize(HANDLE hHandle,int nAddr,int nSize);
int ZAZUpChar2File(HANDLE hHandle,int nAddr,int iBufferID, const char* pFileName);
int ZAZDownCharFromFile(HANDLE hHandle,int nAddr,int iBufferID, const char* pFileName);
int ZAZSetChipAddr(HANDLE hHandle,int nAddr,unsigned char* pChipAddr);
int ZAZBurnCode(HANDLE hHandle,int nAddr,int nType,unsigned char *pImageData, int iLength);
int ZAZIdentify(HANDLE hHandle,int nAddr,int *iMbAddress);
int ZAZEnroll(HANDLE hHandle,int nAddr,int* nID);
void Delay(int nTimes);


int FingerImgSorce(unsigned char  * pImage);
char*    ZAZErr2Str(int nErrCode);
#ifdef __cplusplus
}
#endif

#define DEVICE_USB		0
#define DEVICE_COM		1
#define DEVICE_UDisk	2


extern int IMAGE_X;  
extern int IMAGE_Y; 



#endif

