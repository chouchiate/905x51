#ifndef __DRV_905_H__
#define __DRV_905_H__

#include <stdin.h>

#define WC		0x00			// Write configuration register command
#define RC		0x10 			// Read  configuration register command
#define WTP		0x20 			// Write TX Payload  command
#define RTP		0x21			// Read  TX Payload  command
#define WTA		0x22			// Write TX Address  command
#define RTA		0x23			// Read  TX Address  command
#define RRP		0x24			// Read  RX Payload  command
#define CC 		0x8000			// fast setting of CH_NO, HFREQ_PLL and PA_PWR in the CONFIGURATION REGISTER.
								// instruction format: 1000 pphc cccc cccc
								// CH_NO= ccccccccc, HFREQ_PLL = h PA_PWR = pp

// pin definitions
sbit	TX_EN	=P1^7;
sbit	TRX_CE	=P1^6;
sbit	PWR_UP	=P1^5;
sbit	MISO	=P1^1;
sbit	MOSI	=P1^0;
sbit	SCK		=P3^7;
sbit	CSN		=P3^3;

sbit	AM		=P1^3;
sbit	DR		=P3^2;
sbit	CD		=P1^4;

sbit    LED1    =P3^4;
sbit    LED2    =P3^5;
sbit    KEY1    =P3^0;
sbit    KEY2    =P3^1;

typedef enum {

} _905Configs;

typedef enum {
	_NO_OP = 0,
	_TX_PACKET = 1,
	_RX_PACKET = 2,

} _905Operations;

extern uint8_t (*_905ConfigFn[10])(unsigned char cmd, unsigned char data);
extern uint8_t (*_905OperateFn[10])(void);

extern void setTxMode(void);
extern void setRxMode(void);


#endif /* __DRV_905_H__*/