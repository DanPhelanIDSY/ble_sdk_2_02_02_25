/** @file ipup.c
 *  @brief This module is to handle IPUP (inter-Processor UART Protocol)
 *
 *test
 * Copyright (c) 2002 - 2015  I.D. Systems, Inc.
 * All Rights Reserved.  (www.id-systems.com)
 * 
 *  	Revisions:
 *  	Date	    Initials	Description
 *    ________  ________  ________________________________________________________
 *    20151012    SDS      Created
 *    20150105    JDP      Merged into BLE CC2640 project
 *  @author SDS
 *  @author IDSY
 *  @bug No known bugs.
 */


#ifndef _IPUP_C                     // ensure unique inclusion
#define _IPUP_C                     // module identifier


//------------------------------------------------------------------------------
// header files
//------------------------------------------------------------------------------

#define _INCLUDE_MODULE_IPUP
#ifdef _INCLUDE_MODULE_IPUP	//include module?

#include "stdint.h"
#include <stdbool.h>
#include "msgHandler.h"
#include "ipup.h"
#include "Board.h"
#include "SerialPortControl.h"
#include "SimpleBLECentral.h"
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	#defines:
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯
#define IPUP_MSG_IN_MAX_NUM_HANDLERS 1
//M_MSG_CODE_STM32_IDS_APP_DEST_NUM
#define PARSE_DEST

#define IPUP_BYTE_START_SENTENEL	0
#define IPUP_BYTE_PACKET_LEN1		1
#define IPUP_BYTE_PACKET_LEN0		2
#define IPUP_BYTE_PACKET_BITS1		3
#define IPUP_BYTE_PACKET_BITS0		4


//state machine for ipup rcvr
#define	    IPUP_PROCESS_STEP_WAIT_START 0 //
#define		IPUP_PROCESS_STEP_GET_LEN1  1	 //
#define		IPUP_PROCESS_STEP_GET_LEN2  2	 //
#define		IPUP_PROCESS_STEP_GET_PAYLOAD 3	 //
#define		IPUP_PROCESS_STEP_CRC 4

// Message Codes and Sub Codes Version, MAC address
#define IPUP_CC2640_TEST_CODE   5

#define IPUP_CC2640_SUBCODE_VER 1
#define IPUP_CC2640_SUBCODE_MAC 2

// Message Codes and Sub Code for advertising status
#define IPUP_CC2640_CONNECT_STATUS_CODE   7
#define IPUP_CC2640_CONNECT_STATUS_SUBCODE_STATE 1

// Message Code and Subcodes for DIO related
#define IPUP_CC2640_DIO_CODE   9
#define IPUP_CC2640_SUBCODE_DIO_STATE 1
#define IPUP_CC2640_SUBCODE_DIO_TYPE  2
#define IPUP_CC2640_SUBCODE_DIO_ARM_WAKEUP 3

// Message Code and Subcodes for BLE RF testing related
#define IPUP_CC2640_RF_TESTS_CODE   11
#define IPUP_CC2640_RF_TESTS_SUBCODE_START_DISCOVER 1
#define IPUP_CC2640_RF_TESTS_SUBCODE_STOP_DISCOVER  2
#define IPUP_CC2640_RF_TESTS_SUBCODE_DISP_DEVICE    3
#define IPUP_CC2640_RF_TESTS_SUBCODE_CONNECT_DEVICE 4
#define IPUP_CC2640_RF_TESTS_SUBCODE_DISCONNECT_DEVICE 5
#define IPUP_CC2640_RF_TESTS_SUBCODE_DEVICE_DATA 6

#define IPUP_CC2640_SUBCODE_DIO_TYPE  2
#define IPUP_CC2640_SUBCODE_DIO_ARM_WAKEUP 3

// DIO selected when requesting  DIO_STATE, DIO_TYPE, or DIO_PULSE
#define DIO_BLE_MCU_DIO_2 	2
#define DIO_18V_PG_DIO_3 	3
#define DIO_ARM_RST_DIO_4 	4
//#define DIO_PROX_DIO_5 	5 JTAG INPUTS not available
//#define DIO_PROX_DIO_6 	6 JTAG INPUTS
#define DIO_SPI_MISO_DIO_7 	7
#define DIO_SPI_MOSI_DIO_8 	8
#define DIO_ARM_WAKE_DIO_9 	9
#define DIO_BL_MODE_DIO_10 	10
#define DIO_MODM_IO0_DIO_11 11
#define DIO_MODM_IO1_DIO_12 12
#define DIO_SPI_CS_DIO_13 	13
#define DIO_SPI_CLK_DIO_14 	14

#define IPUP_CC2640_TEST_GET   2
#define IPUP_CC2640_TEST_SET   4
#define IPUP_CC2640_TEST_RSP   8

#define IPUP_PACKET_HEADER_SIZE		(IPUP_BYTE_PACKET_BITS0 + 1) //last byte of header
#define IPUP_PACKET_CRC_SIZE		2

//Byte positions of the IPup header
#define IPUP_BYTE_PACKET_MSG_CODE_POS		    5
#define IPUP_BYTE_PACKET_MSG_SUB_CODE_POS		6
#define IPUP_BYTE_PACKET_MSG_TYPE_POS    		7
#define IPUP_BYTE_PACKET_MSG_DATA_START_POS    	8

//u2Rev
#define IPUP_BYTE_PACKET_BITS1_REV_POS	6
#define IPUP_BYTE_PACKET_BITS1_REV_MASK		0xC0

//u2Type
#define IPUP_BYTE_PACKET_BITS1_TYPE_POS	4
#define IPUP_BYTE_PACKET_BITS1_TYPE_MASK		0x30

//u4TranCode
#define IPUP_BYTE_PACKET_BITS1_TRANCODE_POS	0
#define IPUP_BYTE_PACKET_BITS1_TRANCODE_MASK		0x0F

//u2Reserved
//#define IPUP_BYTE_PACKET_BITS2_XXX_POS	6
//#define IPUP_BYTE_PACKET_BITS2_XXX_MASK		0xC0

//u6Destination
#define IPUP_BYTE_PACKET_BITS_DEST_POS	0
#define IPUP_BYTE_PACKET_BITS2_DEST_MASK		0x3F


#define IPUP_START_SENTENEL	0x55	// 'U'

#define IPUP_MAX_PACKET_HEADER_LEN 5
#define IPUP_MAX_PACKET_MAX_PAYLOAD_LEN 256
#define IPUP_MAX_PACKET_LEN	(IPUP_MAX_PACKET_MAX_PAYLOAD_LEN + IPUP_MAX_PACKET_HEADER_LEN)


// MFG TEST CODES
#define IPUP_TYPE_CC2640_TEST_MAC_ADDR = 5


#define	mIpup_IncSsErrorCount()					\
{															\
	if(	gstrIpupDiagData.un0.strBits.u4SsErrorCount < 0xF)	\
		gstrIpupDiagData.un0.strBits.u4SsErrorCount++;			\
}

#define	mIpup_IncLenErrorCount()					\
{															\
	if(	gstrIpupDiagData.un0.strBits.u4LenErrorCount < 0xF)	\
		gstrIpupDiagData.un0.strBits.u4LenErrorCount++;			\
}

#define	mIpup_IncUnknownRevCount()					\
{															\
	if(	gstrIpupDiagData.un0.strBits.u2UnknownRevCount < 0x3)	\
		gstrIpupDiagData.un0.strBits.u2UnknownRevCount++;			\
}

#define	mIpup_IncZeroTrancodeErrorCount()					\
{															\
	if(	gstrIpupDiagData.un0.strBits.u4ZeroTrancodeErrorCount < 0xF)	\
		gstrIpupDiagData.un0.strBits.u4ZeroTrancodeErrorCount++;			\
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	structs:
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
typedef struct
{
	 uint8_t	u8MsgCode;
	void	(*fpMsgHandler) (strIpupHeader const *pstrHeader, uint8_t	const arru8Data[]);

}strIpupMsgTableEntry;



typedef struct 
{

	union
	{
		uint32_t u32Dummy32;
		struct
		{
			#ifdef _USES_LIITLE_ENDIAN
				unsigned	u4SsErrorCount:4;
				unsigned	u4LenErrorCount:4;

				unsigned	u2UnknownRevCount:2;
				unsigned	u2Reserved:2;
				unsigned	u4ZeroTrancodeErrorCount:4;

				unsigned	u4Reserved1:4;
				unsigned	u4Reserved2:4;

				unsigned	u4Reserved3:4;
				unsigned	u4Reserved4:4;
			#else
				unsigned	u4Reserved4:4;
				unsigned	Reserved3:4;

				unsigned	u4Reserved2:4;
				unsigned	u4Reserved1:4;

				unsigned	u4ZeroTrancodeErrorCount:4;
				unsigned	u2Reserved:2;
				unsigned	u2UnknownRevCount:2;

				unsigned	u4LenErrorCount:4;
				unsigned	u4SsErrorCount:4;
			#endif
			
		}strBits;
	}un0;

	
	uint8_t u8IpupRouteBleCount;
	uint8_t u8IpupRouteZipPortCount;
	uint16_t u16IpupRxCount;
	uint16_t u16IpupRxHandleCount;
	
	
}strIpupDiagData;

typedef struct 
{
	uint8_t u8IsProcessingStep;
	bool bIsErrorHandling;
	bool bGotLength;
	uint16_t u16BytesRemaining;
	uint16_t u16BytesProcessed;
	uint32_t u32LastByteTimestamp;
	uint8_t arru8RxBuf[IPUP_MAX_PACKET_LEN];
	
} strIpupRxBufHandler;



// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	Global Variables:
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
uint8_t myTest[25] = { 0 };
strIpupMsgTableEntry	arrstrIpupMsgHandlerTable[IPUP_MSG_IN_MAX_NUM_HANDLERS];
uint8_t	su8IpupNumRegisteredHandlers;

strIpupDiagData gstrIpupDiagData;

strIpupRxBufHandler garrstrIpupRxBufHandler;


static uint8_t VerisonIpup[25] = {0x55,0x00,0x12,0x00,0x01,0x05,0x01,0x08,
		                   	   	  0x31,0x32,0x33,0x34,0x35,0x36,0x34,0x35,0x36,0x34,0x35,0x36,0x34,0x35,0x36,
								  0x35,0x35};

static uint8_t MacAddrIpup[16] = {0x55,0x00,0x09,0x00,0x01,0x05,0x02,0x08,0x31,0x32,0x33,0x34,0x35,0x36,0x30,0x30};
static uint8_t GetDioIpup[12] = {0x55,0x00,0x05,0x00,0x01,0x09,0x01,0x08,0x03,0x01,0x30,0x30};
static uint8_t GetWakeUpIpup[11] =    {0x55,0x00,0x04,0x00,0x01,0x09,0x03,0x08,0x00,0x30,0x30};
static uint8_t ConnectStateIpup[11] = {0x55,0x00,0x04,0x00,0x01,0x07,0x01,0x08,0x31,0x30,0x30};
//------------------------------------------------------------------------------
// PROTOTYPES
//------------------------------------------------------------------------------
void ipupPacketTransmit( enumIpupPortType ePort, const uint16_t u16Len,  uint8_t	const arru8Packet[]);

// ***************************************************************************
// * Function:  ipupInit
// * Description: 
// *
// * Parameters:	
// *
// * Return Values:
// *            None.
// ***************************************************************************
void ipupInit( void )
{
	 
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupRegisterMsgHandler
//	 Description: 
//					
//	
//	 Parameters: 
//			
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ipupRegisterMsgHandler(uint8_t	u8MsgCode, void	(*fpMsgHandler) (strIpupHeader	const *pstrHeader, uint8_t	const arru8Packet[]))
{
	uint8_t u8i; 
	bool bFound = false;

	for (u8i = 0; u8i < IPUP_MSG_IN_MAX_NUM_HANDLERS; u8i++)
	{
		if (arrstrIpupMsgHandlerTable[u8i].u8MsgCode == u8MsgCode)
		{
			bFound = true;
			break;
		}
	}

	//If handler not currently registered of this u8MsgCode
	if (!bFound)
	{
		//IF table is not full
		if(su8IpupNumRegisteredHandlers < IPUP_MSG_IN_MAX_NUM_HANDLERS)
		{
			arrstrIpupMsgHandlerTable[su8IpupNumRegisteredHandlers].u8MsgCode = u8MsgCode;
			arrstrIpupMsgHandlerTable[su8IpupNumRegisteredHandlers].fpMsgHandler = fpMsgHandler;
			su8IpupNumRegisteredHandlers++;
		}
		else
		{
			//we need to change the table size
			// DP comment out code.
			//assert(0);
		}
	}
	else
	{
		arrstrIpupMsgHandlerTable[u8i].fpMsgHandler = fpMsgHandler;
	}
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketRxForMeHandler
//	 Description: 
//					
//	
//	 Parameters: pu8Packet	- packet pointer
//			
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ipupPacketRxForMeHandler(  strIpupHeader const *pstrHeader,  uint8_t	const arru8Packet[])
{
	int i;
	bool bIsFound = false;
	
	//FOR each handler in the table
	for(i=0; (i < su8IpupNumRegisteredHandlers) && !bIsFound; i++)
	{
		//IF this is a messaged for the handler
		if(arrstrIpupMsgHandlerTable[i].u8MsgCode == arru8Packet[IPUP_BYTE_PACKET_MSG_CODE_POS])
		{

			#ifdef _INCLUDE_IRF_TIMING_TEST /////////////
				idsPrintf1 ("HandlrStart(%u)", TaskGetTicks());
			#endif //_INCLUDE_IRF_TIMING_TEST ///////////

			if(arrstrIpupMsgHandlerTable[i].fpMsgHandler)
			{
				arrstrIpupMsgHandlerTable[i].fpMsgHandler(pstrHeader,
															  &arru8Packet[IPUP_PACKET_HEADER_SIZE+1]);
			}

			#ifdef _INCLUDE_IRF_TIMING_TEST /////////////
				idsPrintf1 ("HandlrDone(%u)", TaskGetTicks());
			#endif //_INCLUDE_IRF_TIMING_TEST ///////////

			bIsFound = true;
		}
	}	//for i

	if(!bIsFound)
	{

		#ifdef _IRF_PACKET_DEBUG_PRINTF
		idsPrintf4 ("iRFPcketHandleMsgArrived(%u),[%d] C:%d H:%d,", 		
			TaskGetTicks(), u8RxQueueIndex,
			arrstriRFPcketRxQueue[u8RxQueueIndex].strHeader.u8MsgCode,
			su8iRFPcketNumRegisteredHandlers
			);
		// DbgUartInsert(sarruTempBuffer, strlen(sarruTempBuffer), ENUM_DBGUART_DELIM_TYPE_CURLY);
	#endif
	}	
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketRxHandler
//	 Description: 
//					
//	
//	 Parameters: pu8Packet	- packet pointer
//			
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
bool ipupPacketRxHandler( uint16_t u16Len,  uint8_t	 arru8Packet[])
{
	bool bIsHandled = false;
	strIpupHeader strHeader;
	// uint8_t myvalue =0;
	uint8_t iCount;

	for (iCount = 0; iCount <15; iCount++){
	myTest[iCount] = arru8Packet[iCount];
   }
		// Allow overflow?
		gstrIpupDiagData.u16IpupRxCount++;

		strHeader.eDest  = (enumIpupEndpoint) (arru8Packet[IPUP_BYTE_PACKET_BITS0]);
		strHeader.u16Size = u16Len;

			//SWITCH destinatiion endpoint
			switch( strHeader.eDest ){
				//CASE For me (STM32)
				case IPUP_DESTINATION_TYPE_STM32_IDS_APP:
				{
					//Allow overflow?
					gstrIpupDiagData.u16IpupRxHandleCount++;
					bIsHandled = true;
					ipupPacketRxForMeHandler( &strHeader, arru8Packet);
				}
				break;

				//CASE For BLE
				//case IPUP_DESTINATION_TYPE_BLE_ENG_PHONE1:
				//case IPUP_DESTINATION_TYPE_BLE_ENG_PHONE2:
				//case IPUP_DESTINATION_TYPE_BLE_IDS_PHONE:
				//case IPUP_DESTINATION_TYPE_BLE_ZIP_PHONE:

				case IPUP_DESTINATION_TYPE_CC2640:
				{
					//Allow overflow?
					gstrIpupDiagData.u8IpupRouteBleCount++;

					//ipupPacketTransmit( IPUP_PORT_TYPE_BLE, u16Len, arru8Packet);
					//void ipupPacketTransmit( enumIpupPortType ePort, const uint16_t u16Len,  uint8_t	const arru8Packet[])
					bIsHandled = true;
				}
				break;

				case IPUP_DESTINATION_TYPE_CC2640_OPERATIONAL:
				{
					// repsond with MAC address or Version
					if(arru8Packet[IPUP_BYTE_PACKET_MSG_CODE_POS] == IPUP_CC2640_TEST_CODE){

						// GET is the only choice for this request.
						if(arru8Packet[IPUP_BYTE_PACKET_MSG_SUB_CODE_POS] == IPUP_CC2640_SUBCODE_MAC){

							ipupPacketTransmitMacAddr();
						}

						// GET is the only choice for this request.
						if(arru8Packet[IPUP_BYTE_PACKET_MSG_SUB_CODE_POS] == IPUP_CC2640_SUBCODE_VER){
						  ipupPacketTransmitVersion();
						}

					}

					// repsond with BLE connection status state
					if(arru8Packet[IPUP_BYTE_PACKET_MSG_CODE_POS] == IPUP_CC2640_CONNECT_STATUS_CODE){

						// GET is the only choice for this request.
						if(arru8Packet[IPUP_BYTE_PACKET_MSG_SUB_CODE_POS] == IPUP_CC2640_CONNECT_STATUS_SUBCODE_STATE){

							ipupPacketTransmitConnectState();
						}

					}


				}
				 break;  // end cc2640_OPS

				case IPUP_DESTINATION_TYPE_CC2640_TESTING:
				{
					// Digital IO related commands
					if(arru8Packet[IPUP_BYTE_PACKET_MSG_CODE_POS] == IPUP_CC2640_DIO_CODE){

						// IPUP_CC2640_SUBCODE_DIO_STATE 1
						if(arru8Packet[IPUP_BYTE_PACKET_MSG_SUB_CODE_POS] == IPUP_CC2640_SUBCODE_DIO_STATE){
							// handle GET/SET
							// IPUP_CC2640_TEST_GET   2
							if(arru8Packet[IPUP_BYTE_PACKET_MSG_TYPE_POS] == IPUP_CC2640_TEST_GET){
							  // get digital IO state and respond
								ipupPacketTransmitGPIOState(arru8Packet[IPUP_BYTE_PACKET_MSG_DATA_START_POS]);
							}

							// IPUP_CC2640_TEST_SET   4
							if(arru8Packet[IPUP_BYTE_PACKET_MSG_TYPE_POS] == IPUP_CC2640_TEST_SET){
							  // not available
							}
						}

						// IPUP_CC2640_SUBCODE_DIO_ARM_WAKEUP 3
						if(arru8Packet[IPUP_BYTE_PACKET_MSG_SUB_CODE_POS] == IPUP_CC2640_SUBCODE_DIO_ARM_WAKEUP){
							// handle SET
							// IPUP_CC2640_TEST_SET   4
							if(arru8Packet[IPUP_BYTE_PACKET_MSG_TYPE_POS] == IPUP_CC2640_TEST_SET){
								// Set the ARM WAKE UP PIN to pulse or steady state value
								SimpleBLECentral_SetArmWake(arru8Packet[IPUP_BYTE_PACKET_MSG_DATA_START_POS]);

								// respond to the command with the setting status
								ipupPacketTransmitARMWakeState();
							}

						}
					}
				}
				break;
				default:
					// DP comment out code.
					// assert(0);
					gstrIpupDiagData.u8IpupRouteZipPortCount++;
			}

			return bIsHandled;

}
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitConnectState
//	 Description:
//
//
//	 Parameters: void,  the function will get the connection state
//			     of the BLE and send it.
//
//
//	 Return Values:
//				None:
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ipupPacketTransmitConnectState( void)
{
	//When the BLE is started it reads the ownAddress

	// use 6 bytes of device address for 8 bytes of system ID value
	ConnectStateIpup[8] =  SimpleBLECentral_GetConnectState();

    // Apply the CRC16 and set last two bytes
    // Util_ADDCalculateCrc( MacAddrIpup, 16);

	//send the data via UART to STM32
	msgHandler_writeBleRxBufLocal(11, ConnectStateIpup);
	GattPortBLE_readCallBack();
}
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitMacAddr
//	 Description: 
//					
//	
//	 Parameters: void,  the function will get the request MAC address
//			     and send them in an IPUP response.
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

void ipupPacketTransmitMacAddr( void)
{
	//When the BLE is started it reads the ownAddress

	// use 6 bytes of device address for 8 bytes of system ID value
	MacAddrIpup[8] =  ownAddress[5];
	MacAddrIpup[9] =  ownAddress[4];
	MacAddrIpup[10] = ownAddress[3];
    MacAddrIpup[11] = ownAddress[2];
	MacAddrIpup[12] = ownAddress[1];
    MacAddrIpup[13] = ownAddress[0];

    // Apply the CRC16 and set last two bytes
    // Util_ADDCalculateCrc( MacAddrIpup, 16);

	//send the data via UART to STM32
	msgHandler_writeBleRxBufLocal(16, MacAddrIpup);
	GattPortBLE_readCallBack();
}
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitVersion
//	 Description:
//
//
//	 Parameters: void,  the function will get the request Version
//			     and send them in an IPUP response.
//
//
//	 Return Values:
//				None:
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ipupPacketTransmitVersion( void)
{
	//When the BLE is started it reads the ownAddress

	// use 6 bytes of device address for 8 bytes of system ID value
	SimpleBLECentral_AppendVersion(8, VerisonIpup);

    // Apply the CRC16 and set last two bytes
    //Util_ADDCalculateCrc( VerisonIpup, 16);

	//send the data via UART to STM32
	msgHandler_writeBleRxBufLocal(25, VerisonIpup);
	GattPortBLE_readCallBack();
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitARMWakeState
//	 Description:
//
//
//	 Parameters: void,  the function will get the current state of ARM WakeUp
//
//
//	 Return Values:
//				None:
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ipupPacketTransmitARMWakeState( void)
{
	//When the BLE is started it reads the ownAddress

	// set the current value of the wake up setting
	GetWakeUpIpup[8] =  SimpleBLECentral_GetArmWake();

    // Apply the CRC16 and set last two bytes
    // Util_ADDCalculateCrc( MacAddrIpup, 16);

	//send the data via UART to STM32
	msgHandler_writeBleRxBufLocal(11, GetWakeUpIpup);
	GattPortBLE_readCallBack();
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitGPIOState
//	 Description:
//
//
//	 Parameters: void,  the function will get the request Version
//			     and send them in an IPUP response.
//
//
//	 Return Values:
//				None:
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ipupPacketTransmitGPIOState( uint8_t u8MyDio)
{

	// msgHandler_RxBufClear();
	// When the BLE is started it reads the ownAddress
	GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS] = u8MyDio;
#ifdef MVAC_PCB_IO_TRUE
	// get the
	switch( u8MyDio){
		case 2:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_MCU_IO0);
			SimpleBLECentral_ConnectTestSequence(BLE_PROC_STEP_DISPLAY_DEVICE);
		   break;
		case 3:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_18V_PG);
			SimpleBLECentral_ConnectTestSequence(BLE_PROC_STEP_DISCOVER_DEVICE);
		   break;
		case 4:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_ARM_RESET);
			SimpleBLECentral_ConnectTestSequence(BLE_PROC_STEP_CONNECT_TO_DEVICE);
		   break;
		case 5:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = 2;
			SimpleBLECentral_ConnectTestSequence(BLE_PROC_STEP_CONNECT_UPDATE);
		   break;
		case 6:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = 2;
			SimpleBLECentral_ConnectTestSequence(BLE_PROC_STEP_START_RSSI_POLL);
		   break;
		case 7:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_SPI0_MISO);
		   break;
		case 8:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_SPI0_MOSI);
		   break;
		case 9:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getOutputValue(Board_ARM_WAKEUP);
		   break;
		case 10:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_BL_MODE);
		   break;
		case 11:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_MODEM_IO0);
		   break;
		case 12:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_MODEM_IO1);
		   break;
		case 13:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_SPI0_CSN);
		   break;
		case 14:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] = PIN_getInputValue(Board_SPI0_CLK);
		   break;

		default:
			GetDioIpup[IPUP_BYTE_PACKET_MSG_DATA_START_POS + 1] =  255; // error
		break;
	}
#endif
	// Apply the CRC16 and set last two bytes
	// Util_ADDCalculateCrc( VerisonIpup, 16);

	//send the data via UART to STM32
	msgHandler_writeBleRxBufLocal(12, GetDioIpup);
	GattPortBLE_readCallBack();
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupGetPortFromEndpoint
//	 Description: 
//					
//	
//	 Parameters: pu8Packet	- packet pointer
//			
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
enumIpupPortType ipupGetPortFromEndpoint( enumIpupEndpoint eEndpoint)
{
	
	 enumIpupPortType eRetVal = IPUP_PORT_TYPE_MAX;
		
		
		switch (eEndpoint)
		{
			case IPUP_DESTINATION_TYPE_STM32_IDS_APP:
			case IPUP_DESTINATION_TYPE_STM32_IDS_APP_COAP:
			case IPUP_DESTINATION_TYPE_STM32_OBD_APP:
			case IPUP_DESTINATION_TYPE_STM32_ZIP_APP:
				// DP comment out code.
				// assert(0);
				break;
		
			case IPUP_DESTINATION_TYPE_CC2640:
			case IPUP_DESTINATION_TYPE_HE910_APP_ZONE:
			case IPUP_DESTINATION_TYPE_BLE_ENG_PHONE1:
			case IPUP_DESTINATION_TYPE_BLE_ENG_PHONE2:
			case IPUP_DESTINATION_TYPE_BLE_IDS_PHONE:
			case IPUP_DESTINATION_TYPE_BLE_ZIP_PHONE:
				eRetVal = IPUP_PORT_TYPE_BLE;
				break;
			
			case IPUP_DESTINATION_TYPE_ZIP_PORT:
				eRetVal = IPUP_PORT_TYPE_ZIP_PORT;
				break;
			
			#ifdef _INCLUDE_IPUP_OVER_CELL_UART //////
			case IPUP_DESTINATION_TYPE_IDS_SERVER1:
			case IPUP_DESTINATION_TYPE_IDS_SERVER2:
			case IPUP_DESTINATION_TYPE_ZIP_SERVER1:
				eRetVal = IPUP_PORT_TYPE_CELLULAR;
				break;
			#endif //_INCLUDE_IPUP_OVER_CELL_UART /////
			
			default:
				// DP comment out code.
				//assert(0);
				break;
		}
		
	return eRetVal;
}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitEndpoint
//	 Description: 
//					
//	
//	 Parameters: pu8Packet	- packet pointer
//			
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void ipupPacketTransmitEndpoint( enumIpupEndpoint eEndpoint, const uint16_t u16Len,  uint8_t	const arru8Packet[])
{
	static uint8_t su8RecBuf2[256];
	
	enumIpupPortType ePort;
	
	//SDS_TODO: need better scheme
	//SDS_TODO:  protection
	memcpy ( &su8RecBuf2[IPUP_PACKET_HEADER_SIZE], arru8Packet, u16Len);
	
	su8RecBuf2[IPUP_BYTE_START_SENTENEL] = IPUP_START_SENTENEL;
	su8RecBuf2[IPUP_BYTE_PACKET_LEN1] = (uint8_t) ((u16Len & 0xFF00) >> 8);
	su8RecBuf2[IPUP_BYTE_PACKET_LEN0] = (uint8_t) ((u16Len & 0x00FF) >> 0);
	
	//SDS_TODO:  add other fields
	su8RecBuf2[IPUP_BYTE_PACKET_BITS0] = 0x00;
	su8RecBuf2[IPUP_BYTE_PACKET_BITS0] = (uint8_t) ((eEndpoint & IPUP_BYTE_PACKET_BITS2_DEST_MASK));
	
	su8RecBuf2[u16Len + IPUP_PACKET_HEADER_SIZE] = 0x0D;
	su8RecBuf2[u16Len + IPUP_PACKET_HEADER_SIZE + 1] = 0x0A;
	
	
	ePort = ipupGetPortFromEndpoint(eEndpoint);
	
	if( ePort < IPUP_PORT_TYPE_MAX)
	{
		//ipupPacketTransmit( ePort, u16Len + IPUP_PACKET_HEADER_SIZE + IPUP_PACKET_CRC_SIZE,  su8RecBuf2);
	}
}


// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  IpupResetPort
//	 Description: 
//					
//	
//	 Parameters: 
//			
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
void IpupResetPort(void)
{

garrstrIpupRxBufHandler.bIsErrorHandling = false;
garrstrIpupRxBufHandler.u8IsProcessingStep = IPUP_PROCESS_STEP_WAIT_START;
garrstrIpupRxBufHandler.u16BytesProcessed = 0;
garrstrIpupRxBufHandler.u16BytesRemaining = 0;

}

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  IpupUartRxProcessByte
//	 Description: 
//        SDS_TODO: replace this with IPUP packet mode. see: Task 379:Use UART IPUP 'Packet Mode' for BLE and Cell UARTs
//	
//	 Parameters: 
//			
//			
//	
//	 Return Values: 
//				None: 
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
bool IpupUartRxProcessBytes(sbpEvtRxUart_t *pMsg)
{
	uint16_t iCount = 0;

    //return value is true, if cmd is to pass to phone app.
	bool bRetVal = false;
	strIpupRxBufHandler * pstrRxBufHandler;
	
	pstrRxBufHandler = &garrstrIpupRxBufHandler;

	//cycle throught the received bytes.
	for (iCount = 0; iCount < pMsg->length; iCount++){

	switch(pstrRxBufHandler->u8IsProcessingStep){
	case IPUP_PROCESS_STEP_WAIT_START:

			if( pMsg->data[iCount] == IPUP_START_SENTENEL){ //got the start sentinel
				pstrRxBufHandler->u8IsProcessingStep = IPUP_PROCESS_STEP_GET_LEN1;
				pstrRxBufHandler->arru8RxBuf[pstrRxBufHandler->u16BytesProcessed] = pMsg->data[iCount];
				pstrRxBufHandler->u16BytesProcessed++;
				}
		break;

	case IPUP_PROCESS_STEP_GET_LEN1:
		    pstrRxBufHandler->u8IsProcessingStep = IPUP_PROCESS_STEP_GET_LEN2;
		    pstrRxBufHandler->arru8RxBuf[pstrRxBufHandler->u16BytesProcessed] = pMsg->data[iCount];
		    pstrRxBufHandler->u16BytesRemaining = (uint16_t) pMsg->data[iCount] << 8;

		    pstrRxBufHandler->u16BytesProcessed++;

			break;

	case IPUP_PROCESS_STEP_GET_LEN2:
		    pstrRxBufHandler->u8IsProcessingStep = IPUP_PROCESS_STEP_GET_PAYLOAD;
		    pstrRxBufHandler->arru8RxBuf[pstrRxBufHandler->u16BytesProcessed] = pMsg->data[iCount];
		    pstrRxBufHandler->u16BytesRemaining += (uint16_t) pMsg->data[iCount] + 4;

		    pstrRxBufHandler->u16BytesProcessed++;

			break;

	case IPUP_PROCESS_STEP_GET_PAYLOAD:

		   pstrRxBufHandler->arru8RxBuf[pstrRxBufHandler->u16BytesProcessed] = pMsg->data[iCount];
		   pstrRxBufHandler->u16BytesProcessed++;

		   if(pstrRxBufHandler->u16BytesProcessed == pstrRxBufHandler->u16BytesRemaining + 1){
			   pstrRxBufHandler->u8IsProcessingStep = IPUP_PROCESS_STEP_CRC;
		   }
		   break;

	case IPUP_PROCESS_STEP_CRC:

		    //Process the message, all data has been recieved.
		    ipupPacketRxHandler( pstrRxBufHandler->u16BytesRemaining, pstrRxBufHandler->arru8RxBuf );

		    iCount = pMsg->length;
            //clear buffer and state machine for next reading
			IpupResetPort();
			break;

	default:
	{
		//clear buffer and state machine for next reading
		IpupResetPort();
	}
	break;
	}

	}//end of for loop cycling through the data

	return bRetVal;
}

#endif // _INCLUDE_MODULE_IPUP
#endif //_IPUP_C


