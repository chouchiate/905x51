#include "pic.h"

#define uchar unsigned char
#define uint unsigned int
uchar temp[32];
///////////////////////////ģʽ���ƶ���/////////////////////////////////
#define TX_EN	RA0 	//0r
#define TRX_CE  RA1	//0r
#define PWR     RA2	//0r
///////////////////////////SPI�ڶ���////////////////////////////////////
#define  MISO	RC4   	//r����ӳ�	1
#define  MOSI   RC5	//r��������	0
#define  SCK    RC3	//r����������    0
////////////////////////////////////////////////////////////////////////
#define  CSN    RE2
///////////////////////////״̬�����///////////////////////////////////
#define  CD     RA3	//r1
#define  AM     RA4	//r1
#define  DR     RA5	//r1
////////////////////////////905-SPIָ��/////////////////////////////////
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
uchar TxRxBuf[32]=			     //��������
{
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
0x17,0x18,0x19,0x20,0x21,0x22,0x23,0x24,
0x25,0x26,0x27,0x28,0x29,0x30,0x31,0x32,
};
uchar TxAddress[4]={0xcc,0xcc,0xcc,0xcc };    //���յ�ַ
////////////////////////////////////////////////////////////////////////////
uchar RFConf[11]=
{
  0x00,                             //��������//
  0x4c,                             //CH_NO,����Ƶ����423MHZ
  0x2C,                             //�������Ϊ10db,���ط����ڵ�Ϊ����ģʽ
  0x44,                             //��ַ�������ã�Ϊ4�ֽ�
  0x20,0x20,                        //���շ�����Ч���ݳ���Ϊ32�ֽ�
  0xCC,0xCC,0xCC,0xCC,              //���յ�ַ
  0x58,                              //CRC������8λCRCУ�飬�ⲿʱ���źŲ�ʹ�ܣ�16M����
};
////////////////////////////////////////////////////////////////////////////
void Delay(uchar x)               //1ms��ʱ
{
	uchar i;
	while(x--)
	for(i=0;i<80;i++);
}
////////////////////////////////////////////////////////////////////////////
void Spi_initial()
{
	TRISC=TRISC&0xDF;	//r����SDO��RC5��Ϊ���**0*****
	TRISC=TRISC&0xF7;	//r����SCK��RC3��Ϊ���****0***
	TRISC=TRISC|0x10;	//r����SDI��RC4��Ϊ����***1****   
	INTCON=0x00;		//r�ر������ж�
	PIR1=0x00;		//r
	SSPCON=0x20;		//rд��ʱû�г�ͻ����������ʱû�����������SPI���ܣ�����ʱ���ڵ͵�ƽ��fosc/4
	SSPSTAT=0xc0;		//r������ʱ���ͣ�
		ADCON1=0x07;
	SSPIF=0;		//rPIR1=PIR1|0x08 ;
}
////////////////////////////////////////////////////////////////////////////	
uchar SpiReadSpiWrite(uchar DATA)//r
{
	uchar b;
	Spi_initial();
	SSPBUF=DATA;	 	//д��Ҫ�������ݷ��͵�ͬʱҲ�õ�����
	do
	{
	;
	}
	while(SSPIF==0);
	SSPIF=0;	
	b=SSPBUF;		//�ѵõ����ݱ���
	return b;
	}
////////////////////////////////////////////////////////////////////////////
///////////////////////��ʼ���Ĵ���////////////////////////////////////////
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
///////////////////////��ʼ���Ĵ���////////////////////////////////////////
void Config905(void)
{
	uchar i;
	CSN=0;	
	Delay(1);				// Spi enable for write a spi command
	//SpiWrite(WC);				// Write config commandд����������
	for (i=0;i<11;i++)			// Write configration words  д��������
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
	TX_EN=1;				//����ʹ��
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
	TXSTA=0x04;		//r�趨TXSTA�Ĵ�����ͬ����ģʽ��8λ���ݣ����ݷ��ͽ�ֹ��
	SPBRG=12; 		//r6MHZ, BRGH=1, 9600BPS.
	PIE1=0; 		//r���PIE1�����ж�ʹ�ܡ�
	RCSTA=0x80; 		//r��ֹ���գ���ͨѶ�˿�
	TRISC=TRISC|0X80;  	//r��RC7(RX)����Ϊ���뷽ʽ 
	TRISC=TRISC&0XBF;  	//rRC6(TX)����Ϊ��� 
	TXEN=1;			//r�������ͣ�TSR��ʼ��λ������
}
void R_S_Byte(uchar R_Byte)
{	
     	TXREG= R_Byte;  
}
main()
{
	StartUART();			//���ڳ�ʼ��
	system_Init();			//ϵͳ��ʼ��
	Config905();			//д�������֣�p-t-r 100
	while(1)
	{
	TX();
/*	CSN=0;                  	//SPIʹ��
	Delay(1);
	SpiReadSpiWrite(RTA);   	//д������� 	
	for(j=0;j<4;j++)		//����������
	{
	temp[j]=SpiReadSpiWrite(0);
 	}
 	for(j=0;j<4;j++)
	{
 	R_S_Byte(temp[j]);		//������ʾ
 	Delay(1000);
	}
	CSN=1;				//��ֹSPI
	Delay(10);
	//R_S_Byte(temp[j]);*/
	}
	
}