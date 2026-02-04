
#ifndef TMC_PROTO_H
#define TMC_PROTO_H

//Opcodes of all TMCL commands that can be used in direct mode
#define TMCL_ROR 1		//Rotate right with specified velocity
#define TMCL_ROL 2		//Rotate left with specified velocity
#define TMCL_MST 3		//Stop motor movement
#define TMCL_MVP 4		//Move to position (absolute or relative)
#define TMCL_SAP 5		//Set axis parameter (motion control specific settings)
#define TMCL_GAP 6		//Get axis parameter (read out motion control specific settings)
#define TMCL_STAP 7
#define TMCL_RSAP 8
#define TMCL_SGP 9
#define TMCL_GGP 10
#define TMCL_STGP 11
#define TMCL_RSGP 12
#define TMCL_RFS 13
#define TMCL_SIO 14
#define TMCL_GIO 15
#define TMCL_SCO 30
#define TMCL_GCO 31
#define TMCL_CCO 32

//Opcodes of TMCL control functions (to be used to run or abort a TMCL program in the module)
#define TMCL_APPL_STOP 128
#define TMCL_APPL_RUN 129
#define TMCL_APPL_RESET 131

//Options for MVP commandds
#define MVP_ABS 0
#define MVP_REL 1
#define MVP_COORD 2

//Options for RFS command
#define RFS_START 0
#define RFS_STOP 1
#define RFS_STATUS 2

//Options for SAP command
#define TARGET_POS		0
#define ACTUAL_POS		1
#define TARGET_SPEED	2
#define ACTUAL_SPEED	3
#define MAX_SPEED		4
#define MAX_ACCELERATION	5	//加速度
#define MAX_CURRENT		6

#define POSITION_RECHED	8		//是否达到指定目标
#define MICRO_STEP		140		//电机细分


#define FALSE 0
#define TRUE 1

//Return code
#define TMC_STATUS_OK	100

//Result codes for GetResult
#define TMCL_RESULT_OK 0
#define TMCL_RESULT_NOT_READY 1
#define TMCL_RESULT_CHECKSUM_ERROR 2



#endif