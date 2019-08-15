#ifndef			_IEC_PROTOCOL_H
#define			_IEC_PROTOCOL_H
/************************ 缩写符***************************/
/*		缩写符		    中文名称			英文名称					*/
/*		ASDU	      应用数据服务单元	Application Service data Unit	*/
/*		APCI		  应用规约控制信息  Application Protocol control	*/
/*										Information						*/
/*		COM			  命令				Command							*/
/*		COT			  传送原因			Cause Of Transmission			*/
/*		CU			  通信单元			Communication Unit				*/
/*		CP32Time2a	  4个8位位组时间	Four Object Binary Time			*/
/*		CP56Time2a	  7个8位位组时间	Seven Object Binary Time		*/
/*		DCO			  双命令			Double Command					*/
/*		DFC			  数据流控制		Data flow control				*/
/*		DPI			  双点信息			Double-Point Information		*/
/*		DR			  差错				Error							*/
/*		FAN			  故障序号			Fault Number					*/
/*		F-Code		  功能码			Function code					*/
/*		FCB			  帧计数位			Frame Count Bit					*/
/*		FCV			  帧计数有效位		Frame Count Bit Valid			*/
/*		FT			  帧传输格式		Frame Transmission Format		*/
/*		FUN			  功能类型			FUNCTTION　TYPE					*/
/*		GDD			  通用分类数据描述  Generic data description		*/
/*		GEN			  通用分类功能类型  Generic function type			*/
/*		GGI			  通用分类数据总	Generic interrogation of		*/
/*					   查询(总召唤)		  generic data					*/
/*		GI			  总查询(总召唤)	Generic interrogation			*/
/*		IEC			  国际电工委员会    International Electrotechnical	*/
/*											Commission					*/
/*		IED			  智能电子装置		Intrlligent Electronic Device   */
/*		INF			  信息符号			Information Number				*/
/*		IV			  无效				Invalid							*/
/*		LPCI		  链路规约控制信息  Link Protocol Control			*/
/*											Information					*/
/*		LPDU		  链路规约数据单元  Link Protocol Data Unit			*/
/*		MEA			  带品质描述词的	Measurand with quality			*/
/*					  被测值			 descriptor						*/
/*		MVAL		  被测值的值		Value of measurand				*/
/*		NO			  数目				Number							*/
/*		NOC			  通道数目			Number of Channels				*/
/*		OV			  溢出				Overflow						*/
/*		P			  有功功率			Active Power					*/
/*		PRM			  启动报文			Primary Message					*/
/*		Q			  无功功率			Reactive Power					*/
/*		RES			  备用				Reserved						*/
/*		RET			  相对时间			Relative time					*/
/*		RII			  返回信息标识符	Return Information Identifier	*/
/*		SCN			  扫描序号			Scan Number						*/
/*		SIN			  附加信息			Supplementary Information		*/
/*		SU			  夏季标志位		Summer Bit						*/
/*		SQ			  同类信息元素		Sequence of equal information	*/
/*						的顺序				elements					*/
/*		TOO			  命令类型			Type Of Order					*/
/*		TYP			  类型标识			Type Identification				*/
/************************************************************************/
//---------------------IEC60870-5-103-----------------------------------
// fellow define for IEC103 command
#define	RESETCU									0				// reset CU
#define SENDCON									3				// send confirm frame
#define SENDNOCON								4				// send no confirm frame
#define RESETFCB 	 							7				// reset FCB
#define REQUIRELINK 	 						9				// require link status
#define REQUIREFIRSTDATA  						10				// require first data
#define REQUIRESECONDDATA  						11				// require second data

#define BROADCAST_ADDR							0xff


#define	START_CODE1								0x10			// 103 protocol start code1
#define	START_CODE2								0x68			// 103 protocol start code2
#define	END_CODE								0x16			// 103 protocol end code
#define	_FCV									0x10			// 103 protocol FCV bit
#define	_FCB									0x20			// 103 protocol FCB bit
#define	_PRM									0x40			// 103 protocol PRM bit
#define	_DFC									0x10			// 103 protocol DFC bit
#define	_ACD									0x20			// 103 protocol ACD bit

/*
//---------------------IEC60870-5-103-----------------------------------
// fellow define for IEC103 command
#define	RESETCU									0				// reset CU
#define SENDCON									3				// send confirm frame
#define SENDNOCON								4				// send no confirm frame
#define RESETFCB 	 							7				// reset FCB
#define REQUIRELINK 	 						9				// require link status
#define REQUIREFIRSTDATA  						10				// require first data
#define REQUIRESECONDDATA  						11				// require second data
*/

#define IEC103_FUN_FCK							0x01


#define	SECT_MONITOR							0x00


#define	IEC103_TI_M_TM_TA_3     				1  				// Frame with time tag
#define	IEC103_TI_M_TMR_TA_3    				2 				// Frame with relative time tag
#define	IEC103_TI_M_MEI_NA_3    				3 				// measure valueI
#define	IEC103_TI_M_TME_TA_3    				4 				// measure value with relative time tag
#define	IEC103_TI_M_IRC_NA_3    				5 				// Reset CU
#define	IEC103_TI_M_SYN_TA_3    				6 				// clock synchronicity
#define	IEC103_TI_M_TGI_NA_3    				8 				// end of total call
#define	IEC103_TI_M_MEII_NA_3   				9 				// measure valueII
#define	IEC103_TI_M_GD_TA_3     				10 				// GENERAL DATA
#define	IEC103_TI_M_GI_NA_3     				11 				// general type identification
//#define	IEC103_TI_M_VER_1     					5  				// monitor ver

#define	IEC103_TI_M_LRD_TA_3					23				// disturbance table recorded

#define IEC103_TI_C_ODT_NA_3					24				// disturbance data transmission command
#define IEC103_TI_C_ADT_NA_3					25				// disturbance data traunsimmion authorization

#define	IEC103_TI_M_RTD_TA_3					26				// disturbance data transmission ready
#define IEC103_TI_M_RTC_NA_3					27				// recorded channel data transmission ready
#define IEC103_TI_M_RTT_NA_3					28				// state change with flag transmission ready
#define IEC103_TI_M_TOT_TA_3					29				// state change with flag transmission
#define	IEC103_TI_M_TOV_NA_3					30				// transmit	disturbance data
#define IEC103_TI_M_EOT_TA_3					31				// end of transmission


#define	IEC103_TI_C_SET_NA_3    				61 				// Set query and  return command
#define	IEC103_TI_C_IGI_NA_3    				7 				// general inspect
#define	IEC103_TI_C_VER_1    					60 				// load ver
#define	IEC103_TI_C_GRC_NA_3    				20 				// modify soft_jump/set_zone/signal_reset
#define IEC103_TI_M_IT_NA_3						36
#define	IEC103_TI_M_ST_NA_3						38
#define IEC103_TI_M_ST_TA_3						39
#define IEC103_TI_M_SP_NA_3						40				// single-point information without time tag,for general inspect
#define IEC103_TI_M_SP_TA_3						41              // single-point information with time tag
#define IEC103_TI_M_DP_NA_3						42				// double-point information without time tag,for general inspect
#define IEC103_TI_M_DP_TA_3						43				// double-point information with time tag
#define IEC103_TI_M_MEVII_NA_3					50
#define IEC103_TI_C_DC_NA_3						64
#define IEC103_TI_C_RC_NA_3						65
#define IEC103_TI_C_CC_NA_3						67
#define	IEC103_TI_M_ASDU70						70
#define IEC103_TI_C_CI_NA_3						88				// counter interrogation command


#define	SET_A_QUERY 		       				100 			// set query
#define	SET_A_PRE	     						101				// total Set change prepare
#define	SET_A_PRE_RET     						102				// total Set change return
#define	SET_A_EXE			     				103				// total Set change executive
#define	SET_A_EXE_RET		  					104				// total Set change executive return sure
#define	SET_A_UNDO			     				105				// total Set change cancel
#define	SET_A_UNDO_RET			   				106				// total Set change cancel sure
#define	SET_S_PRE								107
#define SET_S_PRE_RET							108
#define SET_S_EXE								109
#define	SET_S_EXE_RET							110
#define	SET_S_UNDO								111
#define	SET_S_UNDO_RET							112



/*	Process information in monitor direction */
#define IEC104_TI_M_SP_NA_1						1				// single-point information without time tag
#define IEC104_TI_M_SP_TA_1						2				// single-point information with time tag
#define IEC104_TI_M_DP_NA_1						3				// double-point information without time tag
#define IEC104_TI_M_DP_TA_1						4				// double-point information with time tag
#define IEC104_TI_M_ST_NA_1						5				// step position information without time tag
#define IEC104_TI_M_ST_TA_1						6				// step position information with time tag
#define IEC104_TI_M_BO_NA_1						7				// bistring of 32 bits
#define IEC104_TI_M_ME_NA_1						9				// measured value,normalize value
#define IEC104_TI_M_ME_TA_1						10				// measured value with time tag
#define	IEC104_TI_M_ME_NB_1						11				// measured value,scaled
#define IEC104_TI_M_ME_NC_1						13				// measured value,short floating point number
#define IEC104_TI_M_IT_NA_1						15				// integrated totals
#define IEC104_TI_M_IT_TA_1						16				// integrated totals with time tag
#define IEC104_TI_M_EP_TA_1						17				// protective or autoclose event report with time tag
#define IEC104_TI_M_PS_NA_1						20				// packed single-point information with status change detection
#define IEC104_TI_M_ME_ND_1						21				// measured value,normalized value without quality descriptor
#define IEC104_TI_M_ME_NE_1						22
#define IEC104_TI_M_SP_TB_1						30				// single-point information with time tag CP56Time2a
#define IEC104_TI_M_DP_TB_1						31				// double-point intormation with time tag CP56Time2a
#define IEC104_TI_M_ST_TB_1						32				// step position information with time tag CP56Time2a
#define IEC104_TI_M_BO_TB_1						33				// bitstring of 32 bit with time tag CP56Time2a
#define IEC104_TI_M_ME_TD_1						34				// measured value,normalized value with time tag CP56Time2a
#define IEC104_TI_M_ME_TE_1						35				// measured value,scaled value with time tag CP56Time2a
#define IEC104_TI_M_ME_TF_1						36				// measured value,short float point number with time tag CP56Time2a
#define IEC104_TI_M_IT_TB_1						37				// integrated totals with tag CP56Time2a
#define IEC104_TI_M_EP_TD_1						38				// event of protection equipment with time tag CP56Time2a
#define IEC104_TI_M_EP_TE_1						39				// packed start events of protection wquipment with time tag CP56Time2a
#define IEC104_TI_M_EP_TF_1						40				// packed output circuit information of protection equipment with time tag CP56Time2a
//<41..44>														// reserved for furthure compatible definitions


/* Process information in control direction */
#define IEC104_TI_C_SC_NA_1						45				// single command
#define IEC104_TI_C_DC_NA_1						46				// double command
#define IEC104_TI_C_RC_NA_1						47				// regulating step command
#define IEC104_TI_C_SE_NA_1						48				// set point command,normalized value
#define IEC104_TI_C_SE_NB_1						49				// set point command,scaled value
#define IEC104_TI_C_SE_NC_1						50				// set point command,short floating-point number
#define IEC104_TI_C_BO_NA_1						51				// botstromg of 32 bits
//<52..57>														// reserved for further compatible definitions
#define IEC104_TI_C_SC_TA_1						58				// single command with time tag CP56Time2a
#define IEC104_TI_C_DC_TA_1						59				// double command with time tag CP56Time2a
#define IEC104_TI_C_RC_TA_1						60				// regulating step command with time tag CP56Time2a
#define IEC104_TI_C_SE_TA_1						61				// set point command,normalized value with time tag CP56Time2a
#define IEC104_TI_C_SE_TB_1						62				// set point command,scaled value with time tag CPTime2a
#define IEC104_TI_C_SE_TC_1						63				// set point command,short floating-point number with time tag CP56Time2a
#define IEC104_TI_C_BO_TA_1						64				// bitstring of 32 bits with time tag CP56Time2a

//<65..69>														// reserved for further compatible definitions

/*System information in monitor direction*/
#define  IEC104_TI_M_EI_NA_1					70				// end of initialization
//<71..99>														// reserved for further compatible definitions
/*system information in control direction*/
#define	IEC104_TI_C_IC_NA_1						100				// interrogation command
#define IEC104_TI_C_CI_NA_1						101				// counter interrogation command
#define IEC104_TI_C_RD_NA_1						102				// read command
#define IEC104_TI_C_CS_NA_1						103				// clock synchronization command
#define IEC104_TI_C_RP_NA_1						105				// reset process command
#define IEC104_TI_C_TS_TA_1						107				// test command with time tag CP56Time2a
//<108..109>													// reserved for furter compatible definitions

/* Parameter control direction*/
#define  IEC104_TI_P_ME_NA_1					110				// parameter of measured value,normalized value
#define  IEC104_TI_P_ME_NB_1					111				// parameter of measured value,scaled value
#define  IEC104_TI_P_ME_NC_1					112				// parameter of measured value,short float-point number
#define  IEC104_TI_P_AC_NA_1					113				// patameter activation
//<114..119>													// reserved for furter compatible definitions

/* file translate*/
#define  IEC104_TI_F_FR_NA_1					120				// file ready
#define  IEC104_TI_F_SR_NA_1					121				// section ready
#define  IEC104_TI_F_SC_NA_1					122				// call directory,select file, call file,call section
#define  IEC104_TI_F_LS_NA_1					123				// last section,last segment
#define  IEC104_TI_F_AF_AN_1					124				// ack file,ack section
#define  IEC104_TI_F_SG_NA_1					125				// segment
#define  IEC104_TI_F_DR_TA_1					126				// directory


#define	IEC103_COT_M_per						1 				// outburst
#define	IEC103_COT_M_cyc          				2 				// cycle
#define	IEC103_COT_M_resetFCB     				3 				// reset FCB
#define	IEC103_COT_M_resetCU      				4 				// reset ccommunication unit
#define	IEC103_COT_M_setup        				5 				// setup or start
#define	IEC103_COT_M_powerON      				6 				// take on power
#define	IEC103_COT_M_testMODE     				7 				// test mode
#define	IEC103_COT_M_synCLOCK     				8 				// sysnchronicity clock
#define	IEC103_COT_M_totalQUERY   				9 				// TOTAL QUERY
#define	IEC103_COT_M_queryEND    				10 				// total query end
#define	IEC103_COT_M_localOP     				11 				// local operation
#define	IEC103_COT_M_remoteOP    				12 				// remote operation
#define	IEC103_COT_M_actcon      				20 				// command confirm
#define	IEC103_COT_M_deactcon    				21 				// command deny confirm
#define	IEC103_COT_M_disturbD    				31 				// disturb data send
#define	IEC103_COT_M_GenActCon   				40 				// general classify command confirm
#define	IEC103_COT_M_Gendeactcon 				41 				// general classify command deny confirm
#define	IEC103_COT_M_intro5      				42 				// read availibility
#define	IEC103_COT_M_intro6      				43 				// read invalid
#define IEC103_COT_M_opendeactcon   			82				//
#define IEC103_COT_M_countdectcon   			64


#define  IEC104_COT_M_PN						0x40
#define	 IEC104_COT_M_cyc						1
#define  IEC104_COT_M_scan						2
#define  IEC104_COT_M_spont						3
#define	 IEC104_COT_M_init						4
#define  IEC104_COT_M_req						5
#define	 IEC104_COT_M_act						6
#define  IEC104_COT_M_acton						7
#define  IEC104_COT_M_deact						8
#define  IEC104_COT_M_deactcon					9
#define  IEC104_COT_M_actterm					10
#define	 IEC104_COT_M_retrem					11
#define  IEC104_COT_M_retloc					12
#define	 IEC104_COT_M_file						13
#define  IEC104_COT_M_introgen					20
#define	 IEC104_COT_M_intro1					21
#define  IEC104_COT_M_intro2					22
#define  IEC104_COT_M_intro3					23
#define  IEC104_COT_M_intro4					24
#define  IEC104_COT_M_reqcogen					37
#define  IEC104_COT_M_reqco1					38
#define  IEC104_COT_M_reqco2					39
#define  IEC104_COT_M_reqco3					40
#define  IEC104_COT_M_reqco4					41


#define  IEC104_QOI								20

#define	 IEC104_SE							0x80
#define	 IEC104_QU							0x7C
#define	 IEC103_SE							0x80
#define	 IEC103_UNDO						0xC0
#define	 IEC103_EXE							0x00


#define	 XJ_IEC_1								1
#define	 XJ_IEC_2								2


#define	IEC103_OFFSET_LEN						2
#define IEC103_OFFSET_CODE						4
#define	IEC103_OFFSET_ADDR						5
#define	IEC103_OFFSET_TI						6
#define	IEC103_OFFSET_VSQ						7
#define	IEC103_OFFSET_COT						8
#define	IEC103_OFFSET_COMADDR					9
#define	IEC103_OFFSET_FUN						10
#define	IEC103_OFFSET_INF						11
#define IEC103_OFFSET_CONTEXT					12
#define MIN_IEC103_FRAMELEN						8

#define	IEC104_OFFSET_LEN						1
#define	IEC104_OFFSET_CODE						2
#define	IEC104_OFFSET_TI						6
#define	IEC104_OFFSET_VSQ						7
#define	IEC104_OFFSET_COT						8
#define	IEC104_OFFSET_SECT						10
#define IEC104_OFFSET_ADDR      				11				// sub address
#define IEC104_OFFSET_INF						12
#define IEC104_OFFSET_CONTEXT					15
#define MIN_IEC104_FRAMELEN						13

#define TCP_OFFSET_LEN							1
#define TCP_OFFSET_CODE							2
#define TCP_OFFSET_TI							6
#define TCP_OFFSET_VSQ							7
#define TCP_OFFSET_COT							8
#define TCP_OFFSET_COMADDR						10
#define TCP_OFFSET_ADDR							11
#define TCP_OFFSET_FUN							12
#define TCP_OFFSET_INF							13
#define TCP_OFFSET_CONTEXT						14
#define MIN_TCP_FRAMELEN						12



#define	RC_BASE_103								58
#define	RC_BASE_104_2							0x600B
#define	RC_BASE_104_1							0xB01


#endif
