
/****************************************************************************
 * bsp/drivers/drv_lcd.c
 *
 *   Copyright (C) 2014  DRPM Development Team. All rights reserved.
 *   Author: TSAO, CHIA-CHENG <chiacheng.tsao@gmail.com>
 *
 *   GENERAL DESCRIPTION
 *      This is a driver for the 1.8" SPI display.
 *
 ****************************************************************************/
#include "drv_lcd.h"
#include "drv_spi.h"

#if ( CFG_DRV_LCD > 0 )
#ifdef CODE_1
/* Standard includes. */
#include <stdlib.h>
#include <string.h>

void lcd7735_invertDisplay(const uint8_t mode);
void lcd7735_setRotation(uint8_t m);
void lcd7735_xmit(const uint8_t tb);
void lcd7735_sendCmd(const uint8_t c);
void lcd7735_sendData(const uint8_t d);
static void _putch(uint8_t c);

#define DELAY 0x80
#define putpix(c) { lcd7735_xmit(c >> 8); lcd7735_xmit(c & 0xFF); }
#define _scr(r,c) ((char *)(_screen.scr + ((r) * _screen.ncol) + (c)))

#define cursor_draw		cursor_expose(1)
#define cursor_erase	cursor_expose(0)

static uint16_t _width = ST7735_TFTWIDTH;
static uint16_t _height = ST7735_TFTHEIGHT;

/*
    Font Size           : 8x12
    Memory usage   : 1144 bytes
    Characters        : 95
*/
const unsigned char SmallFont[] = {
0x08,0x0C,0x20,0x5F,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // <Space>
0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x00,0x00, // !
0x00,0x28,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // "
0x00,0x00,0x28,0x28,0xFC,0x28,0x50,0xFC,0x50,0x50,0x00,0x00, // #
0x00,0x20,0x78,0xA8,0xA0,0x60,0x30,0x28,0xA8,0xF0,0x20,0x00, // $
0x00,0x00,0x48,0xA8,0xB0,0x50,0x28,0x34,0x54,0x48,0x00,0x00, // %
0x00,0x00,0x20,0x50,0x50,0x78,0xA8,0xA8,0x90,0x6C,0x00,0x00, // &
0x00,0x40,0x40,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // '
0x00,0x04,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x04,0x00, // (
0x00,0x40,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x20,0x40,0x00, // )
0x00,0x00,0x00,0x20,0xA8,0x70,0x70,0xA8,0x20,0x00,0x00,0x00, // *
0x00,0x00,0x20,0x20,0x20,0xF8,0x20,0x20,0x20,0x00,0x00,0x00, // +
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x40,0x80, // ,
0x00,0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00, // -
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00, // .
0x00,0x08,0x10,0x10,0x10,0x20,0x20,0x40,0x40,0x40,0x80,0x00, // /
0x00,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00, // 0
0x00,0x00,0x20,0x60,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00, // 1
0x00,0x00,0x70,0x88,0x88,0x10,0x20,0x40,0x80,0xF8,0x00,0x00, // 2
0x00,0x00,0x70,0x88,0x08,0x30,0x08,0x08,0x88,0x70,0x00,0x00, // 3
0x00,0x00,0x10,0x30,0x50,0x50,0x90,0x78,0x10,0x18,0x00,0x00, // 4
0x00,0x00,0xF8,0x80,0x80,0xF0,0x08,0x08,0x88,0x70,0x00,0x00, // 5
0x00,0x00,0x70,0x90,0x80,0xF0,0x88,0x88,0x88,0x70,0x00,0x00, // 6
0x00,0x00,0xF8,0x90,0x10,0x20,0x20,0x20,0x20,0x20,0x00,0x00, // 7
0x00,0x00,0x70,0x88,0x88,0x70,0x88,0x88,0x88,0x70,0x00,0x00, // 8
0x00,0x00,0x70,0x88,0x88,0x88,0x78,0x08,0x48,0x70,0x00,0x00, // 9
0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x20,0x00,0x00, // :
0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x20,0x20,0x00, // ;
0x00,0x04,0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x04,0x00,0x00, // <
0x00,0x00,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0x00,0x00, // =
0x00,0x40,0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x40,0x00,0x00, // >
0x00,0x00,0x70,0x88,0x88,0x10,0x20,0x20,0x00,0x20,0x00,0x00, // ?
0x00,0x00,0x70,0x88,0x98,0xA8,0xA8,0xB8,0x80,0x78,0x00,0x00, // @
0x00,0x00,0x20,0x20,0x30,0x50,0x50,0x78,0x48,0xCC,0x00,0x00, // A
0x00,0x00,0xF0,0x48,0x48,0x70,0x48,0x48,0x48,0xF0,0x00,0x00, // B
0x00,0x00,0x78,0x88,0x80,0x80,0x80,0x80,0x88,0x70,0x00,0x00, // C
0x00,0x00,0xF0,0x48,0x48,0x48,0x48,0x48,0x48,0xF0,0x00,0x00, // D
0x00,0x00,0xF8,0x48,0x50,0x70,0x50,0x40,0x48,0xF8,0x00,0x00, // E
0x00,0x00,0xF8,0x48,0x50,0x70,0x50,0x40,0x40,0xE0,0x00,0x00, // F
0x00,0x00,0x38,0x48,0x80,0x80,0x9C,0x88,0x48,0x30,0x00,0x00, // G
0x00,0x00,0xCC,0x48,0x48,0x78,0x48,0x48,0x48,0xCC,0x00,0x00, // H
0x00,0x00,0xF8,0x20,0x20,0x20,0x20,0x20,0x20,0xF8,0x00,0x00, // I
0x00,0x00,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x90,0xE0,0x00, // J
0x00,0x00,0xEC,0x48,0x50,0x60,0x50,0x50,0x48,0xEC,0x00,0x00, // K
0x00,0x00,0xE0,0x40,0x40,0x40,0x40,0x40,0x44,0xFC,0x00,0x00, // L
0x00,0x00,0xD8,0xD8,0xD8,0xD8,0xA8,0xA8,0xA8,0xA8,0x00,0x00, // M
0x00,0x00,0xDC,0x48,0x68,0x68,0x58,0x58,0x48,0xE8,0x00,0x00, // N
0x00,0x00,0x70,0x88,0x88,0x88,0x88,0x88,0x88,0x70,0x00,0x00, // O
0x00,0x00,0xF0,0x48,0x48,0x70,0x40,0x40,0x40,0xE0,0x00,0x00, // P
0x00,0x00,0x70,0x88,0x88,0x88,0x88,0xE8,0x98,0x70,0x18,0x00, // Q
0x00,0x00,0xF0,0x48,0x48,0x70,0x50,0x48,0x48,0xEC,0x00,0x00, // R
0x00,0x00,0x78,0x88,0x80,0x60,0x10,0x08,0x88,0xF0,0x00,0x00, // S
0x00,0x00,0xF8,0xA8,0x20,0x20,0x20,0x20,0x20,0x70,0x00,0x00, // T
0x00,0x00,0xCC,0x48,0x48,0x48,0x48,0x48,0x48,0x30,0x00,0x00, // U
0x00,0x00,0xCC,0x48,0x48,0x50,0x50,0x30,0x20,0x20,0x00,0x00, // V
0x00,0x00,0xA8,0xA8,0xA8,0x70,0x50,0x50,0x50,0x50,0x00,0x00, // W
0x00,0x00,0xD8,0x50,0x50,0x20,0x20,0x50,0x50,0xD8,0x00,0x00, // X
0x00,0x00,0xD8,0x50,0x50,0x20,0x20,0x20,0x20,0x70,0x00,0x00, // Y
0x00,0x00,0xF8,0x90,0x10,0x20,0x20,0x40,0x48,0xF8,0x00,0x00, // Z
0x00,0x38,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x38,0x00, // [
0x00,0x40,0x40,0x40,0x20,0x20,0x10,0x10,0x10,0x08,0x00,0x00, // <Backslash>
0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x70,0x00, // ]
0x00,0x20,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ^
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC, // _
0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // '
0x00,0x00,0x00,0x00,0x00,0x30,0x48,0x38,0x48,0x3C,0x00,0x00, // a
0x00,0x00,0xC0,0x40,0x40,0x70,0x48,0x48,0x48,0x70,0x00,0x00, // b
0x00,0x00,0x00,0x00,0x00,0x38,0x48,0x40,0x40,0x38,0x00,0x00, // c
0x00,0x00,0x18,0x08,0x08,0x38,0x48,0x48,0x48,0x3C,0x00,0x00, // d
0x00,0x00,0x00,0x00,0x00,0x30,0x48,0x78,0x40,0x38,0x00,0x00, // e
0x00,0x00,0x1C,0x20,0x20,0x78,0x20,0x20,0x20,0x78,0x00,0x00, // f
0x00,0x00,0x00,0x00,0x00,0x3C,0x48,0x30,0x40,0x78,0x44,0x38, // g
0x00,0x00,0xC0,0x40,0x40,0x70,0x48,0x48,0x48,0xEC,0x00,0x00, // h
0x00,0x00,0x20,0x00,0x00,0x60,0x20,0x20,0x20,0x70,0x00,0x00, // i
0x00,0x00,0x10,0x00,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0xE0, // j
0x00,0x00,0xC0,0x40,0x40,0x5C,0x50,0x70,0x48,0xEC,0x00,0x00, // k
0x00,0x00,0xE0,0x20,0x20,0x20,0x20,0x20,0x20,0xF8,0x00,0x00, // l
0x00,0x00,0x00,0x00,0x00,0xF0,0xA8,0xA8,0xA8,0xA8,0x00,0x00, // m
0x00,0x00,0x00,0x00,0x00,0xF0,0x48,0x48,0x48,0xEC,0x00,0x00, // n
0x00,0x00,0x00,0x00,0x00,0x30,0x48,0x48,0x48,0x30,0x00,0x00, // o
0x00,0x00,0x00,0x00,0x00,0xF0,0x48,0x48,0x48,0x70,0x40,0xE0, // p
0x00,0x00,0x00,0x00,0x00,0x38,0x48,0x48,0x48,0x38,0x08,0x1C, // q
0x00,0x00,0x00,0x00,0x00,0xD8,0x60,0x40,0x40,0xE0,0x00,0x00, // r
0x00,0x00,0x00,0x00,0x00,0x78,0x40,0x30,0x08,0x78,0x00,0x00, // s
0x00,0x00,0x00,0x20,0x20,0x70,0x20,0x20,0x20,0x18,0x00,0x00, // t
0x00,0x00,0x00,0x00,0x00,0xD8,0x48,0x48,0x48,0x3C,0x00,0x00, // u
0x00,0x00,0x00,0x00,0x00,0xEC,0x48,0x50,0x30,0x20,0x00,0x00, // v
0x00,0x00,0x00,0x00,0x00,0xA8,0xA8,0x70,0x50,0x50,0x00,0x00, // w
0x00,0x00,0x00,0x00,0x00,0xD8,0x50,0x20,0x50,0xD8,0x00,0x00, // x
0x00,0x00,0x00,0x00,0x00,0xEC,0x48,0x50,0x30,0x20,0x20,0xC0, // y
0x00,0x00,0x00,0x00,0x00,0x78,0x10,0x20,0x20,0x78,0x00,0x00, // z
0x00,0x18,0x10,0x10,0x10,0x20,0x10,0x10,0x10,0x10,0x18,0x00, // {
0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10, // |
0x00,0x60,0x20,0x20,0x20,0x10,0x20,0x20,0x20,0x20,0x60,0x00, // }
0x40,0xA4,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ~
};  

// Init for 7735R, part 1 (red or green tab)
static const uint8_t  Rcmd1[] = {                 
	15,							// 15 commands in list:
	ST7735_SWRESET,   DELAY,	//  1: Software reset, 0 args, w/delay
	150,						//     150 ms delay
	ST7735_SLPOUT ,   DELAY,	//  2: Out of sleep mode, 0 args, w/delay
	255,						//     500 ms delay
	ST7735_FRMCTR1, 3      ,	//  3: Frame rate ctrl - normal mode, 3 args:
	0x01, 0x2C, 0x2D,			//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR2, 3      ,	//  4: Frame rate control - idle mode, 3 args:
	0x01, 0x2C, 0x2D,			//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR3, 6      ,	//  5: Frame rate ctrl - partial mode, 6 args:
	0x01, 0x2C, 0x2D,			//     Dot inversion mode
	0x01, 0x2C, 0x2D,			//     Line inversion mode
	ST7735_INVCTR , 1      ,	//  6: Display inversion ctrl, 1 arg, no delay:
	0x07,						//     No inversion
	ST7735_PWCTR1 , 3      ,	//  7: Power control, 3 args, no delay:
	0xA2,
	0x02,						//     -4.6V
	0x84,						//     AUTO mode
	ST7735_PWCTR2 , 1      ,	//  8: Power control, 1 arg, no delay:
	0xC5,						//     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
	ST7735_PWCTR3 , 2      ,	//  9: Power control, 2 args, no delay:
	0x0A,						//     Opamp current small
	0x00,						//     Boost frequency
	ST7735_PWCTR4 , 2      ,	// 10: Power control, 2 args, no delay:
	0x8A,						//     BCLK/2, Opamp current small & Medium low
	0x2A,  
	ST7735_PWCTR5 , 2      ,	// 11: Power control, 2 args, no delay:
	0x8A, 0xEE,
	ST7735_VMCTR1 , 1      ,	// 12: Power control, 1 arg, no delay:
	0x0E,
	ST7735_INVOFF , 0      ,	// 13: Don't invert display, no args, no delay
	ST7735_MADCTL , 1      ,	// 14: Memory access control (directions), 1 arg:
	0xC0,						//     row addr/col addr, bottom to top refresh, RGB order
	ST7735_COLMOD , 1+DELAY,	//  15: Set color mode, 1 arg + delay:
	0x05,						//     16-bit color 5-6-5 color format
	10							//     10 ms delay
};
// Init for 7735R, part 2 (green tab only)
static const uint8_t Rcmd2green[] = {
	2,							//  2 commands in list:
	ST7735_CASET  , 4      ,	//  1: Column addr set, 4 args, no delay:
	0x00, 0x02,					//     XSTART = 0
	0x00, 0x7F+0x02,			//     XEND = 129
	ST7735_RASET  , 4      ,	//  2: Row addr set, 4 args, no delay:
	0x00, 0x01,					//     XSTART = 0
	0x00, 0x9F+0x01				//     XEND = 160
};
// Init for 7735R, part 2 (red tab only)
static const uint8_t Rcmd2red[] = {
	2,							//  2 commands in list:
	ST7735_CASET  , 4      ,	//  1: Column addr set, 4 args, no delay:
	0x00, 0x00,					//     XSTART = 0
	0x00, 0x7F,					//     XEND = 127
	ST7735_RASET  , 4      ,	//  2: Row addr set, 4 args, no delay:
	0x00, 0x00,					//     XSTART = 0
	0x00, 0x9F				//     XEND = 159
};
// Init for 7735R, part 3 (red or green tab)
static const uint8_t Rcmd3[] = {
	4,							//  4 commands in list:
	ST7735_GMCTRP1, 16      ,	//  1: Magical unicorn dust, 16 args, no delay:
	0x02, 0x1c, 0x07, 0x12,
	0x37, 0x32, 0x29, 0x2d,
	0x29, 0x25, 0x2B, 0x39,
	0x00, 0x01, 0x03, 0x10,
	ST7735_GMCTRN1, 16      ,	//  2: Sparkles and rainbows, 16 args, no delay:
	0x03, 0x1d, 0x07, 0x06,
	0x2E, 0x2C, 0x29, 0x2D,
	0x2E, 0x2E, 0x37, 0x3F,
	0x00, 0x00, 0x02, 0x10,
	ST7735_NORON  ,    DELAY,	//  3: Normal display on, no args, w/delay
	10,							//     10 ms delay
	ST7735_DISPON ,    DELAY,	//  4: Main screen turn on, no args w/delay
	100							//     100 ms delay
};

static int colstart = 0;
static int rowstart = 0; // May be overridden in init func
//static uint8_t tabcolor	= 0;
static uint8_t orientation = PORTRAIT;

/* ASCII green-screen terminal emulator */

typedef struct _cursor {
	uint16_t	row;
	uint16_t	col;
	uint8_t		*bitmap; // not used yet
} Cursor;

typedef struct _font {
	uint8_t 	*font;
	uint8_t 	x_size;
	uint8_t 	y_size;
	uint8_t		offset;
	uint16_t	numchars;
} Font;

static struct __screen {
	Cursor 		c;
	uint8_t 	nrow;
	uint8_t 	ncol;
	Font 		fnt;
	uint16_t 	fg;
	uint16_t 	bg;
	char		*scr;
} _screen;

//static Font cfont;
//static uint8_t _transparent = 0;
//static uint16_t _fg = ST7735_GREEN;
//static uint16_t _bg = ST7735_BLACK;


/* printf in "CLib" will call putchar.
   user can implement this function -- send a char to UART? */
int putchar (int c)
{
    lcd7735_putc(c);
    return c;
}

void delay_ms(uint32_t ms)
{    
    #if 1 // Use this will save 20 word in code.
    uint32_t c = ms*50; // 49 ~= 1 us / ((1 / 49152000) * 10^6) us

    do{
        //asm("NOP");
        reset_watch_dog();
    }while(--c);
    #else
    unsigned long c = 0;

    for(c = 0 ; c < ( 50*n ) ; c++ )
    {
        asm("NOP");
    }
    #endif
    
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
static void commandList(const uint8_t *addr) {
	uint8_t  numCommands, numArgs;
	uint16_t ms;

	numCommands = *addr++;   // Number of commands to follow
	while(numCommands--) {                 // For each command...
		lcd7735_sendCmd(*addr++); //   Read, issue command
		numArgs  = *addr++;    //   Number of args to follow
		ms       = numArgs & DELAY;          //   If hibit set, delay follows args
		numArgs &= ~DELAY;                   //   Mask out delay bit
		while(numArgs--) {                   //   For each argument...
			lcd7735_sendData(*addr++);  //     Read, issue argument
		}

		if(ms) {
			ms = *addr++; // Read post-command delay time (ms)
			if(ms == 255) ms = 500;     // If 255, delay for 500 ms
			delay_ms(ms);
		}
	}
}

// Initialization code common to both 'B' and 'R' type displays
static void commonInit(const uint8_t *cmdList) {
	// toggle RST low to reset; CS low so it'll listen to us
	LCD_CS_L();
#ifdef LCD_SOFT_RESET
	lcd7735_sendCmd(ST7735_SWRESET);
	delay_ms(500);
#else
	LCD_RST_H();
	delay_ms(500);
	LCD_RST_L();
	delay_ms(500);
	LCD_RST_H();
	delay_ms(500);
#endif    
	if(cmdList) commandList(cmdList);
}

// Initialization for ST7735R screens (green or red tabs)
void lcd7735_initR(uint8_t options) {
	delay_ms(50);
	commonInit(Rcmd1);
	if(options == INITR_GREENTAB) {
		commandList(Rcmd2green);
		colstart = 2;
		rowstart = 1;
	} else {
		// colstart, rowstart left at default '0' values
		commandList(Rcmd2red);
	}
	commandList(Rcmd3);

	// if black, change MADCTL color filter
	if (options == INITR_BLACKTAB) {
		lcd7735_sendCmd(ST7735_MADCTL);
		lcd7735_sendData(0xC0);
	}

	//  tabcolor = options;
}

void lcd7735_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	lcd7735_sendCmd(ST7735_CASET);		// Column addr set
	lcd7735_sendData(0x00);
	lcd7735_sendData(x0+colstart);     // XSTART 
	lcd7735_sendData(0x00);
	lcd7735_sendData(x1+colstart);     // XEND

	lcd7735_sendCmd(ST7735_RASET); // Row addr set
	lcd7735_sendData(0x00);
	lcd7735_sendData(y0+rowstart);     // YSTART
	lcd7735_sendData(0x00);
	lcd7735_sendData(y1+rowstart);     // YEND

	lcd7735_sendCmd(ST7735_RAMWR); // write to RAM
}

void lcd7735_pushColor(uint16_t color) {
	LCD_A0_H();  
	putpix(color);
}

// draw color pixel on screen
void lcd7735_drawPixel(int16_t x, int16_t y, uint16_t color) {

	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

	lcd7735_setAddrWindow(x,y,x+1,y+1);
	lcd7735_pushColor(color);
}

// fill a rectangle
void lcd7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {	
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;

	lcd7735_setAddrWindow(x, y, x+w-1, y+h-1);

	LCD_A0_H();
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
			putpix(color);
		}
	}
}

/* Private functions */
static void _scrollup() {
	int r,c;
	_screen.c.row = 0;
	_screen.c.col = 0;
	for(r=1;r<_screen.nrow;r++)
		for(c=0;c<_screen.ncol;c++) {
			_putch(*_scr(r,c));
			_screen.c.col++;
			if( _screen.c.col == _screen.ncol ) {			
				_screen.c.col = 0;
				_screen.c.row++;
			}
		}
		for(c=0;c<_screen.ncol;c++) {
			_putch(' ');
			_screen.c.col++;
		}
		_screen.c.row = _screen.nrow - 1;
		_screen.c.col = 0;
}

static void cursor_expose(int flg) {
	uint8_t i,fz;
	uint16_t j;
	int x,y;

	fz = _screen.fnt.x_size/8;
	x = _screen.c.col * _screen.fnt.x_size;
	y = _screen.c.row * _screen.fnt.y_size;
	lcd7735_setAddrWindow(x,y,x+_screen.fnt.x_size-1,y+_screen.fnt.y_size-1);
	for(j=0;j<((fz)*_screen.fnt.y_size);j++) {
		for(i=0;i<8;i++) {
			if( flg )
				lcd7735_pushColor(_screen.fg);
			else
				lcd7735_pushColor(_screen.bg);
		}
	}
}

static void cursor_nl() {
	_screen.c.col = 0;
	_screen.c.row++;
	if( _screen.c.row == _screen.nrow ) {
		_scrollup();
	}
}

static void cursor_fwd() {
	_screen.c.col++; 
	if( _screen.c.col == _screen.ncol ) {
		cursor_nl();
	}
}


static void cursor_init() {
	_screen.c.row = 0;
	_screen.c.col = 0;
}

static void _putch(uint8_t c) {
	uint8_t i,ch,fz;
	uint16_t j;
	uint16_t temp; 
	int x,y;

	fz = _screen.fnt.x_size/8;
	x = _screen.c.col * _screen.fnt.x_size;
	y = _screen.c.row * _screen.fnt.y_size;
	lcd7735_setAddrWindow(x,y,x+_screen.fnt.x_size-1,y+_screen.fnt.y_size-1);
	temp=((c-_screen.fnt.offset)*((fz)*_screen.fnt.y_size))+4;
	for(j=0;j<((fz)*_screen.fnt.y_size);j++) {
		ch = _screen.fnt.font[temp];
		for(i=0;i<8;i++) {   
			if((ch&(1<<(7-i)))!=0) {
				lcd7735_pushColor(_screen.fg);
			} else {
				lcd7735_pushColor(_screen.bg);
			}   
		}
		temp++;
	}
	*_scr(_screen.c.row, _screen.c.col) = c;
}

/* Public functions */
void lcd7735_init_screen(void *font,uint16_t fg, uint16_t bg, uint8_t orientation) {
	lcd7735_setRotation(orientation);
	lcd7735_fillScreen(bg);
	_screen.fg = fg;
	_screen.bg = bg;
	_screen.fnt.font = (uint8_t *)font;
	_screen.fnt.x_size = _screen.fnt.font[0];
	_screen.fnt.y_size = _screen.fnt.font[1];
	_screen.fnt.offset = _screen.fnt.font[2];
	_screen.fnt.numchars = _screen.fnt.font[3];
	_screen.nrow = _height / _screen.fnt.y_size;
	_screen.ncol = _width  / _screen.fnt.x_size;
	_screen.scr = malloc(_screen.nrow * _screen.ncol);
	memset((void*)_screen.scr,' ',_screen.nrow * _screen.ncol);
	cursor_init();
	cursor_draw;
    delay_ms(100);
}

void lcd7735_putc(char c) {
	if( c != '\n' && c != '\r' ) {
		_putch(c);
		cursor_fwd();
	} else {
		cursor_erase;
		cursor_nl();
	}
	cursor_draw;
}

void lcd7735_puts(char *s) {
	int i;
	for(i=0;i<strlen(s);i++) {
		if( s[i] != '\n' && s[i] != '\r' ) {
			_putch(s[i]);
			cursor_fwd();
		} else {
			cursor_erase;
			cursor_nl();
		}
	}
	cursor_draw;
}

#if 0
void lcd7735_cursor_set(uint16_t row, uint16_t col) {
	if( row < _screen.nrow && col < _screen.ncol ) {
		_screen.c.row = row;
		_screen.c.col = col;
	}
	cursor_draw;
}

void lcd7735_cursor_get(uint16_t *row, uint16_t *col) {
		*row = _screen.c.row;
		*col = _screen.c.col;
}
#endif

/* Service functions */
void lcd7735_fillScreen(uint16_t color) {
	lcd7735_fillRect(0, 0,  _width, _height, color);
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t lcd7735_Color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void lcd7735_setRotation(uint8_t m) {
	uint8_t rotation = m % 4; // can't be higher than 3

	lcd7735_sendCmd(ST7735_MADCTL);
	switch (rotation) {
   case PORTRAIT:
	   lcd7735_sendData(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
	   _width  = ST7735_TFTWIDTH;
	   _height = ST7735_TFTHEIGHT;
	   break;
   case LANDSAPE:
	   lcd7735_sendData(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
	   _width  = ST7735_TFTHEIGHT;
	   _height = ST7735_TFTWIDTH;
	   break;
   case PORTRAIT_FLIP:
	   lcd7735_sendData(MADCTL_RGB);
	   _width  = ST7735_TFTWIDTH;
	   _height = ST7735_TFTHEIGHT;
	   break;
   case LANDSAPE_FLIP:
	   lcd7735_sendData(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
	   _width  = ST7735_TFTHEIGHT;
	   _height = ST7735_TFTWIDTH;
	   break;
   default:
	   return;
	}
	orientation = m;
}

void lcd7735_invertDisplay(const uint8_t mode) {
	if( mode == INVERT_ON ) lcd7735_sendCmd(ST7735_INVON);
	else if( mode == INVERT_OFF ) lcd7735_sendCmd(ST7735_INVOFF);
}

void lcd7735_lcdOff() {
	lcd7735_sendCmd(ST7735_DISPOFF);
}

void lcd7735_lcdOn() {
	lcd7735_sendCmd(ST7735_DISPON);
}

uint8_t lcd7735_getWidth() {
	return(_width);
}
uint8_t lcd7735_getHeight() {
	return(_height);
}

void lcd7735_init(void) {
    spi_initialize();
    LCD_SCK_INIT();
    LCD_SDA_INIT();
    LCD_A0_INIT();
    LCD_RST_INIT();
}

void lcd7735_xmit(const uint8_t tb) {
#if 0
    for(i=0; i<8; i++) {
        if (d & 0x80) LCD_MOSI1;
        else LCD_MOSI0;
        d = d<<1;
        LCD_SCK0;
        LCD_SCK1;
    }
#else
    reset_watch_dog();
    if (tb & 0x80) LCD_SDA_H(); else LCD_SDA_L();	/* bit7 */
    LCD_SCK_L(); LCD_SCK_H();
    if (tb & 0x40) LCD_SDA_H(); else LCD_SDA_L();	/* bit6 */
    LCD_SCK_L(); LCD_SCK_H();
    if (tb & 0x20) LCD_SDA_H(); else LCD_SDA_L();	/* bit5 */
    LCD_SCK_L(); LCD_SCK_H();
    if (tb & 0x10) LCD_SDA_H(); else LCD_SDA_L();	/* bit4 */
    LCD_SCK_L(); LCD_SCK_H();
    if (tb & 0x08) LCD_SDA_H(); else LCD_SDA_L();	/* bit3 */
    LCD_SCK_L(); LCD_SCK_H();
    if (tb & 0x04) LCD_SDA_H(); else LCD_SDA_L();	/* bit2 */
    LCD_SCK_L(); LCD_SCK_H();
    if (tb & 0x02) LCD_SDA_H(); else LCD_SDA_L();	/* bit1 */
    LCD_SCK_L(); LCD_SCK_H();
    if (tb & 0x01) LCD_SDA_H(); else LCD_SDA_L();	/* bit0 */
    LCD_SCK_L(); LCD_SCK_H();
#endif
}

// Send control command to controller
void lcd7735_sendCmd(const uint8_t c) {
    LCD_A0_L();
    //LCD_CS_L();
    lcd7735_xmit(c);
    //LCD_CS_H();
}

// Send parameters o command to controller
void lcd7735_sendData(const uint8_t d) {
    LCD_A0_H();
    //LCD_CS_L();
    lcd7735_xmit(d);
    //LCD_CS_H();
}

#if 0
void test_ascii_screen(void) {
	unsigned char x;
	int i;

    lcd7735_init_screen((void *)&SmallFont[0],ST7735_WHITE,ST7735_BLACK,PORTRAIT);
    x = 0x20;

	for(i=0;i<95;i++) {
		lcd7735_putc(x+i);
	}
}
#endif
#endif

#endif
