/****************************************************************************
 * bsp/drivers/drv_spi.c
 *
 *   Copyright (C) 2014  DRPM Development Team. All rights reserved.
 *   Author: TSAO, CHIA-CHENG <chiacheng.tsao@gmail.com>
 *
 *   GENERAL DESCRIPTION
 *      This module performs Serial Peripheral Interface (SPI) functionality.
 *
 ****************************************************************************/
#include "drv_spi.h"

void spi_initialize(void)
{
    reset_watch_dog();
#ifdef USE_BIT_BANGING_SPI
    MISO_INIT();
    MOSI_INIT();
    SCLK_INIT();
#endif

    /* Chip Select Pins */
#ifdef USE_HW_CS_CTRL
    CS_HWCTRL_INIT();
#else
    #ifdef USE_SDCARD
    //SD_CS : Micro SD Card Chip Select
    CS_SDCARD_INIT();
    #endif
#endif


    #ifdef USE_SFLASH
    //F_CS : Serial Flash Chip Select
    CS_SFLASH_INIT();
    #endif

    #ifdef USE_TFTMOD
    //CS : TFT Drive IC Chip Select
    CS_TFTMOD_INIT();
    #endif
}

/**
 * spi_xmit() - send a byte and recv response
 */
void spi_xmit(const uint8_t d)
{
    reset_watch_dog();
#ifdef USE_BIT_BANGING_SPI
	if (d & 0x80) DI_H(); else DI_L();	/* bit7 */
	CK_H(); CK_L();
	if (d & 0x40) DI_H(); else DI_L();	/* bit6 */
	CK_H(); CK_L();
	if (d & 0x20) DI_H(); else DI_L();	/* bit5 */
	CK_H(); CK_L();
	if (d & 0x10) DI_H(); else DI_L();	/* bit4 */
	CK_H(); CK_L();
	if (d & 0x08) DI_H(); else DI_L();	/* bit3 */
	CK_H(); CK_L();
	if (d & 0x04) DI_H(); else DI_L();	/* bit2 */
	CK_H(); CK_L();
	if (d & 0x02) DI_H(); else DI_L();	/* bit1 */
	CK_H(); CK_L();
	if (d & 0x01) DI_H(); else DI_L();	/* bit0 */
	CK_H(); CK_L();
#else
    // TODO : HW SPI MODE
#endif
}

/**
 * spi_rcvr() - send dummy btye then recv response
 */

uint8_t spi_rcvr(void)
{
	uint8_t r;

    reset_watch_dog();
#ifdef USE_BIT_BANGING_SPI
	DI_H();	/* Send 0xFF */

	r = 0;   if (DO) r++;	/* bit7 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit6 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit5 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit4 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit3 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit2 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit1 */
	CK_H(); CK_L();
	r <<= 1; if (DO) r++;	/* bit0 */
    CK_H(); CK_L();
#else
    // TODO : HW SPI MODE
#endif

    return r;
}

void spi_skip_bytes (
	uint8_t n		/* Number of bytes to skip */
)
{
    reset_watch_dog();
#ifdef USE_BIT_BANGING_SPI
	DI_H();	/* Send 0xFF */

	do {
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
		CK_H(); CK_L();
	} while (--n);
#else
    // TODO : HW SPI MODE
#endif

}


void spi_select(uint8_t cs, uint8_t high)
{
    reset_watch_dog();
    switch(cs)
    {
#ifndef USE_HW_CS_CTRL
        #ifdef USE_SDCARD
        case CS_SDCARD :

            if(high)
                CS_SDCARD_H();
            else
                CS_SDCARD_L();

            break;
        #endif
#endif

        #ifdef USE_SFLASH
        case CS_SFLASH :

            if(high)
                CS_SFLASH_H();
            else
                CS_SFLASH_L();

            break;
        #endif

        #ifdef USE_TFTMOD
        case CS_TFTMOD :

            if(high)
                CS_TFTMOD_H();
            else
                CS_TFTMOD_L();

            break;
        #endif

        default :
            break;
    }

    
}

/**
 * spi_set_divisor() - set new clock divider for USCI
 *
 * USCI speed is based on the SMCLK divided by BR0 and BR1
 * initially we start slow (400kHz) to conform to SDCard
 * specifications then we speed up once initialized (SPI_DEFAULT_SPEED)
 *
 * returns the previous setting
 */

uint16_t spi_set_divisor(const uint16_t clkdiv)
{
    reset_watch_dog();
    return 0;
}

