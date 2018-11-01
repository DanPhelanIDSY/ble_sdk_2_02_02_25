/** @file ipup.h
 *  @brief This module is to handle IPUP (inter-Processor UART Protocol)
 *
 *
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

//_INCLUDE_MODULE_IPUP

#ifndef _IPUP_H                     // ensure unique inclusion
#define _IPUP_H                     // module identifier

//------------------------------------------------------------------------------
// header files
//------------------------------------------------------------------------------
#include "simpleBLECentral.h"
extern uint8_t ownAddress[6];

#define _INCLUDE_MODULE_IPUP
#ifdef _INCLUDE_MODULE_IPUP		//include module ?


//------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------

//See: MVAC3(MD020301-InterProcessorUartProtocol IPUP)b.xlsx!.enumDestination

typedef enum		
{		
	//reserved	 //0

	//Start 'STM32' endpoints
	IPUP_DESTINATION_TYPE_STM32_IDS_APP = 1,	 //1	//non-CoAP messages to IDS_APP
	IPUP_DESTINATION_TYPE_STM32_IDS_APP_COAP,	//2		//CoAP messages to IDS_APP
	IPUP_DESTINATION_TYPE_STM32_OBD_APP,	 //3		//messages to OBD_APP
	IPUP_DESTINATION_TYPE_STM32_ZIP_APP,	 //4		//messages to ZIP_APP
	
	//Start 'connected' endpoints (inside MVAC)
	IPUP_DESTINATION_TYPE_CC2640_OPERATIONAL = 5,	  //5	//messages to CC2640 operational data
	IPUP_DESTINATION_TYPE_CC2640_TESTING = 7, //7 hardware testing related commands
	IPUP_DESTINATION_TYPE_CC2640 = 8,	      //8		//messages to CC2640
	IPUP_DESTINATION_TYPE_HE910_APP_ZONE,	  //9		//messages to HE/LE910 (cell modem)

	//Start 'local' endpoints (BLE)
	IPUP_DESTINATION_TYPE_BLE_ENG_PHONE1 = 16,	 //16	//messages to BLE engineering connected phone app 1
	IPUP_DESTINATION_TYPE_BLE_ENG_PHONE2,	 //17	//messages to BLE engineering connected phone app 2
	IPUP_DESTINATION_TYPE_BLE_IDS_PHONE,	 //18	//messages to BLE customer connected phone app 1
	IPUP_DESTINATION_TYPE_BLE_ZIP_PHONE,	 //19	//messages to BLE customer connected phone app 2
	
	//Start 'local' endpoints (RS232)
	IPUP_DESTINATION_TYPE_ZIP_PORT = 24,	//24	//messages to ZipPort (RS232)
		
	//Start 'remote' endpoints
	IPUP_DESTINATION_TYPE_IDS_SERVER1 = 32,	 //32	//messages to ID Systems server 1
	IPUP_DESTINATION_TYPE_IDS_SERVER2,	 //33	//messages to ID Systems server 2
	IPUP_DESTINATION_TYPE_ZIP_SERVER1,	 //34	//messages to Zip Car server 1

} enumIpupEndpoint;		



typedef enum		
{		
	//reserved	 //0

	//Start 'STM32' endpoints
	IPUP_PACKET_TYPE_DATA = 0,	 //
	IPUP_PACKET_TYPE_ACK,	 //
	IPUP_PACKET_TYPE_NAK,	 //
	IPUP_PACKET_TYPE_REQ,	 //

} enumIpupPacketType;		

typedef enum		
{		
	//reserved	 //0

	//Start 'STM32' endpoints
	IPUP_PORT_TYPE_BLE = 1,	 //
	IPUP_PORT_TYPE_ZIP_PORT,	 //
	IPUP_PORT_TYPE_STM32,	 //		using binary protocol
	
	IPUP_PORT_TYPE_MAX
	
} enumIpupPortType;		

//------------------------------------------------------------------------------
// Struct
//------------------------------------------------------------------------------

typedef struct 
{
	enumIpupEndpoint	 eDest;
	enumIpupPacketType	eType;
	uint16_t	u16Size;
	uint8_t u8MsgCode;
} strIpupHeader;

//------------------------------------------------------------------------------
// PROTOTYPES
//------------------------------------------------------------------------------

// ***************************************************************************
// * Function:  ipupInit
// * Description: 
// *
// * Parameters:	
// *
// * Return Values:
// *            None.
// ***************************************************************************
void ipupInit( void );


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
void ipupRegisterMsgHandler(uint8_t	u8MsgCode, void	(*fpMsgHandler) (strIpupHeader	const *pstrHeader, uint8_t	const arru8Packet[]));



// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitPort
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
void ipupPacketTransmitPort( enumIpupPortType ePort, enumIpupEndpoint eEndpoint, const uint16_t u16Len,  uint8_t	const arru8Packet[]);

// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  ipupPacketTransmitEndpoint
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
void ipupPacketTransmitEndpoint( enumIpupEndpoint eEndpoint, const uint16_t u16Len,  uint8_t	const arru8Packet[]);

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
bool ipupPacketRxHandler( uint16_t u16Len,  uint8_t arru8Packet[]);

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
void IpupResetPort(void);

void ipupPacketTransmitConnectState( void);

void ipupPacketTransmitMacAddr( void);

void ipupPacketTransmitVersion( void);

void ipupPacketTransmitARMWakeState( void);

void ipupPacketTransmitGPIOState( uint8_t  u8MyDio);
// _____________________________________________________________________________
// ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
//	 Function:  IpupUartRxProcessBytes
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
bool IpupUartRxProcessBytes(sbpEvtRxUart_t *pMsg);


#endif // _INCLUDE_MODULE_IPUP
#endif //_IPUP_H

