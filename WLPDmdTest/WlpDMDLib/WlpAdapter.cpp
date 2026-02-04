#include "StdAfx.h"
#include "WlpAdapter.h"

#ifdef _WIN64

CWlpAdapter::CWlpAdapter(void)
{
	device=0;
	m_bDevopened= false ;
	m_Alldonwloadfilecount = 0 ;
	FrameActiveTime = 0 ; 
	RAMsize = 0 ;  
	FWdownloaded = 0 ;
	pLargeImageBuff=0;
}

CWlpAdapter::~CWlpAdapter(void)
{
	if(device!=0)
		CloseDev() ;
}

bool CWlpAdapter::OpenDev()
{
	//Step 1.Open and GetDMDType
	LONG ret ;
	if ( m_bDevopened)
		return true ;
	bool  bFWBeDown=false  ;
	ret  = WlpDevAlloc( 0 , &device ,  bFWBeDown ) ; 
	if ( ret == WLP_SUCCESS )
	{
		if (bFWBeDown == false )
		{
			TRACE("\n  Start download Firmware :");
			PULONGLONG deviceAddr = NULL ;
			deviceAddr = (PULONGLONG)device ;
			if (WlpDownloadFirmWare(deviceAddr,0))
				TRACE("\n  Download Firmware success.");
			else
			{
				TRACE("\n  Download Firmware fail." ) ;
				return false ;
			}
			if (DevReAlloc(deviceAddr))
			{
				TRACE("\n   ReLoad WLP device success." )  ;
				m_bDevopened = true ; 
			}
			else
			{
				TRACE("\n  ReLoad WLP device fail.");
				return false;
			}
		}
		else
		{	
			m_bDevopened = true ; 
			TRACE("\n  Load WLP device success.");	
		}
	}
	else
	{
		TRACE("\n  Load device fail."); 
		return false ;
	}
	/***************************************************/
	//Get FPGA version 
	ULONG FpgaVer = 0 ;
	ret = WlpDevInquire(device,WLP_DEV_VERSION,&FpgaVer) ;
	if (ret == WLP_SUCCESS)
		TRACE("\n  Get FPGA version:  %x" , FpgaVer );	
	else
	{
		TRACE("\n  Get FPGA version fail.");
		goto FAIL ;
		return false ;
	}
	ret = WlpDevInquire( device , WLP_DEV_DMD_TYPE , &DevInit.dmdtype  );
	if (ret != WLP_SUCCESS)
	{
		TRACE("\n  Get DMD type fail.");
		goto FAIL ;
		return  false ;
	}
	switch( DevInit.dmdtype  )
	{
		case WLP_DMDTYPE_XGA_055A:	
			DMDtype = WLP_DMDTYPE_XGA_055A ;
			TRACE("\n  Get DMD type : .55 " );	
			break;
		case WLP_DMDTYPE_XGA_07A:	
			DMDtype = WLP_DMDTYPE_XGA_07A ;
			TRACE("\n  Get DMD type : .7" );
			break;
		case WLP_DMDTYPE_1080P_095A:	
			DMDtype = WLP_DMDTYPE_1080P_095A ;
			TRACE("\n  Get DMD type : .95" );		
			break;
		default:
			TRACE("\n Warning! Can't get DMD type, please check if it attached to the board." );			
			break ;
	}
	if (!Read_DDR_SPD())
		return false ;
	//Step 2.reset device
	printf("\n  Reset device");	
	ULONG	param2 = 0;
	ret =  WlpDevControl(device, WLP_DEV_RESET, &param2) ;
	if (ret == WLP_SUCCESS)
		TRACE("\n	RESET DEV ----- success.");	
	else
	{
		TRACE("\n	RESET DEV ----- fail.");
		return false;
	}
	//wait for reseting device to finish.
	Sleep(100);
	//Step 3.reset dmd
	TRACE("\n  Reset dmd ");
	param2 = 0;
	ret =  WlpDevControl(device, WLP_DEV_DMD_RESET, &param2) ;
	if (ret == WLP_SUCCESS)
	{		
		FrameActiveTime = param2 ;
		TRACE("\n	RESET DMD----- success.");
	}
	else
	{
		TRACE("\n	RESET DMD----- fail.");
		return false;
	}
	//wait for reseting dmd to finish.
	Sleep(100);
	//Step 4. Read Frame Active time
	param2 = 0;
	ret =  WlpDevControl(device, WLP_DEV_GET_FRAME_MARGIN , &param2 ) ;
	if (ret == WLP_SUCCESS)
		FrameActiveTime = param2 ;
	else
		return  false ;
	TRACE("\n   WLP_DEV_GET_FRAME_MARGIN:  %d \n " , FrameActiveTime );
	TRACE(" \n  *****************************************");
	return  true ;
FAIL:
	WlpDevFree(device);
	return  false ;
}

bool CWlpAdapter::Read_DDR_SPD()
{
	if ( m_bDevopened == false )
	{
		TRACE("\n Please open the device first . ");
		return  false ;
	}
	UCHAR SPDinfo[256];
	memset( SPDinfo , 0 , sizeof(UCHAR ) ) ;
	for ( int i = 0 ; i <18 ; i++ )
	{
		I2C_OPER DDR_SPD ;
		memset( &DDR_SPD, 0 , sizeof(I2C_OPER ) );
		DDR_SPD.W_or_R     =  I2C_R ; 
		DDR_SPD.NumOfBytes =  1 ;
		DDR_SPD.RegAddr    =  i ;   // 
		if ( DMDtype == WLP_DMDTYPE_XGA_07A )
		{
			DDR_SPD.DevAddr    =  I2C_DEV_ADDR_DDR_SPD_R ;
		}
		else if (  DMDtype == WLP_DMDTYPE_1080P_095A )
		{
			DDR_SPD.DevAddr    =  I2C_DEV_ADDR_DDR_SPD_R_V6 ;
		}
		else
		{
			DDR_SPD.DevAddr    =  I2C_DEV_ADDR_DDR_SPD_R ;	
		}
		*PULONG(DDR_SPD.R_Data) = (ULONG)0 ;	
		I2CCtrl( DDR_SPD ) ;
		I2CReadData	( PULONG(DDR_SPD.R_Data) ) ; 
		SPDinfo[i] = DDR_SPD.R_Data[0] ;
	}
	// Caculate the MEM size here:	
	RAMsizeGB = 0 ;
	RAMsizeMB  = 0 ;
	// DDR2 
	if ( DMDtype == WLP_DMDTYPE_XGA_07A )
	{
		int index2 = 0 ;
		index2  =  	  ( SPDinfo[3] ) + ( SPDinfo[4] ) +   // ( SPDinfo[5] & 0X03 ) can only use one rank 
			+  (int)( log( double(SPDinfo[6] ) )/ log( (double)2) )  \
			+  (int)( log( double(SPDinfo[17])) / log( (double)2) )  \
			- 30 ;
		RAMsizeGB =  (float)( pow( (long)2 , (double)(index2) ) ) / (float)8 ;   //GB
	}
	// DDR3 
	if ( DMDtype == WLP_DMDTYPE_1080P_095A )
	{
		long size;
		/* calculate the total size in MB */
		size = 256 << (SPDinfo[DDR3_SPD_REG_DENSITY_BANKS] & 0xf);
		size >>= 3; /* in terms of bytes instead of bits. */
		size *= 8 << (SPDinfo[DDR3_SPD_REG_MODULE_BUS_WIDTH] & 0x7);
		size /= 4 << (SPDinfo[DDR3_SPD_REG_MODULE_ORG] & 0x7);
		size *= 1 + ((SPDinfo[DDR3_SPD_REG_MODULE_ORG] >> 3) & 0x7);
		RAMsizeGB = (float)(size / 1024);
	}
	/****************************************************************************/
	RAMsizeMB =  UINT( RAMsizeGB*(float)1024 ) ;
	TRACE( " \n  RAM size :  %f GB  = %d MB \n " , RAMsizeGB , RAMsizeMB ) ;
	RAMsize = (ULONG)( RAMsizeGB * 1024*1024*1024 ) ;  //Byte
	if ( RAMsize == 0  )
	{
		TRACE( " \n  Can't find Memory on the board ! \n "  ) ;
		return false ;
	}
	return true ;
}

LONG CWlpAdapter::I2CCtrl( I2C_OPER I2COper )
{
	int ret ;
	ULONG Ctrl = I2COper.W_or_R +
		I2COper.NumOfBytes * 256  +  
		I2COper.RegAddr    * 256 * 256 +  
		I2COper.DevAddr    * 256 * 256 * 256  ;
	ret =  WlpDevControl( device , WLP_DEV_I2C_CTRL , &Ctrl ) ;
	if( ret == WLP_SUCCESS )
	{
		TRACE(" \n I2C write OK .");
		return true ; 
	}
	else if( ret == WLP_I2C_BUSY ) 
	{
		TRACE("  \n I2C Bus busy . ");
		return false ; 
	}
	else if( ret == WLP_I2C_FAIL )
	{
		TRACE("  \n I2C operate failed.  Perhaps i2c chip does not exist or the chip address is not right." ) ;
		return false ; 
	}
	return true ; 
}

LONG CWlpAdapter::I2CReadData( PULONG RData )
{
	int ret ;
	ret =  WlpDevControl( device , WLP_DEV_I2C_READDATA ,  RData ) ;
	if( ret == WLP_SUCCESS )
		TRACE("  \n I2C Read OK .");
	else if( ret == WLP_I2C_BUSY ) 
		TRACE("\n I2C Bus busy . ");
	else if( ret == WLP_I2C_FAIL )
		TRACE("\n I2C operate failed.  Perhaps i2c chip does not exist or the chip address is not right." ) ;
	return ret ;
}

void CWlpAdapter::CloseDev()
{
	WLP_HDEVICE t = device ;
	device = 0;
	if(t)	
		WlpDevFree(t) ;
}

LONG CWlpAdapter::SetTrigerMode1(int trigeMode ,int trigeSync)
{
	if ( m_bDevopened == FALSE )
	{
		TRACE("\n Please open the device first . ");
		return WLP_ERR ;
	}
	switch( trigeMode )
	{
		case WLP_TRIGGER_SINGLE_FRAME :
			TRACE("\n Input Trigger set:  SINGLE_FRAME "  );
			break;
		case WLP_TRIGGER_FRAMES :
			TRACE("\n Input Trigger set:  FRAME "  );
			break;
		case WLP_TRIGGER_FRAMES_AND_AUTOSTOP :
			TRACE("\n Input Trigger set:  FRAME and AUTOSTOP "  );
			break;
		case WLP_TRIGGER_LOOP :
			TRACE("\n Input Trigger set:  LOOP "  );
			break;
	}
	/********************************/
	//Patch the FPGA bug .
	memset( &ThisTrigger , 0X00 , sizeof( SophistcateTrigger ) ) ;
	ThisTrigger.TrigeModeX    = trigeMode ;
	if ( trigeMode == WLP_TRIGGER_SINGLE_FRAME )
	{
		ThisTrigger.m_FrameMarginForMode1    =  DevInit.frame_margin ;
		ThisTrigger.m_FrameIntervalForMode1  =  DevInit.frame_interval ;	 
	}
	TriggerQueue.push_back( ThisTrigger ) ;
	int temp_trigger_setting = 0 ;
	temp_trigger_setting = TrigerSetALL(  trigeMode , trigeSync ); 
	int ret = 0 ;
	ret = WlpDevControl(device , WLP_DEV_TRIGGER_SETTING, &temp_trigger_setting);
	return ret ; 
}

LONG CWlpAdapter::SetTrigerMode2(int trigeMode ,int trigeSync)
{
	// TODO: Add your control notification handler code here
	if ( m_bDevopened == FALSE )
	{
		TRACE("\n Please open the device first . ");
		return WLP_ERR ;
	}
	switch( trigeSync )
	{
		case WLP_SYNC_ALL_FRAMES :
			TRACE("\n Output Trigger set  ALL_FRAMES  "  );
			break;
		case WLP_SYNC_SINGLE_FRAME :
			TRACE("\n Output Trigger set  SINGLE_FRAME "  );
			break;
		default:
			break ;
	}
	int temp_trigger_setting = 0 ;
	temp_trigger_setting = TrigerSetALL(trigeMode,trigeSync); 
	int ret = 0 ;
	ret = WlpDevControl(device , WLP_DEV_TRIGGER_SETTING, &temp_trigger_setting);
	return ret ; 
}

ULONG CWlpAdapter::TrigerSetALL(int trigeMode , int trigeSync)
{
	int temp_trigger_setting = 0 ;
	temp_trigger_setting = (INT) ( (INT)1 | trigeMode |trigeSync );
	return temp_trigger_setting ;
}

bool  CWlpAdapter::InitForDownload(UINT m_FrameMargin,UINT m_FrameInterval,UINT gray,int trigeMode,int trigeSync) 
{
	if ( m_bDevopened == FALSE )
	{
		TRACE("\n  Please open the device first . ") ;
		return false ;
	}
	if ( RAMsize == 0  )
	{
		TRACE(" \n  Can't find Memory on the board, wouldn't download image !") ;
		return false ;
	}
	/********************************/
	//Patch the FPGA bug .
	memset( &ThisTrigger , 0X00 , sizeof( SophistcateTrigger ) ) ;
	ThisTrigger.TrigeModeX  = trigeMode ;
	ThisTrigger.m_FrameMarginForMode1   = m_FrameMargin   ;
	ThisTrigger.m_FrameIntervalForMode1 = m_FrameInterval ;
	TriggerQueue.push_back( ThisTrigger ) ;
	/********************************/
	bool bInitDev = FALSE ;
	bInitDev = InitDev(m_FrameMargin,m_FrameInterval,gray,trigeMode,trigeSync); 
	if ( bInitDev == false )
	{
		TRACE( "\n  Initialize Device Fail .\n " ) ;	
		return false ;
	}
	ReadyforTrigger = true;
	return true ;
}

bool CWlpAdapter::InitDev(UINT m_FrameMargin,UINT m_FrameInterval,UINT gray,int trigeMode,int trigeSync)
{
	if ( m_bDevopened == FALSE )
	{
		TRACE("\n Please open the device first . ");
		return FALSE ;
	}
	TRACE("\n Let's Start ");
	/*UINT filesize =  (UINT)m_filenames.size( ) ;
	if ( m_filenames.size() <= 0 )
	{
		TRACE("\n  Are you kidding? You havn't seleceted image files. ");
		return FALSE ;
	}*/
	int ret;
	//Step 4.init device
	TRACE("\n  Init device ") ;
	if ( DevInit.dmdtype == 0XFF )
	{
		TRACE("\n  There is NO DMD , it will does not continue ! ");
	}
	switch( DevInit.dmdtype )
	{
		case WLP_DMDTYPE_XGA_055A:
			m_BitFrameSize = Length_720P*Widthth_720P/8;		
			break;

		case WLP_DMDTYPE_XGA_07A:
			m_BitFrameSize = Length_720P*Widthth_720P/8;    
			break;
		case WLP_DMDTYPE_1080P_095A:
			m_BitFrameSize = Length_1080P*Widthth_1080P/8;    
			break;
		default:
			break;
	}
	TRACE("\n  BitFrameSize is 0D%d Bytes = 0X%X Bytes =  %5.2f KB =  %5.2f MB "  ,m_BitFrameSize, m_BitFrameSize  , 
		(float)((float)m_BitFrameSize /(ULONG)1024) , (float)((float)m_BitFrameSize /(ULONG)1048576) );
	DevInit.gray = gray ;
	TRACE("\n  Gray : %X "  , DevInit.gray );
	DevInit.frame_margin = m_FrameMargin ; 
	TRACE("\n  Frame Margin : %d "  , m_FrameMargin );
	DevInit.frame_interval = m_FrameInterval ; 
	TRACE("\n  Frame Interval : %d "  , m_FrameInterval ) ;
	switch( trigeMode )
	{
		case WLP_TRIGGER_SINGLE_FRAME:
			printf("\n  Input Trigger set:  SINGLE_FRAME "  );
			break;
		case WLP_TRIGGER_FRAMES:
			printf("\n  Input Trigger set:  FRAME "  );
			break;
		case WLP_TRIGGER_FRAMES_AND_AUTOSTOP:
			printf("\n  Input Trigger set:  FRAME and AUTOSTOP "  );
			break;
		case WLP_TRIGGER_LOOP:
			printf("\n  Input Trigger set:  LOOP "  );
			break;
	}
	switch( trigeSync )
	{
		case WLP_SYNC_ALL_FRAMES:
			printf("\n  Output Trigger set  Enable "  );
			break;
		case WLP_SYNC_SINGLE_FRAME :
			printf("\n  Output Trigger set  Enable "  );
			break;
	}
	DevInit.trigger_setting = TrigerSetALL(trigeMode,trigeSync);
	ret = this->WlpDevInit2( device , &DevInit ) ;
	if ( ret == WLP_SUCCESS )
	{
		TRACE("\n  WlpDevInit2  ------- Init success. ");
		return true ;
	}
	else
	{
		TRACE("\n  WlpDevInit2  ------- Init fail. ");
		return false ;
	}
	return true ;
}

LONG  CWlpAdapter::WlpDevInit2( WLP_HDEVICE hDevice, PDEVINIT2 pDevInit )
{
	if (pDevInit == NULL)
		return WLP_ERR_HDEVICE;
	//set gray
	if ( WlpDevControl( hDevice , WLP_DEV_GRAY , &pDevInit->gray ) != WLP_SUCCESS )
	{
		TRACE("\n Set gray ERROR! ");
		return WLP_ERR_GRAY;
	}
	// set freq
	ULONG ret = WlpDevControl(hDevice , WLP_DEV_FRAME_MARGIN , &pDevInit->frame_margin );
	if (ret != WLP_SUCCESS)
	{
		TRACE("\n Set OLD Active Time ERROR ! ");
		return ret ;
	}
	ret = WlpDevControl(hDevice , WLP_DEV_FRAME_INTERVAL , &pDevInit->frame_interval );
	if (ret != WLP_SUCCESS)
	{
		TRACE("\n Set OLD Blank Time ERROR ! ");
		return ret ;
	}
	if( !Wlp_DevNewFrequencySet(pDevInit->gray,pDevInit->frame_margin,
		pDevInit->frame_interval))
	{
		TRACE("\n Set New Frequency ERROR ! ");		
		return WLP_ERR ;
	}
	//set trigger
	ret = WlpDevControl(hDevice, WLP_DEV_TRIGGER_SETTING, &pDevInit->trigger_setting) ;
	if (ret != WLP_SUCCESS)
	{
		TRACE("\n Set trigger mode ERROR ! ");
		return ret ;
	}
	// all init success
	return WLP_SUCCESS ;
}

bool CWlpAdapter::Wlp_DevNewFrequencySet(ULONG gray, ULONG frame_margin,ULONG frame_interval)
{
	FrameActiveTime = frame_margin ;
	int DMD_LOAD_TIME = 0 ;
	if ( DMDtype == WLP_DMDTYPE_1080P_095A )
		DMD_LOAD_TIME = (int)DMD_LOAD_TIME_1080p ;
	else if (  DMDtype == WLP_DMDTYPE_XGA_07A )
		DMD_LOAD_TIME = (int)DMD_LOAD_TIME_720P ;
	else
		DMD_LOAD_TIME = (int)DMD_LOAD_TIME_720P ;
	float Index1 =  0 ;
	Index1 =  (float)pow( (LONG)2 , (double)gray ) - 1 ;
	if(((DMD_RESET_TIME+DMD_LOAD_TIME)*Index1<=(int)frame_margin) &&((DMD_RESET_TIME + DMD_LOAD_TIME)<= (int)frame_interval) 
		||((frame_interval == 0)&&((DMD_RESET_TIME + DMD_LOAD_TIME )*Index1 <= frame_margin)))
		TRACE( "\n  Set Normal Mode ! " ) ;
	else if(((DMD_RESET_TIME + FAST_CLEAR_TIME )*Index1 <= frame_margin *LimitEdge2)    // Test For LuanShuHui
		&&((DMD_RESET_TIME + DMD_LOAD_TIME)*Index1 >= frame_margin * LimitEdge1) 
		&&((DMD_RESET_TIME + DMD_LOAD_TIME)*3<= frame_interval * LimitEdge1))
	{
		TRACE( "\n  Set Fast clear Mode ! " ) ;
	}
	else
	{
		TRACE( "\n  Neither Mode ! \n  Improper ActiveTime / BlankTime setting !  \n  See the 'New function of FARO'DMD.doc 'for more detail ! " ) ;
		return  false ;

	}
	ULONG ret ;
	ret = WlpDevNewFrequencySet(device,gray,frame_margin,frame_interval);
	if ( ret != WLP_SUCCESS )
	{
		TRACE( " Set Frequency Error ! " ) ; 
		return false;
	}
	return true ;
}

void CWlpAdapter::GetPicSize(int* iWidth, int* iHeight)
{
	switch( DevInit.dmdtype )
	{
		case WLP_DMDTYPE_XGA_055A:
			*iWidth  = Length_720P ;
			*iHeight =  Widthth_720P ;
			break;
		case WLP_DMDTYPE_XGA_07A:
			*iWidth  = Length_720P ;
			*iHeight =  Widthth_720P ;
			break;
		case WLP_DMDTYPE_1080P_095A:
			*iWidth  = Length_1080P ;
			*iHeight = Widthth_1080P ;
			break;
		default:
			break;
	}
}

bool CWlpAdapter::DownloadPic(unsigned char* pPicData,int iPicCount)
{
	//Step 6.calc DownLoadLen  
	TRACE("\n  Calculating for downloading images :  ");
	ULONG SingleFrameLen = m_BitFrameSize * DevInit.gray ;
	ULONG SingleFrameLen_1080p_EXTEND = ( Length_1080P_EXTEND *Widthth_1080P / 8 ) * DevInit.gray ;
	TRACE( "\n   Firstly, The black image prelude .  Add on 2013.09.25  "   );
	if ( DMDtype == WLP_DMDTYPE_1080P_095A )
		SingleFrameLen = SingleFrameLen_1080p_EXTEND ;
	if (pLargeImageBuff)
		free(pLargeImageBuff) ;
	pLargeImageBuff = (PUCHAR)malloc( sizeof(BYTE)*SingleFrameLen );
	if ( pLargeImageBuff == NULL )
	{
		TRACE( "\n  *** Can't alloc memory for The black image prelude !!! *** "   );
		return FALSE ;
	}
	memset( pLargeImageBuff , 0x00, sizeof(BYTE)*SingleFrameLen ) ;
	ULONG ret1 = 0 ;
	ret1 = SingleDownload( 0 , SingleFrameLen , 0 , 0 , SingleFrameLen ) ;
	if ( ret1 != WLP_SUCCESS )
	{
    free(pLargeImageBuff);
		TRACE(" \n               ------- The black image prelude fail ! \n  ");  
		return false ;
	}
	else
		TRACE(" \n               ------- The black image prelude OK !  \n  ");  
	free( pLargeImageBuff ) ;
	TRACE( "\n  OK , Let's download the real images:  "   );


	ULONG LargeImageBuffSize = SingleFrameLen * iPicCount;
	pLargeImageBuff = (PUCHAR)malloc(sizeof(BYTE)*LargeImageBuffSize);
	if ( pLargeImageBuff == NULL )
	{
		printf( "\n  *** Can't alloc memory !!! ***"   );
		return FALSE ;
	}
	float MByte = (float)LargeImageBuffSize/(ULONG)1048576 ;
	TRACE("\n  You determine to download 100 pictures one time." );
	TRACE("\n  Alloc Memory for download : %10.5f M  " , MByte );

	memcpy(pLargeImageBuff,pPicData,LargeImageBuffSize);
	//memset( pLargeImageBuff , 0X00 , sizeof(UCHAR)*LargeImageBuffSize  ) ;
	m_Alldonwloadfilecount = iPicCount;//0 ;
	UINT TotalConvertFileCount = 0 ;
	UINT ConvertFileCountOneTime = 0 ; 
	UINT DownloadTimes = 0 ;
	ULONG ThisTimeStartAddr = 0+SingleFrameLen;  

	ret1 = SingleDownload(ThisTimeStartAddr ,LargeImageBuffSize , 
				DownloadTimes ,ConvertFileCountOneTime ,SingleFrameLen ) ;
	if ( ret1 != WLP_SUCCESS )
			goto EndDown ;
	free( pLargeImageBuff );
	pLargeImageBuff = NULL ;
	return true ;
EndDown:
	free( pLargeImageBuff );
	pLargeImageBuff = NULL ;
	return false; //13.09.11 
}

ULONG CWlpAdapter::SingleDownload(ULONG ThisTimeStartAddr ,ULONG SingleDownSize , 
	ULONG DownloadTimes ,ULONG ThisTimeConvertFileCount,ULONG SingleFrameLen )
{
	ULONG ret ;
	//Step 5.Set download addr
	if ( ThisTimeStartAddr + SingleDownSize >= RAMsize )
	{
		TRACE("\n  Warming : the image data to download is exceed the DDR RAM size , the download size will be truncated ");
		TRACE("\n   Download for the %4d time, and the last time ." , DownloadTimes +1  ) ;
		//Step 7. download frame data to DMD
		TRACE("\n       Downloading:  "  );
		ULONG temp = RAMsize - ThisTimeStartAddr ;
		float RemainSizeMB = (float)( (temp)/1048576 ) ;
		TRACE("\n       The remain RAM size %5f MB , which is only available for about %4d pictures, " , RemainSizeMB , ( RAMsize - ThisTimeStartAddr)/SingleFrameLen  );
		TRACE("\n       Download Start at %5f MB  " ,(float)ThisTimeStartAddr/1048576  );
		TRACE("\n       ``````````````  "  );
		ret = WlpDevLoadFrame( device , pLargeImageBuff , ThisTimeStartAddr , RAMsize - ThisTimeStartAddr ) ;
		if ( ret == WLP_SUCCESS )
		{
			printf("\n       Load frames success.");
			m_Alldonwloadfilecount = m_Alldonwloadfilecount +  (RAMsize - ThisTimeStartAddr)/SingleFrameLen ;
			printf("\n       Have download %6d pictures\n " , m_Alldonwloadfilecount ) ;
			printf("\n   _____________________________________");
		}
		else
		{
			printf("\n       Load frames fail.");
			return WLP_ERR ;
		}
		return WLP_RAM_FULL ;
	}
	TRACE("\n   Download for the %4d time. " , DownloadTimes +1  ) ;
	//Step 7. download frame data to DMD
	TRACE("\n       Downloading:  "  );
	TRACE("\n       This time will download %4d pictures, the size is %5f MB " , ThisTimeConvertFileCount, (float)SingleDownSize/1048576  );
	TRACE("\n       Download Start at %5f MB  " ,(float)ThisTimeStartAddr/1048576  );
	TRACE("\n       ``````````````  "  );
	ret = WlpDevLoadFrame( device , pLargeImageBuff , ThisTimeStartAddr ,SingleDownSize ) ;
	if ( ret == WLP_SUCCESS )
	{
		TRACE("\n       Download frames success: ");
		m_Alldonwloadfilecount = m_Alldonwloadfilecount + ThisTimeConvertFileCount ;
		TRACE("         Have download %6d pictures" , m_Alldonwloadfilecount );
		ULONG xsize = (ULONG)( RAMsize - ThisTimeStartAddr - SingleDownSize );
		TRACE("         The RAM remains %6f MB\n " , (float)xsize/1048576  );
		TRACE("\n   _____________________________________");
	}
	else
	{
		TRACE("\n       Load frames fail.");
	}
	return ret ;
}

int CWlpAdapter::Start()
{
	ULONG param2 = 0;
	ULONG ret = 0;
	// Set DMD start Addr
	TRACE("\n   Set DMD display start Addr at: "  );
	param2 = 0;//m_BitFrameSize * DevInit.gray ;//0 ;
	ret =  WlpDevControl(device, WLP_DEV_DISPLAY_START_SET , &param2) ;
	if (ret == WLP_SUCCESS)
		TRACE("\nWLP_DEV_DISPLAY_START_SET    %d MB " , param2 );
	else
		TRACE("\nWLP_DEV_DISPLAY_START_SET    Set FAIL! "  );
	TRACE("\n   Set DMD display length: "  );
	TRACE("\n               -------  include the black image prelude ! "  );
	// For The black image prelude .  Add on 2013.09.25 
	if ( DMDtype == WLP_DMDTYPE_1080P_095A )
		m_BitFrameSize = Length_1080P_EXTEND*Widthth_1080P /8 ;
	param2 = ( m_Alldonwloadfilecount + 1 )* m_BitFrameSize * DevInit.gray   ; 
	ret =  WlpDevControl(device, WLP_DEV_DISPLAY_LENGTH_SET , &param2 ) ;
	if (ret == WLP_SUCCESS )
		TRACE("\nWLP_DEV_DISPLAY_LENGTH_SET   %10d B = %6f MB " , param2 ,(float)param2/1048576 );
	else
		TRACE("\nWLP_DEV_DISPLAY_LENGTH_SET   Set FAIL! "  );
	/***************************************************************************************/
	//Step 9.start dmd
	printf("\n   Start dmd \n"  );
	//下面是直接置0或置1，所以这里param2等于什么值不重要
	ret =  WlpDevControl( device , WLP_DEV_DMD_START , &param2 ) ;
	if (ret == WLP_SUCCESS)
		TRACE("               ------- Start dmd success.");
	else
	{
		TRACE("               ------- Start dmd fail.");
		return 0;
	}
	//wait for starting dmd to finish.
	Sleep(100);
	/***************************************************************************************/
	TRACE(" Please 'trigger' to roll the picturs....\n") ;
  return 0;
}

void CWlpAdapter::Stop()
{
	if ( m_bDevopened == FALSE )
	{
		TRACE("\n Please open the device first . ");
		return ;
	}
	UINT param2 = 0;
	int ret =  WlpDevControl(device, WLP_DEV_DMD_STOP, &param2) ;
	if (ret == WLP_SUCCESS)
	{
		ReadyforTrigger = false ;
		TRACE("\n Stop DMD success.");
	}
	else
		TRACE("\n Stop DMD fail.");
	/********************************/
	//Patch the FPGA bug .
	TriggerQueue.clear( ) ;
	/********************************/
}

bool CWlpAdapter::Trigger()
{
	if ( m_bDevopened == FALSE )
	{
		TRACE("\n Please open the device first . ");
		return false ;
	}
	if ( ReadyforTrigger != true  )
	{
		TRACE( " \n Please download images firstly ! \n " ) ;
		return false ;
	}
	UINT param2 = 1;
	int ret  = 0 ;
	int size = (int)TriggerQueue.size();
	if (TriggerQueue.size()==1)
	{
		if((TriggerQueue[0].TrigeModeX == WLP_TRIGGER_SINGLE_FRAME)) 
		{
			ret =  WlpDevControl( device , WLP_DEV_TRIGGER_SINGLE , &param2 ) ;
			if ( ret == WLP_SUCCESS )
				TRACE("  TriSingle 1 ; ");
			else
				TRACE(" \nTriger Single 1 fail.\n ");
			ULONG tempSleep = 0 ;
			tempSleep = ( ThisTrigger.m_FrameMarginForMode1  + ThisTrigger.m_FrameIntervalForMode1 ) / 1000  ;
			Sleep( tempSleep + 1 ) ; 
	 		TRACE(" Slept for %d ms ." , tempSleep +1  );
			ret =  WlpDevControl(device, WLP_DEV_TRIGGER_SINGLE, &param2 ) ;
			if (ret == WLP_SUCCESS)
				TRACE("  TriSingle 2 ; ");	
			else
				TRACE(" \nTriger Single 2 fail.\n ");
			TriggerQueue[0].TrigeModeX = WLP_TRIGGER_MODE_5 ;
			return true;	
		}
		if ((TriggerQueue[0].TrigeModeX != WLP_TRIGGER_SINGLE_FRAME)) 
		{
			ret =  WlpDevControl( device , WLP_DEV_TRIGGER_SINGLE , &param2 ) ;
			if ( ret == WLP_SUCCESS )
				TRACE("  TriSingle ; ");
			else
				TRACE(" \nTriger Single fail.\n ");
			return true;
		}
	}
	int QueueSize = 0 ;
	QueueSize = (int)TriggerQueue.size() ;
	bool TriggerTwice = false ;
	int Other3Mode = 0 ;
	//如果当前是模式1，那么就得根据前面的切换过的模式进行判断
	if ((TriggerQueue.size()>= 2))
	{
		// 最后一个不是模式1 
		if (TriggerQueue[QueueSize-1].TrigeModeX != WLP_TRIGGER_SINGLE_FRAME  )
		{
			ret =  WlpDevControl( device , WLP_DEV_TRIGGER_SINGLE , &param2 ) ;
			if ( ret == WLP_SUCCESS )
				TRACE("  TriSingle ; ");
			else
				TRACE(" \nTriger Single fail.\n ");
			return true;
		}
		else// 最后一个是模式1 
		{
			// 从后往前找，直到找到第一个 “非模式1” ， 记录位置
			for ( int i = QueueSize -1 ; i>= 0 ; i-- )
			{
				if (TriggerQueue[i].TrigeModeX == WLP_TRIGGER_SINGLE_FRAME )
					continue ;
				// 情况1， 找到“被trigger”的模式1 ， 触发一次
				if ( TriggerQueue[ i ].TrigeModeX == WLP_TRIGGER_MODE_5 )
				{
					ret =  WlpDevControl( device , WLP_DEV_TRIGGER_SINGLE , &param2 ) ;
					if ( ret == WLP_SUCCESS )
						TRACE("  TriSingle ; ");
					else
						TRACE(" \nTriger Single fail.\n ");
					TriggerQueue[ QueueSize -1 ].TrigeModeX  = WLP_TRIGGER_MODE_5 ;
					return true;
				}
				//情况2，  前面触发了其他三种模式，触发两次
				else if (  ( TriggerQueue[ i ].TrigeModeX != WLP_TRIGGER_MODE_5 ) 
					  &&( TriggerQueue[ i ].TrigeModeX != WLP_TRIGGER_SINGLE_FRAME )  )
				{
					ret =  WlpDevControl( device , WLP_DEV_TRIGGER_SINGLE , &param2 ) ;
					if ( ret == WLP_SUCCESS )
						TRACE("  TriSingle 1 ; ");
					else
						TRACE(" \nTriger Single 1 fail.\n ");
					ULONG tempSleep = 0 ;
					tempSleep = (  ThisTrigger.m_FrameMarginForMode1  +  ThisTrigger.m_FrameIntervalForMode1 ) / 1000  ;
					Sleep( tempSleep + 1 ) ; 
		 			TRACE(" Slept for %d ms ." , tempSleep +1  );
					ret =  WlpDevControl(device, WLP_DEV_TRIGGER_SINGLE, &param2 ) ;
					if (ret == WLP_SUCCESS)
						TRACE("  TriSingle 2 ; ");
					else
						TRACE(" \nTriger Single 2 fail.\n ");
					// 被trigger了 ,记录下来
					TriggerQueue[ QueueSize -1 ].TrigeModeX = WLP_TRIGGER_MODE_5;
					return true;
				}
			}
			//情况3， 全部都是模式1，触发两次
			{
				ret =  WlpDevControl( device , WLP_DEV_TRIGGER_SINGLE , &param2 ) ;
				if ( ret == WLP_SUCCESS )
					TRACE("  TriSingle 1 ; ");
				else
					TRACE(" \nTriger Single 1 fail.\n ");
				ULONG tempSleep = 0 ;
				tempSleep = (  ThisTrigger.m_FrameMarginForMode1  +  ThisTrigger.m_FrameIntervalForMode1 ) / 1000  ;
				Sleep( tempSleep + 1 ) ; 
				TRACE(" Slept for %d ms ." , tempSleep +1  );
				ret =  WlpDevControl(device, WLP_DEV_TRIGGER_SINGLE, &param2 ) ;
				if (ret == WLP_SUCCESS)
					TRACE("  TriSingle 2 ; ");
				else
					TRACE(" \nTriger Single 2 fail.\n ");
				// 被trigger了 ,记录下来
				TriggerQueue[ QueueSize -1 ].TrigeModeX = WLP_TRIGGER_MODE_5 ;
				return true ;		 
			}
		}
	}
	return false;
}

//设置无效?
bool CWlpAdapter::TriggerExtSet(ULONG delay,bool polarity,ULONG plusewidth,ULONG ImageNum)
{
	if ( (plusewidth + delay) >= FrameActiveTime )
	{
		TRACE("\n	 Improper trigger setting !	 \n   Plusewidth add delay time must NOT LESS than the ActiveTime ! ");
		return false;
	}
	delay = delay + 7 ;
	if ( delay == 0 )
		delay = 12 ;
	else
		delay = delay*48 ;
	ULONG ret = 0 ;
	ret = WlpDevTriggerExtSet(device,delay,polarity,plusewidth *48,ImageNum);
	if (ret == WLP_SUCCESS)
	{
		TRACE("\n	 Set Trigger Extension	 ----- success.");
		return true ;
	}
	else
	{
		TRACE("\n	 Set Trigger Extension	 ----- fail.");
		return false ;
	}
}

char* CWlpAdapter::UsePixel2bin(char* pSrc, int width,int height, unsigned char bitmask, char* pOut)
{
	return pixel2bin(pSrc,width,height,bitmask,pOut);
}

char* CWlpAdapter::UseBin2pixel(char* pSrc, int width,int height, unsigned char bitmask, char* pOut)
{
	return bin2pixel(pSrc,width,height,bitmask,pOut);
}

#else

CWlpAdapter::CWlpAdapter(void)
{
  device = 0;
  m_bDevopened = false;
  m_Alldonwloadfilecount = 0;
  FrameActiveTime = 0;
  RAMsize = 0;
  FWdownloaded = 0;
  pLargeImageBuff = 0;
}

CWlpAdapter::~CWlpAdapter(void)
{
  if (device != 0)
    CloseDev();
}

bool CWlpAdapter::OpenDev()
{
  return false;
}

bool CWlpAdapter::Read_DDR_SPD()
{
  return false;
}

LONG CWlpAdapter::I2CCtrl(I2C_OPER I2COper)
{
  return 0;
}

LONG CWlpAdapter::I2CReadData(PULONG RData)
{
  return 0;
}

void CWlpAdapter::CloseDev()
{
}

LONG CWlpAdapter::SetTrigerMode1(int trigeMode, int trigeSync)
{
  return 0;
}

LONG CWlpAdapter::SetTrigerMode2(int trigeMode, int trigeSync)
{
  return 0;
}

ULONG CWlpAdapter::TrigerSetALL(int trigeMode, int trigeSync)
{
  return 0;
}

bool  CWlpAdapter::InitForDownload(UINT m_FrameMargin, UINT m_FrameInterval, UINT gray, int trigeMode, int trigeSync)
{
  return false;
}

bool CWlpAdapter::InitDev(UINT m_FrameMargin, UINT m_FrameInterval, UINT gray, int trigeMode, int trigeSync)
{
  return false;
}

LONG  CWlpAdapter::WlpDevInit2(WLP_HDEVICE hDevice, PDEVINIT2 pDevInit)
{
  return 0;
}

bool CWlpAdapter::Wlp_DevNewFrequencySet(ULONG gray, ULONG frame_margin, ULONG frame_interval)
{
  return false;
}

void CWlpAdapter::GetPicSize(int* iWidth, int* iHeight)
{
}

bool CWlpAdapter::DownloadPic(unsigned char* pPicData, int iPicCount)
{
  return false;
}

ULONG CWlpAdapter::SingleDownload(ULONG ThisTimeStartAddr, ULONG SingleDownSize,
  ULONG DownloadTimes, ULONG ThisTimeConvertFileCount, ULONG SingleFrameLen)
{
  return 0;
}

int CWlpAdapter::Start()
{
  return 0;
}

void CWlpAdapter::Stop()
{
}

bool CWlpAdapter::Trigger()
{
  return false;
}

bool CWlpAdapter::TriggerExtSet(ULONG delay, bool polarity, ULONG plusewidth, ULONG ImageNum)
{
  return false;
}

char* CWlpAdapter::UsePixel2bin(char* pSrc, int width, int height, unsigned char bitmask, char* pOut)
{
  return 0;
}

char* CWlpAdapter::UseBin2pixel(char* pSrc, int width, int height, unsigned char bitmask, char* pOut)
{
  return 0;
}

#endif
