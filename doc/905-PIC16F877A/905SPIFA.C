#include "pic.h"

#define uchar unsigned char
#define uint unsigned int
uchar temp[32];
///////////////////////////模式控制定义/////////////////////////////////
#define TX_EN	RA0 	//0r
#define TRX_CE  RA1	//0r
#define PWR     RA2	//0r
///////////////////////////SPI口定义////////////////////////////////////
#define  MISO	RC4   	//r主入从出	1
#define  MOSI   RC5	//r主出从入	0
#define  SCK    RC3	//r主出，从入    0
////////////////////////////////////////////////////////////////////////
#define  CSN    RE2
///////////////////////////状态输出口///////////////////////////////////
#define  CD     RA3	//r1
#define  AM     RA4	//r1
#define  DR     RA5	//r1
////////////////////////////905-SPI指令/////////////////////////////////
#define WC	0x00
#define RRC	0x10
#define WTP	0x20
#define RTP	0x21
#define WTA	0x22
#define RTA	0x23
#define RRP	0x24
////////////////////////////////////////////////////////////////////////////
void Delay(uchar x);
void Spi_initial();
uchar SpiReadSpiWrite(uchar DATA);
void system_Init(void);
void Config905(void);
void TxPacket(void);
void SetTxMode(void);
unsigned char CheckCD(void);
void TX(void);
void StartUART(void);
void R_S_Byte(uchar R_Byte);
/////////////////////////////////////////////////////////////////////////////
uchar TxRxBuf[32]=			     //发送数据
{
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
0x17,0x18,0x19,0x20,0x21,0x22,0x23,0x24,
0x25,0x26,0x27,0x28,0x29,0x30,0x31,0x32,
};
uchar TxAddress[4]={0xcc,0xcc,0xcc,0xcc };    //接收地址
////////////////////////////////////////////////////////////////////////////
uchar RFConf[11]=
{
  0x00,                             //配置命令//
  0x4c,                             //CH_NO,配置频段在423MHZ
  0x2C,                             //输出功率为10db,不重发，节电为正常模式
  0x44,                             //地址宽度设置，为4字节
  0x20,0x20,                        //接收发送有效数据长度为32字节
  0xCC,0xCC,0xCC,0xCC,              //接收地址
  0x58,                              //CRC充许，8位CRC校验，外部时钟信号不使能，16M晶振
};
////////////////////////////////////////////////////////////////////////////
void Delay(uchar x)               //1ms延时
{
	uchar i;
	while(x--)
	for(i=0;i<80;i++);
}
////////////////////////////////////////////////////////////////////////////
void Spi_initial()
{
	TRISC=TRISC&0xDF;	//r设置SDO（RC5）为输出**0*****
	TRISC=TRISC&0xF7;	//r设置SCK（RC3）为输出****0***
	TRISC=TRISC|0x10;	//r设置SDI（RC4）为输入***1****   
	INTCON=0x00;		//r关闭所有中断
	PIR1=0x00;		//r
	SSPCON=0x20;		//r写入时没有冲突发生，接收时没有溢出，开启SPI功能，空闲时处于低电平，fosc/4
	SSPSTAT=0xc0;		//r上升沿时发送，
		ADCON1=0x07;
	SSPIF=0;		//rPIR1=PIR1|0x08 ;
}
////////////////////////////////////////////////////////////////////////////	
uchar SpiReadSpiWrite(uchar DATA)//r
{
	uchar b;
	Spi_initial();
	SSPBUF=DATA;	 	//写入要发送数据发送的同时也得到数据
	do
	{
	;
	}
	while(SSPIF==0);
	SSPIF=0;	
	b=SSPBUF;		//把得到数据保存
	return b;
	}
////////////////////////////////////////////////////////////////////////////
///////////////////////初始化寄存器////////////////////////////////////////
void system_Init(void)			//r
{
   	TRISA=TRISA&0xF8;		//r
	TRISA=TRISA|0x38; 		//r
	TRISE=TRISE&0xFB; 		//r*****0**
	TRISC=0x90; 	
	ADCON1=0x07;		//
   	CSN=1;				// Spi 	disable
	SCK=0;				// Spi clock line init low
	DR=0;				// Init DR for input
	AM=0;				// Init AM for input
	CD=0;				// Init CD for input
	PWR=1;				// nRF905 power on
	TRX_CE=0;			// Set nRF905 in standby mode
	TX_EN=0;			// set radio in Rx mode
}
///////////////////////初始化寄存器////////////////////////////////////////
void Config905(void)
{
	uchar i;
	CSN=0;	
	Delay(1);				// Spi enable for write a spi command
	//SpiWrite(WC);				// Write config command写放配置命令
	for (i=0;i<11;i++)			// Write configration words  写放配置字
	{
	 SpiReadSpiWrite(RFConf[i]);
	}
	CSN=1;					// Disable Spi
}
///////////////////////////////////////////////////////////////////
void TxPacket(void)
{
	uchar i;
	CSN=0;
	Delay(1);
	SpiReadSpiWrite(WTP);			// Write payload command
	for (i=0;i<32;i++)
	{
	SpiReadSpiWrite(TxRxBuf[i]);		// Write 32 bytes Tx data
	}					// Spi enable for write a spi command
	CSN=1;
	Delay(1);				// Spi disable
	CSN=0;					// Spi enable for write a spi command
	SpiReadSpiWrite(WTA);				// Write address command
	for (i=0;i<4;i++)			// Write 4 bytes address
	{
	SpiReadSpiWrite(TxAddress[i]);
	}
	CSN=1;					// Spi disable
	TRX_CE=1;				// Set TRX_CE high,start Tx data transmission
	Delay(1);				// while (DR!=1);
	TRX_CE=0;				// Set TRX_CE low
}
///////////////////////////////////////////////////////////////////
void SetTxMode(void)
{
	TRX_CE=0;				//
	TX_EN=1;				//发送使能
	Delay(1); 				// delay for mode change(>=650us)
}
///////////////////////////////////////////////////////////////////
void TX(void)
{
        SetTxMode();			// Set nRF905 in Tx mode
	TxPacket();			// Send data by nRF905
}
///////////////////////////////////////////////////////////////////
void StartUART(void)
{
	TXSTA=0x04;		//r设定TXSTA寄存器。同步从模式，8位数据，数据发送禁止，
	SPBRG=12; 		//r6MHZ, BRGH=1, 9600BPS.
	PIE1=0; 		//r清除PIE1所有中断使能。
	RCSTA=0x80; 		//r禁止接收，打开通讯端口
	TRISC=TRISC|0X80;  	//r将RC7(RX)设置为输入方式 
	TRISC=TRISC&0XBF;  	//rRC6(TX)设置为输出 
	TXEN=1;			//r启动发送，TSR开始移位操作。
}
void R_S_Byte(uchar R_Byte)
{	
     	TXREG= R_Byte;  
}
main()
{
	StartUART();			//串口初始化
	system_Init();			//系统初始化
	Config905();			//写入配置字，p-t-r 100
	while(1)
	{
	TX();
/*	CSN=0;                  	//SPI使能
	Delay(1);
	SpiReadSpiWrite(RTA);   	//写入控制字 	
	for(j=0;j<4;j++)		//读出配置字
	{
	temp[j]=SpiReadSpiWrite(0);
 	}
 	for(j=0;j<4;j++)
	{
 	R_S_Byte(temp[j]);		//串口显示
 	Delay(1000);
	}
	CSN=1;				//禁止SPI
	Delay(10);
	//R_S_Byte(temp[j]);*/
	}
	
}