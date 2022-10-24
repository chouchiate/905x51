/**
 * @file sys_spi.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "includes.h"

unsigned char   u8Receive_Data[5];
code char       u8Transmit_Data[5] = {0x4E, 0x55, 0x56, 0x4F, 0x54};
unsigned char   u8CNT = 0;
bit             SPI_Send_Flag = 0, SPI_Receive_Finish_Flag = 0;

//-----------------------------------------------------------------------------------------------------------
void SPI0_ISR(void) interrupt 9                  // Vecotr @  0x4B
{
    clr_SPI0SR_SPIF;
    if(!SPI_Send_Flag)
    {
        u8Receive_Data[u8CNT] = SPI0DR;
        u8CNT ++;
        SPI0DR = u8Transmit_Data[u8CNT];
    }
    SPI_Send_Flag = 0;

    if (u8CNT==5)
      SPI_Receive_Finish_Flag = 1;

}


void SPI0_Initial(void)
{
    MFP_P00_SPI0_MOSI;      /*define  SPI0 MOSI pin */
    P00_QUASI_MODE;
    MFP_P01_SPI0_MISO;      /*define  SPI0 MISO pin */
    P01_QUASI_MODE;
    MFP_P02_SPI0_CLK;       /*define  SPI0 CLK pin */
    P02_QUASI_MODE;
    MFP_P03_SPI0_SS;        /*define  SPI0 SS pin as normal GPIO*/
    P03_QUASI_MODE;

    clr_SPI0CR0_MSTR;                                   // SPI in Slave mode
    clr_SPI0CR0_LSBFE;                                  // MSB first

    clr_SPI0CR0_CPOL;                                   // The SPI clock is low in idle mode
    set_SPI0CR0_CPHA;                                   // The data is sample on the second edge of SPI clock

    ENABLE_SPI0_INTERRUPT;                              // Enable SPI interrupt
    ENABLE_GLOBAL_INTERRUPT;
    set_SPI0CR0_SPIEN;                                  // Enable SPI function

    clr_SPI0SR_SPIF;                                    // Clear SPI flag
    SPI0DR = u8Transmit_Data[0];
}