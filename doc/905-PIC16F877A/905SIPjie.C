#include "pic.h"
//////////////////////////////////
#define uchar unsigned char
#define uint unsigned int
///////////////////////////模式控制定义/////////////////////////////////
#define TX_EN	RA0 			//0r
#define TRX_CE  RA1			//0r
#define PWR     RA2			//0r
///////////////////////////SPI口定义////////////////////////////////////
#define  MISO	RC4   			//r主入从出	1
#define  MOSI   RC5			//r主出从入	0
#define  SCK    RC3			//r主出，从入   0
////////////////////////////////////////////////////////////////////////
#define  CSN    RE2
///////////////////////////状态输出口///////////////////////////////////
#define  CD     RA3			//r1
#define  AM     RA4			//r1
#define  DR     RA5			//r1
////////////////////////////905-SPI指令/////////////////////////////////
#define WC	0x00
#define RRC	0x10
#define WTP	0x20
#define RTP	0x21
#define WTA	0x22
#define RTA	0x23
#define RRP	0x24
////////////////////////////////////////////////////
void Delay(uchar n);
void Spi_initial();
uchar SpiReadSpiWrite(uchar DATA);
void nRF905Init(void);
void Config905(void);
void SetRxMode(void);
uchar CheckDR(void);
void RxPacket(void);
void RX(void);
void StartUART( void );
void R_S_Byte(uchar R_Byte);
//RF寄存器配置//
uchar RFConf[11]=
{
  0x00,                           	 //配置命令//
  0x4c,
  0x2c,
  0x44,
  0x20,0x20,
  0xcc,0xcc,0xcc,0xcc,
  0x58,                            	//CRC充许，8位CRC校验，外部时钟信号不使能，16M晶振
};
uchar TxRxBuffer[32];
//////////////////////////////////80us延时///////////////////////////////////
void Delay(uchar n)
{
	uint k;
	while(n--)
	for(k=0;k<80;k++);
}
///////////////
////////////////////////////////////////////////////////////////////////////
void Spi_initial()
{
	TRISC=TRISC&0xDF;		//r设置SDO（RC5）为输出**0*****
	TRISC=TRISC&0xF7;		//r设置SCK（RC3）为输出****0***
	TRISC=TRISC|0x10;		//r设置SDI（RC4）为输入***1****   
	INTCON=0x00;			//r关闭所有中断
	PIR1=0x00;			//r
	SSPCON=0x20;			//r写入时没有冲突发生，接收时没有溢出，开启SPI功能，空闲时处于低电平，fosc/4
	SSPSTAT=0xc0;			//r上升沿时发送，
	SSPIF=0;			//rPIR1=PIR1|0x08 ;
}
////////////////////////////////////////////////////////////////////////////	
uchar SpiReadSpiWrite(uchar DATA)	//r
{
	uchar b;
	Spi_initial();
	SSPBUF=DATA;	 		//写入要发送数据发送的同时也得到数据
	
	while(SSPIF==0);
	SSPIF=0;	
	b=SSPBUF;			//把得到数据保存
	return b;
	}
////////////////初始化nRF905///////////////////
void system_Init(void)			//r
{
   	TRISA=TRISA&0xF8;		//r
	TRISA=TRISA|0x38; 		//r
	TRISE=TRISE&0xFB; 		//r*****0**
	TRISC=0x90; 	
	ADCON1=0x07;		//必须有，设置为数字I/0口
   	CSN=1;				// Spi 	disable
	SCK=0;				// Spi clock line init low
	DR=0;				// Init DR for input
	AM=0;				// Init AM for input
	CD=0;				// Init CD for input
	PWR=1;				// nRF905 power on
	TRX_CE=0;			// Set nRF905 in standby mode
	TX_EN=0;			// set radio in Rx mode
}
////////////////////////////////初始化寄存器////////////////////////////////
void Config905(void)
{
	uchar i;
	CSN=0;				// Spi enable for write a spi command
	//SpiWrite(WC);			// Write config command写放配置命令
	for (i=0;i<11;i++)		// Write configration words  写放配置字
	{
	   SpiReadSpiWrite(RFConf[i]);
	}
	CSN=1;				// Disable Spi
}
////////////////////////////////////////////////////////////////////////
void SetRxMode(void)
{
	TX_EN=0;
	Delay(1); 
	TRX_CE=1;
	Delay(1); 			// delay for mode change(>=650us)
}
void RxPacket(void)							//读数据
{
	uchar i;
    	Delay(1);
    	TRX_CE=0;
	CSN=0;					// Spi enable for write a spi command
    	Delay(1);
	SpiReadSpiWrite(RRP);
	for (i = 0 ;i < 32 ;i++)
	{
	TxRxBuffer[i]=SpiReadSpiWrite(0);	// Read data and save to buffer
      	}
	CSN=1 ;
        while (AM||DR);				//等待读完毕DR=0，AM=0
	TRX_CE=1;							
}
////////////////
void  RX(void)
{
        SetRxMode();			// Set nRF905 in Rx mode
      	while(DR==0);
      	Delay(1);
	RxPacket();			// Recive data by nRF905
	Delay(1);
}
void StartUART(void)
{  	
	TXSTA=0x04;		 //设定TXSTA寄存器。同步从模式，8位数据，数据发送禁止，
	SPBRG=12; 		//6MHZ, BRGH=1, 9600BPS.
	PIE1=0; 		//清除PIE1所有中断使能。
	RCSTA=0x80; 		//禁止接收，打开通讯端口
	TRISC=TRISC|0X80;  	//:将RC7(RX)设置为输入方式 
	TRISC=TRISC&0XBF;  	// RC6(TX)设置为输出 
	TXEN=1;			//启动发送，TSR开始移位操作。
}
void R_S_Byte(uchar R_Byte)
{	
     	TXREG= R_Byte;  
}
void main(void)
{
        char i,j,temp[10];
        StartUART();			//串口初始化
	system_Init();			//系统初始化
	Delay(10);
	Config905();			//写入配置字，p-t-r 100
	while(1)
	{
		RX();
		for(i=0;i<32;i++)
		{
		R_S_Byte(TxRxBuffer[i]);
		Delay(10);
		}
/*	//以下用于检验是否读写配置成功
	CSN=0;                  	//SPI使能
	Delay(1);
	SpiReadSpiWrite(RRC);   	//写入控制字 	
	for(j=0;j<10;j++)		//读出配置字
	{
	temp[j]=SpiReadSpiWrite(0);
 	}
 	for(j=0;j<10;j++)
	{
 	R_S_Byte(temp[j]);		//串口显示
 	Delay(1000);
	}
	CSN=1;				//禁止SPI
*/
	}
}

