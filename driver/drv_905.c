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
    _905ConfigFn[] =
}

uint8_t (*_905OperateFn[10])(void);

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
