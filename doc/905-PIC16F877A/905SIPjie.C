#include "pic.h"
//////////////////////////////////
#define uchar unsigned char
#define uint unsigned int
///////////////////////////ģʽ���ƶ���/////////////////////////////////
#define TX_EN	RA0 			//0r
#define TRX_CE  RA1			//0r
#define PWR     RA2			//0r
///////////////////////////SPI�ڶ���////////////////////////////////////
#define  MISO	RC4   			//r����ӳ�	1
#define  MOSI   RC5			//r��������	0
#define  SCK    RC3			//r����������   0
////////////////////////////////////////////////////////////////////////
#define  CSN    RE2
///////////////////////////״̬�����///////////////////////////////////
#define  CD     RA3			//r1
#define  AM     RA4			//r1
#define  DR     RA5			//r1
////////////////////////////905-SPIָ��/////////////////////////////////
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
//RF�Ĵ�������//
uchar RFConf[11]=
{
  0x00,                           	 //��������//
  0x4c,
  0x2c,
  0x44,
  0x20,0x20,
  0xcc,0xcc,0xcc,0xcc,
  0x58,                            	//CRC������8λCRCУ�飬�ⲿʱ���źŲ�ʹ�ܣ�16M����
};
uchar TxRxBuffer[32];
//////////////////////////////////80us��ʱ///////////////////////////////////
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
	TRISC=TRISC&0xDF;		//r����SDO��RC5��Ϊ���**0*****
	TRISC=TRISC&0xF7;		//r����SCK��RC3��Ϊ���****0***
	TRISC=TRISC|0x10;		//r����SDI��RC4��Ϊ����***1****   
	INTCON=0x00;			//r�ر������ж�
	PIR1=0x00;			//r
	SSPCON=0x20;			//rд��ʱû�г�ͻ����������ʱû�����������SPI���ܣ�����ʱ���ڵ͵�ƽ��fosc/4
	SSPSTAT=0xc0;			//r������ʱ���ͣ�
	SSPIF=0;			//rPIR1=PIR1|0x08 ;
}
////////////////////////////////////////////////////////////////////////////	
uchar SpiReadSpiWrite(uchar DATA)	//r
{
	uchar b;
	Spi_initial();
	SSPBUF=DATA;	 		//д��Ҫ�������ݷ��͵�ͬʱҲ�õ�����
	
	while(SSPIF==0);
	SSPIF=0;	
	b=SSPBUF;			//�ѵõ����ݱ���
	return b;
	}
////////////////��ʼ��nRF905///////////////////
void system_Init(void)			//r
{
   	TRISA=TRISA&0xF8;		//r
	TRISA=TRISA|0x38; 		//r
	TRISE=TRISE&0xFB; 		//r*****0**
	TRISC=0x90; 	
	ADCON1=0x07;		//�����У�����Ϊ����I/0��
   	CSN=1;				// Spi 	disable
	SCK=0;				// Spi clock line init low
	DR=0;				// Init DR for input
	AM=0;				// Init AM for input
	CD=0;				// Init CD for input
	PWR=1;				// nRF905 power on
	TRX_CE=0;			// Set nRF905 in standby mode
	TX_EN=0;			// set radio in Rx mode
}
////////////////////////////////��ʼ���Ĵ���////////////////////////////////
void Config905(void)
{
	uchar i;
	CSN=0;				// Spi enable for write a spi command
	//SpiWrite(WC);			// Write config commandд����������
	for (i=0;i<11;i++)		// Write configration words  д��������
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
void RxPacket(void)							//������
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
        while (AM||DR);				//�ȴ������DR=0��AM=0
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
	TXSTA=0x04;		 //�趨TXSTA�Ĵ�����ͬ����ģʽ��8λ���ݣ����ݷ��ͽ�ֹ��
	SPBRG=12; 		//6MHZ, BRGH=1, 9600BPS.
	PIE1=0; 		//���PIE1�����ж�ʹ�ܡ�
	RCSTA=0x80; 		//��ֹ���գ���ͨѶ�˿�
	TRISC=TRISC|0X80;  	//:��RC7(RX)����Ϊ���뷽ʽ 
	TRISC=TRISC&0XBF;  	// RC6(TX)����Ϊ��� 
	TXEN=1;			//�������ͣ�TSR��ʼ��λ������
}
void R_S_Byte(uchar R_Byte)
{	
     	TXREG= R_Byte;  
}
void main(void)
{
        char i,j,temp[10];
        StartUART();			//���ڳ�ʼ��
	system_Init();			//ϵͳ��ʼ��
	Delay(10);
	Config905();			//д�������֣�p-t-r 100
	while(1)
	{
		RX();
		for(i=0;i<32;i++)
		{
		R_S_Byte(TxRxBuffer[i]);
		Delay(10);
		}
/*	//�������ڼ����Ƿ��д���óɹ�
	CSN=0;                  	//SPIʹ��
	Delay(1);
	SpiReadSpiWrite(RRC);   	//д������� 	
	for(j=0;j<10;j++)		//����������
	{
	temp[j]=SpiReadSpiWrite(0);
 	}
 	for(j=0;j<10;j++)
	{
 	R_S_Byte(temp[j]);		//������ʾ
 	Delay(1000);
	}
	CSN=1;				//��ֹSPI
*/
	}
}
