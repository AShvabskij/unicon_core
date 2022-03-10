#include "MCP2515.h"


//#include "mcp2515.h"
/*RXB0's data registers:*/
unsigned char RXB0D[8] = { RXB0D0,RXB0D1,RXB0D2,RXB0D3,RXB0D4,RXB0D5,RXB0D6,RXB0D7 };
/*TXB0's data registers:*/
unsigned char TXB0D[8] = { TXB0D0,TXB0D1,TXB0D2,TXB0D3,TXB0D4,TXB0D5,TXB0D6,TXB0D7 };

void MCP2515_Init(void)
{
	unsigned char dummy;
	/*First initialize the SPI periphere*/
	/*Then initialize the MCP2515.Step as follow:
	1>Get into configuration mode by reset MCP2515 or write the bit of CANCTRL.REQOP.
	2>Check the CANSTAT by ReadByte_MCP2515(CANSTAT) if MCP2515 is already into configuration mode.
	3>Configurate the registers to set the baudrate:CN1,CN2,CN3.
	4>Configurate TXRTSCTRL to set the function of TXnRTS pin.
	5>Configurate TRXBnCTRL to set the pirority of transmit mailbox:TXB0,TXB1,TXB2.
	6>Configurate TXBn'SID ,EID and DLC.This step can configurate when you are in normal mode.
	7>Configurate RXFnSIDH and RXFnSIDL to resceive the specific ID.
	8>Configurate RXMnSIDH and RXMnSIDL to mask the RXFnSID's ID.
	9>Configurate CANINTE to enable or disable the interrupts.
	10>Return to the normal mode and double check if it is return to the normal mode by CANSTAT.
	*/

	/*Flip into the Configuration Mode*/
	MCP2515_Reset();
	_delay_(0x4000);/* 1ms _delay_ using Fcpu = 16Mhz*/

	//Set the baudrate
	//set CNF1,SJW=00,lengthe is 1TQ,BRP=49,TQ=[2*(BRP+1)]/Fsoc=2*50/8M=12.5us
	SendByte_MCP2515(CNF1, CAN_125Kbps);
	//set CNF2,SAM=0,caputre one time,PHSEG1=(2+1)TQ=3TQ,PRSEG=(0+1)TQ=1TQ
	SendByte_MCP2515(CNF2, 0x80 | PHSEG1_3TQ | PRSEG_1TQ);
	//set CNF3,PHSEG2=(2+1)TQ=3TQ,when CANCTRL.CLKEN=1,set CLKOUT pin to output
	SendByte_MCP2515(CNF3, PHSEG2_3TQ);
	/*
	3 Transimit Buffers:TXB0,TXB1,TXB2
	2 Receive   Buffers:RXB0,RXB1
	6 ID Filter:RXF0~RXF5
	2 ID Mask Regieter:RXM0,RXM1
	*/
	//configurate RXB0' registers
//     SendByte_MCP2515(RXB0CTRL,0x60);//Receive all frames from CAN bus
	SendByte_MCP2515(RXB0CTRL, 0x20);//RXB0 just receive the standard frames 
	SendByte_MCP2515(RXF0SIDH, 0xFF);//Fileter register
	SendByte_MCP2515(RXF0SIDL, 0xE0);//Just receive ID=0x7FF frame
	SendByte_MCP2515(RXF1SIDH, 0xFD);//Fileter register
	SendByte_MCP2515(RXF1SIDL, 0xC0);//Just receive ID=0x7EE frame
	SendByte_MCP2515(RXM0SIDH, 0xFF);//Mask register
	SendByte_MCP2515(RXM0SIDL, 0xE0);
	//Configurate TXB0's ID and DLC registers
	SendByte_MCP2515(TXB0SIDH, 0xFF);//Standard ID
	SendByte_MCP2515(TXB0SIDL, 0xE0);//Standard ID
	SendByte_MCP2515(TXB0DLC, DLC_1);//DLC
	 /*Set the RXB0 or RXB1 interrupt enableling*/
//    SendByte_MCP2515(CANINTE,RX0IE);//Enable RXB0 interrupt

	SendByte_MCP2515(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED);//Go into the normal mode
	dummy = ReadByte_MCP2515(CANSTAT);
	if (OPMODE_NORMAL != (dummy & 0xE0))
		SendByte_MCP2515(CANCTRL, REQOP_NORMAL | CLKOUT_ENABLED);//If normal ok       
}

/**
  * @brief  Reset the MCP2515 then U go into the configruation mode
  * @param  none
  * @retval none
  */
void MCP2515_Reset(void)
{
	CS_LOW();
	SPI_Send(SPI_RESET);
	CS_HIGH();
}

/**
  * @brief  Modify the bit of the register
  * @param  Add:Address of register
  * @param  Mask:1-->U can modify the corresponding bit;0-->U can not modify the corresponding bit
  * @param  Data:the data U need to be sent from MCU to MCP2515
  * @retval none
  */
void MCP2515_BitModify(unsigned char Addr, unsigned char Mask, unsigned char Data)
{
	CS_LOW();
	SPI_Send(SPI_BIT_MODIFY);
	SPI_Send(Addr);
	SPI_Send(Mask);
	SPI_Send(Data);
	CS_HIGH();
}

/**
  * @brief  Send a byte to MCP2515 through SPI to return corresponding status you need
  * @param  Add:Address of register
  * @param  Data:the data U need to be sent from MCU to MCP2515
  * @retval none
  */
void SendByte_MCP2515(unsigned char Addr, unsigned char Data)
{
	CS_LOW();
	SPI_Send(SPI_WRITE);
	SPI_Send(Addr);
	SPI_Send(Data);
	CS_HIGH();
}

/**
  * @brief  Send a command to MCP2515 through SPI to return corresponding status you need
  * @param  CMD:command
  * @retval Data:Data from MCP2515'SO pin
  */
unsigned char ReadByte_MCP2515(unsigned char Addr)
{
	unsigned char Data;
	CS_LOW();
	SPI_Send(SPI_READ);
	SPI_Send(Addr);
	Data = SPI_Receive();
	CS_HIGH();
	return Data;
}

/**
  * @brief  Send a command to MCP2515 through SPI to return corresponding status you need
  * @param  CMD:
				  SPI_RX_STATUS;
				SPI_READ_STATUS;
				SPI_READ_RX;
  * @retval Data:the status you need
  */
unsigned char SPI_CMD_MCP2515(unsigned char CMD)
{
	unsigned char Data;
	CS_LOW();
	SPI_Send(CMD);
	Data = SPI_Receive();
	CS_HIGH();
	return Data;
}

/**
  * @brief  Read 8 bytes to CAN bus
  * @param  CAN_TX_Buf:the address of data'indicator,8bytes
  * @retval none
  */
void CAN_Send8bytes(unsigned char *CAN_TX_Buf)
{
	unsigned char tempdata;
	tempdata = ReadByte_MCP2515(SPI_READ_STATUS);
	SendByte_MCP2515(TXB0D0, CAN_TX_Buf[0]);
	SendByte_MCP2515(TXB0D1, CAN_TX_Buf[1]);
	SendByte_MCP2515(TXB0D2, CAN_TX_Buf[2]);
	SendByte_MCP2515(TXB0D3, CAN_TX_Buf[3]);
	SendByte_MCP2515(TXB0D4, CAN_TX_Buf[4]);
	SendByte_MCP2515(TXB0D5, CAN_TX_Buf[5]);
	SendByte_MCP2515(TXB0D6, CAN_TX_Buf[6]);
	SendByte_MCP2515(TXB0D7, CAN_TX_Buf[7]);
	if (tempdata & 0x04)//check the TXREQ bit
	{
		_delay_(0x4000);
		MCP2515_BitModify(TXB0CTRL, 0x80, 0x00);//clear TXREQ bit
		while (SPI_CMD_MCP2515(SPI_READ_STATUS) & 0x04);//wait until TXREQ bit matchs 0
	}
	CS_LOW();
	SPI_Send(SPI_RTS_TXB0);//TXB0 send-request
	CS_HIGH();
}

/**
  * @brief  Send a byte to CAN bus by TXB0
  * @param  Data:data to be sent
  * @retval none
  */
void CAN_SendOneByte(unsigned char Data)
{
	unsigned char tempdata;
	/*Test the driver between MCP2515 and the SPI periphere :tempdata<--------------*/
	tempdata = SPI_CMD_MCP2515(SPI_READ_STATUS);
	if (tempdata & 0x04)//check the TXREQ bit
	{
		_delay_(0x4000);
		MCP2515_BitModify(TXB0CTRL, 0x08, 0x00);//clear TXREQ bit
		while (SPI_CMD_MCP2515(SPI_READ_STATUS) & 0x04);//wait until TXREQ bit matchs 0
	}
	SendByte_MCP2515(TXB0D0, Data);
	CS_LOW();
	SPI_Send(SPI_RTS_TXB0);//TXB0 send-request
	CS_HIGH();
}

/**
  * @brief  Receive a byte from MCP2515 RXB0
  * @param  none
  * @retval Data:return the effectively data from RXB0
  */
unsigned char CAN_Receive_onebyte()
{
	unsigned char tempdata;
	tempdata = SPI_CMD_MCP2515(SPI_READ_STATUS);//confirm receive data=RXB0IF
	if (tempdata & 0x01)
	{
		tempdata = SPI_CMD_MCP2515(SPI_RX_STATUS);
		if (tempdata & 0x40)//true means RXB0's standard frame
		{
			tempdata = ReadByte_MCP2515(RXB0D0);
		}
		else
		{
			tempdata = 254;
		}//no standard frame receive
		MCP2515_BitModify(CANINTF, 0x01, 0x00);
	}
	else
	{
		tempdata = 255;
	}//no frame receive
	return tempdata;
}

/**
  * @brief   Send n bytes with a given standard ID  corresponding to frame type
  * @param   CAN_TX_Buf: data to be sent
  * @param     DLC:DLC<=8
  * @param   SID:<=0x7FF
  * @param   CAN_FrameType:CAN_STD,CAN_RTR
  * @retval None
  */
void CAN_SendData(unsigned char *CAN_TX_Buf, Frame_TypeDef *Frame)
{
	unsigned char tempdata;
	unsigned char HSID, LSID;
	if (Frame->Type == CAN_STD)
	{
		/*Set the ID of the frame*/
		HSID = (unsigned char)(Frame->SID >> 3);
		LSID = (unsigned char)((Frame->SID << 5) & 0xE0);
		SendByte_MCP2515(TXB0SIDH, HSID);
		SendByte_MCP2515(TXB0SIDL, LSID);
		/*Set the DLC and the type of the frame*/
		SendByte_MCP2515(TXB0DLC, Frame->DLC | CAN_STD);
		/*Write the data into the TXB0 data registers */
		for (tempdata = 0; tempdata < Frame->DLC; tempdata++)
			SendByte_MCP2515(TXB0D[tempdata], CAN_TX_Buf[tempdata]);
		/*        SendByte_MCP2515(TXB0D0,CAN_TX_Buf[0]);
				SendByte_MCP2515(TXB0D1,CAN_TX_Buf[1]);
				SendByte_MCP2515(TXB0D2,CAN_TX_Buf[2]);
				SendByte_MCP2515(TXB0D3,CAN_TX_Buf[3]);
				SendByte_MCP2515(TXB0D4,CAN_TX_Buf[4]);
				SendByte_MCP2515(TXB0D5,CAN_TX_Buf[5]);
				SendByte_MCP2515(TXB0D6,CAN_TX_Buf[6]);
				SendByte_MCP2515(TXB0D7,CAN_TX_Buf[7]);*/
	}
	else  /*if(CAN_FrameType==CAN_RTR)*/
	{
		/*Set the ID of the frame*/
		HSID = (unsigned char)(Frame->SID >> 3);
		LSID = (unsigned char)((Frame->SID << 5) & 0xE0);
		SendByte_MCP2515(TXB0SIDH, HSID);
		SendByte_MCP2515(TXB0SIDL, LSID);
		/*Set the type of the frame*/
		SendByte_MCP2515(TXB0DLC, CAN_RTR);
	}

	tempdata = SPI_CMD_MCP2515(SPI_READ_STATUS);
	if (tempdata & 0x04)
	{
		_delay_(0x4000);
		MCP2515_BitModify(TXB0CTRL, 0x80, 0x00);
		while (SPI_CMD_MCP2515(SPI_READ_STATUS) & 0x04);
	}
	/*Send the SPI_RTS_TXB0 request command to MCP2515 to send the data loaded in the data register*/
	CS_LOW();
	SPI_Send(SPI_RTS_TXB0);
	CS_HIGH();
}

/**
  * @brief  Receive n bytes from MCP2515 RXB0
  * @param  none
  * @retval Data:return the effectively data from RXB0
  */
unsigned char CAN_ReceiveData(unsigned char *CAN_RX_Buf, Frame_TypeDef *Frame)
{
	unsigned char tempdata;
	unsigned int CAN_ID;
	//    tempdata=SPI_CMD_MCP2515(SPI_READ_STATUS);//check if it is received a  frame
	tempdata = SPI_CMD_MCP2515(SPI_RX_STATUS);

	if (tempdata & 0x40)//RXB0
	{
		CAN_ID = (unsigned int)((ReadByte_MCP2515(RXB0SIDH)) << 8);
		CAN_ID |= (unsigned int)(ReadByte_MCP2515(RXB0SIDL));
		CAN_ID >>= 5;
		/*Check the CAN_ID you received that if it is matched with the SID you wish*/
		if (CAN_ID == Frame->SID)
		{
			//            _delay_(0x4000);
			for (tempdata = 0; tempdata < Frame->DLC; tempdata++)
				CAN_RX_Buf[tempdata] = ReadByte_MCP2515(RXB0D[tempdata]);
			/*            CAN_RX_Buf[0]=ReadByte_MCP2515(RXB0D0);
						CAN_RX_Buf[1]=ReadByte_MCP2515(RXB0D1);
						CAN_RX_Buf[2]=ReadByte_MCP2515(RXB0D2);
						CAN_RX_Buf[3]=ReadByte_MCP2515(RXB0D3);
						CAN_RX_Buf[4]=ReadByte_MCP2515(RXB0D4);
						CAN_RX_Buf[5]=ReadByte_MCP2515(RXB0D5);
						CAN_RX_Buf[6]=ReadByte_MCP2515(RXB0D6);
						CAN_RX_Buf[7]=ReadByte_MCP2515(RXB0D7);*/
			MCP2515_BitModify(CANINTF, 0x01, 0x00);
		}
		else
		{
			return 0;//ID is not correct
		}
		return 1;//receive ok
	}
	else
		return 2;//no standard frame receive
}


