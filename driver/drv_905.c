/**
 * @file drv_905.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "includes.h"

uint8_t (*_905ConfigFn[10])(unsigned char cmd, unsigned char data);

void map905ConfigFns(void) {
    _905ConfigFn[_CONF_CENTER_FREQ] =
	_905ConfigFn[_CONF_FREQ_BAND] =
	_905ConfigFn[_CONF_POWER_MODE] =
	_905ConfigFn[_CONF_RETRAN_MODE] =
	_905ConfigFn[_CONF_RX_ADDR_WIDTH] =
	_905ConfigFn[_CONF_TX_ADDR_WIDTH] =
	_905ConfigFn[_CONF_RX_PAYLOAD_WIDTH]  =
	_905ConfigFn[_CONF_TX_PAYLOAD_WIDTH] =
	_905ConfigFn[_CONF_RX_ADDR_ID] =
	_905ConfigFn[_CONF_OP_CLK_FREQ] =
	_905ConfigFn[_CONF_OP_CLK_EN] =
}

uint8_t (*_905OperateFn[10])(void);


uint8_t _905TxPacket(void)
{
	uchar i;
	//Config905();
	CSN=0;						// Spi enable for write a spi command
	SpiWrite(WTP);				// Write payload command
	for (i=0;i<32;i++)
	{
		SpiWrite(TxBuf[i]);		// Write 32 bytes Tx data
	}
	CSN=1;						// Spi disable
	Delay(1);
	CSN=0;						// Spi enable for write a spi command
	SpiWrite(WTA);				// Write address command
	for (i=0;i<4;i++)			// Write 4 bytes address
	{
		SpiWrite(RxTxConf.buf[i+5]);
	}
	CSN=1;						// Spi disable
	TRX_CE=1;					// Set TRX_CE high,start Tx data transmission
	Delay(1);					// while (DR!=1);
	TRX_CE=0;					// Set TRX_CE low
}

uint8_t _905RxPacket(void)
{
	uchar i;
	TRX_CE=0;					// Set nRF905 in standby mode
	CSN=0;						// Spi enable for write a spi command
	SpiWrite(RRP);				// Read payload command
	for (i=0;i<32;i++)
	{
		RxBuf[i]=SpiRead();		// Read data and save to buffer
	}
	CSN=1;						// Disable spi
	while(DR||AM);
//	P0=	RxBuf[0];				// Buffer[0] output from P0 port...
	TRX_CE=1;
	//xx=(RxBuf[0]>>4)&0x0f;
	//TxData(xx);
	 //=====================
	   if(RxBuf[0] == 1) LED1 = 0 ;
	   if(RxBuf[1] == 1) LED2 = 0 ;

	Delay(500);					// ...light led
//	P0=0xff;					// Close led
      LED1 = 1 ;
	  LED2 = 1 ;
}

void map905OperateFns(void) {
    _905OperateFn[_NO_OP] = _905NoOps;
    _905OperateFn[_TX_PACKET] = _905TxPacket;
    _905OperateFn[_RX_PACKET] = _905RXPacket;

}

void setTxMode(void)
{
	TX_EN=1;
	TRX_CE=0;
    // delay
}

void setRxMode(void)
{
	TX_EN=0;
	TRX_CE=1;
    // delay.
}
