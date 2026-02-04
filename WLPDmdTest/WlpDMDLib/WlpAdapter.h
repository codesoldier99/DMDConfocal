#pragma once
#include "WlpAdapter.h"
#include "IDMDAdapter.h"
#include "wlp_interface.h"

#include "I2C_Addr.h"
#include "lis3dh_driver.h"

#include<algorithm>
#include<string>
#include <functional>
#include<vector>
using namespace std;


/** @brief trigger Enable */
#define WLP_TRIGGER_DISABLE			0x00000000		/**< ignore external or software trigger signal.*/
#define WLP_TRIGGER_ENABLE			0x00000001		/**< trigger is enable.*/

/** @brief enmu trigger mode */
#define WLP_TRIGGER_SINGLE_FRAME	0x00000000		/**< one trigger for one frame mode.*/
#define WLP_TRIGGER_FRAMES			0x00000002		/**< one trigger display all frames and then auto stop ,or stop by trigger.*/
#define WLP_TRIGGER_FRAMES_AND_AUTOSTOP	0x00000004	/**< display all frames  and ignore all trigger signal before it's auto stop.*/
#define WLP_TRIGGER_LOOP			0x00000006		/**< loop display all frames, at the end of the stream,first frame will be showed again.*/
#define WLP_TRIGGER_MODE_5          0x00000008 

/** @brief enmu output sync signal mode */
#define WLP_SYNC_ALL_FRAMES			0x00000000		/**< signal out after display all frames.*/
#define WLP_SYNC_SINGLE_FRAME		0x00000008		/**< signal out after display every frame.*/

#define I2C_W  0X0     
#define I2C_R  0X1     
#define I2C_DEV_ADDR_PWM_W  0X58    //0B 0101 0000
#define I2C_DEV_ADDR_PWM_R  0X59    //0B 0101 0001

#define I2C_DEV_ADDR_DDR_SPD_W  0XA4    //0B 
#define I2C_DEV_ADDR_DDR_SPD_R  0XA5    //0B  

#define I2C_DEV_ADDR_DDR_SPD_W_V6  0XA6    //0B 
#define I2C_DEV_ADDR_DDR_SPD_R_V6  0XA7    //0B  



typedef struct {
	ULONG dmdtype;			/**< device dmdtype, more infomation looking discription on WlpDevControl.*/
/**< image display time. The time unit is  nanosecond.
    * for .7 dmd: The min value of frame_margin is 13.64.
    * for .95 dmd:	The min value of frame_margin is 13.64.
*/
	ULONG frame_margin ;
/**< after image display, the black time.  The time unit is  nanosecond.
    * The min value of frame_interval is 0.
    * The default value is 0.
*/
	ULONG frame_interval ;
	ULONG gray;				/**< device gray, more infomation looking discription on WlpDevControl.*/
	ULONG trigger_setting;  /**< device trigger setting, more infomation looking discription on WlpDevControl.*/
}DEVINIT2 , *PDEVINIT2 ;


/**< IIC operation parameter.*/   
typedef struct
{
	UCHAR  W_or_R  ;      /**< indicator  */
	ULONG  NumOfBytes;   
	ULONG  DevAddr ;	  /**< IIC slave device address  */
	ULONG  RegAddr ;      /**< Register address in the device  */
	UCHAR  W_Data[4];  /**< to contain the data write to IIC slave device */
	UCHAR  R_Data[4];  /**< to contain the data read from IIC slave device */
} I2C_OPER , *pI2C_OPER ;


typedef struct {
	ULONG TrigeModeX ;
	ULONG m_FrameMarginForMode1   ;
	ULONG m_FrameIntervalForMode1 ;
}SophistcateTrigger ;

enum GrayBit
{
	GrayBit_16 =   0XFFFF , //0B 1111 1111 1111 1111  
	GrayBit_15 =   0XFFFE , //0B 1111 1111 1111 1110  
	GrayBit_14 =   0XFFFC , //0B 1111 1111 1111 1100  
	GrayBit_13 =   0XFFF8 , //0B 1111 1111 1111 1000 
	GrayBit_12 =   0XFFF0 , //0B 1111 1111 1111 0000  
	GrayBit_11 =   0XFFE0 , //0B 1111 1111 1110 0000  
	GrayBit_10 =   0XFFC0 , //0B 1111 1111 1100 0000  
	GrayBit_9  =   0XFF80 , //0B 1111 1111 1000 0000 
	GrayBit_8  =   0XFF , //0B 1111 1111 
	GrayBit_7  =   0XFE , //0B 1111 1110 
	GrayBit_6  =   0XFC , //0B 1111 1100 
	GrayBit_5  =   0XF8 , //0B 1111 1000 
	GrayBit_4  =   0XF0 , //0B 1111 0000 
	GrayBit_3  =   0XE0 , //0B 1110 0000 
	GrayBit_2  =   0XC0 , //0B 1100 0000 
	GrayBit_1  =   0X80 , //0B 1000 0000 
};

class CWlpAdapter : public IDMDAdapter
{
public:
	CWlpAdapter(void);
	virtual ~CWlpAdapter(void);
	PUCHAR pLargeImageBuff;//下载图像使用的Buff
private:
	WLP_HDEVICE device;//DMD驱动，开关用
	ULONG  RAMsize;//获取的总共RAM的大小  
	BOOL m_bDevopened;//指示是否开启

	DEVINIT2 DevInit;//驱动器里面的内容
	ULONG FrameActiveTime;//明亮时间
	
	ULONG  m_Alldonwloadfilecount;//下载的图像个数
	int FWdownloaded; //指示下载的图像数目
	SophistcateTrigger ThisTrigger;//触发模式
	vector<SophistcateTrigger>  TriggerQueue;//触发模式列表	
	int DMDtype;//DMD型号
	

	//Step1 开启使用的函数
	bool Read_DDR_SPD();//Caculating the DDR RAM size through read the Memory modules'SPD 
	LONG I2CCtrl( I2C_OPER I2CWriteOper ) ;
	LONG I2CReadData(  PULONG RData);
	//Step2 下载图片之前初始化使用的函数
	bool InitDev(UINT m_FrameMargin,UINT m_FrameInterval,UINT gray,int trigeMode,int trigeSync); 
	LONG  WlpDevInit2( WLP_HDEVICE hDevice , PDEVINIT2 pDevInit ) ;
	bool Wlp_DevNewFrequencySet(ULONG gray,ULONG frame_margin,ULONG frame_interval);
	//Step3 下载函数
	ULONG SingleDownload(ULONG ThisTimeStartAddr ,ULONG SingleDownSize , 
		ULONG DownloadTimes,ULONG ThisTimeConvertFileCount,ULONG SingleFrameLen);
	//Step Null 更改触发模式使用的函数
	ULONG TrigerSetALL(int trigeMode,int trigeSync); 

	//trigeMode:WLP_TRIGGER_SINGLE_FRAME单帧触发
			// WLP_TRIGGER_FRAMES 循环显示，用triger控制
			// WLP_TRIGGER_FRAMES_AND_AUTOSTOP 只循环一次
			// WLP_TRIGGER_LOOP 循环显示，triger 完用 Stop进行停止  使用这个！！
	//trigeSync:外部输出一般用 WLP_SYNC_SINGLE_FRAME，即每帧都有触发
public:
	//Step1
	bool OpenDev(void);
	void CloseDev(void);
	//Step2
	bool InitForDownload(UINT m_FrameMargin, UINT m_FrameInterval,UINT gray,int trigeMode, int trigeSync);
	bool TriggerExtSet(ULONG delay,bool polay,ULONG plusewidth,ULONG ImageNum);	
	//Step3 下载
	void GetPicSize(int* iWidth,int* iHeight);
	bool DownloadPic(unsigned char* pPicData,int iPicCount);
	//Step4
	int Start();
	void Stop(void);
	//Step Null
	LONG SetTrigerMode1(int trigeMode,int trigeSync);
	LONG SetTrigerMode2(int trigeMode,int trigeSync);
	
	bool Trigger();	

	char* UsePixel2bin(char* pSrc, int width,int height, unsigned char bitmask, char* pOut);
	char* UseBin2pixel(char* pSrc, int width,int height, unsigned char bitmask, char* pOut);
};

enum ddr3_reg_map {
	/* 0 */
	DDR3_SPD_REG_SIZE_CRC,
	DDR3_SPD_REG_REVISION,
	DDR3_SPD_REG_DEVICE_TYPE,
	DDR3_SPD_REG_MODULE_TYPE,
	DDR3_SPD_REG_DENSITY_BANKS,
	DDR3_SPD_REG_ADDRESSING,
	DDR3_SPD_REG_VOLTAGE,
	DDR3_SPD_REG_MODULE_ORG,
	DDR3_SPD_REG_MODULE_BUS_WIDTH,
	DDR3_SPD_REG_FTB_DIVIDEND_DIVSOR,
	/* 10 */
	DDR3_SPD_REG_MTB_DIVIDEND,
	DDR3_SPD_REG_MTB_DIVISOR,
	DDR3_SPD_REG_TCK_MIN,
	DDR3_SPD_REG_RESERVED,
	DDR3_SPD_REG_CAS_LAT_LSB,
	DDR3_SPD_REG_CAS_LAT_MSB,
	DDR3_SPD_REG_TAA_MIN,
	DDR3_SPD_REG_TWR_MIN,
	DDR3_SPD_REG_TRCD_MIN,
	DDR3_SPD_REG_TRRD_MIN,
	/* 20 */
	DDR3_SPD_REG_TRP_MIN,
	DDR3_SPD_REG_TRAS_TRC_UP_NIBBLE,
	DDR3_SPD_REG_TRAS_MIN_LSB,
	DDR3_SPD_REG_TRC_MIN_LSB,
	DDR3_SPD_REG_TRFC_MIN_LSB,
	DDR3_SPD_REG_TRFC_MIN_MSB,
	DDR3_SPD_REG_TWTR_MIN,
	DDR3_SPD_REG_TRTP_MIN,
	DDR3_SPD_REG_TFAW_UP_NIBBLE,
	DDR3_SPD_REG_TFAW_MIN,
	/* 30 */
	DDR3_SPD_REG_OPT_FEATURES,
	DDR3_SPD_REG_THERM_REFRESH_OPT,
	DDR3_SPD_REG_MODULE_THERM_SENSOR,
	DDR3_SPD_REG_SDRAM_DEVICE_TYPE,
	DDR3_SPD_REG_GENERAL_SECTION_START,
	DDR3_SPD_REG_GENERAL_SECTION_END = 59,
	/* 60 */
	DDR3_SPD_REG_MODULE_SPECIFIC_SECTION_START,
	DDR3_SPD_REG_MODULE_SPECIFIC_SECTION_END = 116,
	/* 117 */
	DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_LSB,
	DDR3_SPD_REG_MODULE_MANUF_JEDEC_ID_MSB,
	DDR3_SPD_REG_MODULE_MANUF_LOC,
	/* 120 */
	DDR3_SPD_REG_MODULE_MANUF_DATE_YEAR,
	DDR3_SPD_REG_MODULE_MANUF_DATE_WEEK,
	DDR3_SPD_REG_MODULE_MANUF_SERIAL_0,
	DDR3_SPD_REG_MODULE_MANUF_SERIAL_1,
	DDR3_SPD_REG_MODULE_MANUF_SERIAL_2,
	DDR3_SPD_REG_MODULE_MANUF_SERIAL_3,
	DDR3_SPD_REG_CRC_0,
	DDR3_SPD_REG_CRC_1,
	DDR3_SPD_REG_MODULE_PART_NUM_0,
	DDR3_SPD_REG_MODULE_PART_NUM_START = DDR3_SPD_REG_MODULE_PART_NUM_0,
	DDR3_SPD_REG_MODULE_PART_NUM_1,
	/* 130 */
	DDR3_SPD_REG_MODULE_PART_NUM_2,
	DDR3_SPD_REG_MODULE_PART_NUM_3,
	DDR3_SPD_REG_MODULE_PART_NUM_4,
	DDR3_SPD_REG_MODULE_PART_NUM_5,
	DDR3_SPD_REG_MODULE_PART_NUM_6,
	DDR3_SPD_REG_MODULE_PART_NUM_7,
	DDR3_SPD_REG_MODULE_PART_NUM_8,
	DDR3_SPD_REG_MODULE_PART_NUM_9,
	DDR3_SPD_REG_MODULE_PART_NUM_10,
	DDR3_SPD_REG_MODULE_PART_NUM_11,
	/* 140 */
	DDR3_SPD_REG_MODULE_PART_NUM_12,
	DDR3_SPD_REG_MODULE_PART_NUM_13,
	DDR3_SPD_REG_MODULE_PART_NUM_14,
	DDR3_SPD_REG_MODULE_PART_NUM_15,
	DDR3_SPD_REG_MODULE_PART_NUM_16,
	DDR3_SPD_REG_MODULE_PART_NUM_17,
	DDR3_SPD_REG_MODULE_PART_NUM_END = DDR3_SPD_REG_MODULE_PART_NUM_17,
	DDR3_SPD_REG_MODULE_REVISION_0,
	DDR3_SPD_REG_MODULE_REVISION_1,
	DDR3_SPD_REG_DRAM_MANUF_JEDEC_ID_LSB,
	DDR3_SPD_REG_DRAM_MANUF_JEDEC_ID_MSB,
	DDR3_SPD_REG_MANUF_SPECIFIC_DATA_START,
	DDR3_SPD_REG_MANUF_SPECIFIC_DATA_END = 175,
	/* 176 */
	DDR3_SPD_REG_CUSTOMER_USE_START,
	DDR3_SPD_REG_CUSTOMER_USE_END = 255,
};

