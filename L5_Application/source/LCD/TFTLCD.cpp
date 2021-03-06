#include "TFTLCD.h"
#include "gpio.hpp"
#include "utilities.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define swap(a, b)  { int16_t t = a; a = b; b = t; }


#define TFTLCD_DRIV_ID_READ     0x00
#define TFTLCD_DRIV_OUT_CTRL        0x01
#define TFTLCD_DRIV_WAV_CTRL        0x02
#define TFTLCD_ENTRY_MOD        0x03
#define TFTLCD_RESIZE_CTRL      0x04
#define TFTLCD_DISP_CTRL1       0x07
#define TFTLCD_DISP_CTRL2       0x08
#define TFTLCD_DISP_CTRL3       0x09
#define TFTLCD_DISP_CTRL4       0x0A
#define TFTLCD_FRM_MARKER_POS       0x0D
#define TFTLCD_POW_CTRL1        0x10
#define TFTLCD_POW_CTRL2        0x11
#define TFTLCD_POW_CTRL3        0x12
#define TFTLCD_POW_CTRL4        0x13
#define TFTLCD_GRAM_HOR_AD      0x20
#define TFTLCD_GRAM_VER_AD      0x21
#define TFTLCD_RW_GRAM          0x22
#define TFTLCD_VCOMH_CTRL       0x29
#define TFTLCD_FRM_RATE_COL_CTRL    0x2B
#define TFTLCD_GAMMA_CTRL1      0x30
#define TFTLCD_GAMMA_CTRL2      0x31
#define TFTLCD_GAMMA_CTRL3      0x32
#define TFTLCD_GAMMA_CTRL4      0x35
#define TFTLCD_GAMMA_CTRL5      0x36
#define TFTLCD_GAMMA_CTRL6      0x37
#define TFTLCD_GAMMA_CTRL7      0x38
#define TFTLCD_GAMMA_CTRL8      0x39
#define TFTLCD_GAMMA_CTRL9      0x3C
#define TFTLCD_GAMMA_CTRL10     0x3D
#define TFTLCD_HOR_START_AD     0x50
#define TFTLCD_HOR_END_AD       0x51
#define TFTLCD_VER_START_AD     0x52
#define TFTLCD_VER_END_AD       0x53
#define TFTLCD_GATE_SCAN_CTRL1      0x60
#define TFTLCD_GATE_SCAN_CTRL2      0x61
#define TFTLCD_PART_IMG1_DISP_POS   0x80
#define TFTLCD_PART_IMG1_START_AD   0x81
#define TFTLCD_PART_IMG1_END_AD     0x82
#define TFTLCD_PART_IMG2_DISP_POS   0x83
#define TFTLCD_PART_IMG2_START_AD   0x84
#define TFTLCD_PART_IMG2_END_AD     0x85
#define TFTLCD_PANEL_IF_CTRL1       0x90
#define TFTLCD_PANEL_IF_CTRL2       0x92

//#define DATAPORT1 PORTD
//#define DATAPIN1  PIND
//#define DATADDR1  DDRD
//
//#define DATAPORT2 PORTB
//#define DATAPIN2  PINB
//#define DATADDR2  DDRB
//
//#define DATA1_MASK 0xFC  // top 6 bits
//#define DATA2_MASK 0x03  // bottom 2 bits
//
//#define MEGA_DATAPORT PORTA
//#define MEGA_DATAPIN  PINA
//#define MEGA_DATADDR  DDRA


// All pins appear to be active low

// PORT 1
#define RESET_PIN 31
#define CS_PIN    30
#define CD_PIN    29
#define WR_PIN    28
#define RD_PIN    23

#define SCRN_DELAY 1
volatile uint32_t sig_time_elapsed = 0;
//#define CS_ACTIVE *portOutputRegister(csport) &= ~cspin;
//#define CS_IDLE *portOutputRegister(csport) |= cspin;
//
//#define CS_ACTIVE   chipSelect.setLow();
//#define CS_IDLE     chipSelect.setHigh();
#define CS_ACTIVE   LPC_GPIO1->FIOCLR = (1 << CS_PIN);//delay_us(SCRN_DELAY);
#define CS_IDLE     LPC_GPIO1->FIOSET = (1 << CS_PIN);//delay_us(SCRN_DELAY);


//#define RD_IDLE *portOutputRegister(rdport) |= rdpin;
//#define RD_ACTIVE *portOutputRegister(rdport) &= ~rdpin;

//#define RD_IDLE     readSelect.setHigh();
//#define RD_ACTIVE   readSelect.setLow();
#define RD_IDLE     LPC_GPIO1->FIOSET = (1 << RD_PIN);//delay_us(SCRN_DELAY);
#define RD_ACTIVE   LPC_GPIO1->FIOCLR = (1 << RD_PIN);//delay_us(SCRN_DELAY);

//#define WR_IDLE *portOutputRegister(wrport) |= wrpin;
//#define WR_ACTIVE *portOutputRegister(wrport) &=~ wrpin;

//#define WR_IDLE     writeSelect.setHigh();
//#define WR_ACTIVE   writeSelect.setLow();
#define WR_IDLE     LPC_GPIO1->FIOSET = (1 << WR_PIN); sig_time_elapsed++; //for (sig_time_elapsed = 0; sig_time_elapsed < SCRN_DELAY; sig_time_elapsed++);//delay_us(SCRN_DELAY);
#define WR_ACTIVE   LPC_GPIO1->FIOCLR = (1 << WR_PIN); sig_time_elapsed++;//for (sig_time_elapsed = 0; sig_time_elapsed < SCRN_DELAY; sig_time_elapsed++);//delay_us(SCRN_DELAY);

//#define CD_DATA *portOutputRegister(cdport) |= cdpin;
//#define CD_COMMAND *portOutputRegister(cdport) &= ~cdpin;

//#define CD_DATA     comm_data.setHigh();
//#define CD_COMMAND  comm_data.setLow();
#define CD_DATA     LPC_GPIO1->FIOSET = (1 << CD_PIN);//delay_us(SCRN_DELAY);
#define CD_COMMAND  LPC_GPIO1->FIOCLR = (1 << CD_PIN);//delay_us(SCRN_DELAY);


#define RD_WR_IDLE  LPC_GPIO1->FIOSET = (1 << RD_PIN) | (1 << WR_PIN);
#define CS_WR_IDLE  LPC_GPIO1->FIOSET = (1 << CS_PIN) | (1 << WR_PIN);


//#include "glcdfont.c"
//#include <avr/pgmspace.h>
//#include "pins_arduino.h"
//#include "wiring_private.h"

  void TFTLCD::testPinsLow() {
      resetSelect.setLow();
      CS_ACTIVE
      RD_ACTIVE
      WR_ACTIVE
      CD_COMMAND
  }


  void TFTLCD::testPinsHigh() {
      resetSelect.setHigh();
      CS_IDLE
      RD_IDLE
      WR_IDLE
      CD_DATA
  }

  // resetSelect(P1_31), chipSelect(P1_30), comm_data(P1_29), writeSelect(P1_28), readSelect(P1_23)
  void TFTLCD::testControlSequence() {
      delay_ms(500);

      resetSelect.setHigh();
      delay_ms(500);

      resetSelect.setLow();
      CS_IDLE
      delay_ms(500);

      CS_ACTIVE
      CD_DATA
      delay_ms(500);

      CD_COMMAND
      WR_IDLE
      delay_ms(500);

      WR_ACTIVE
      RD_IDLE
      delay_ms(500);

      RD_ACTIVE
      delay_ms(500);
  }

  void TFTLCD::testDataSequence() {
      write8(0);
      delay_ms(500);
      write8(1);
      delay_ms(500);
      write8(2);
      delay_ms(500);
      write8(4);
      delay_ms(500);
      write8(8);
      delay_ms(500);
      write8(16);
      delay_ms(500);
      write8(32);
      delay_ms(500);
      write8(64);
      delay_ms(500);
      write8(128);
      delay_ms(500);
      write8(0);
      delay_ms(500);
  }

void TFTLCD::goHome(void) {
  goTo(0,0);
}

uint16_t TFTLCD::width(void) {
  return _width;
}
uint16_t TFTLCD::height(void) {
  return _height;
}

void TFTLCD::goTo(int x, int y) {
  if (rotation == 1 or rotation == 3){
    swap(x, y);
  }
  writeRegister(TFTLCD_GRAM_HOR_AD, x);     // GRAM Address Set (Horizontal Address) (R20h)
  writeRegister(TFTLCD_GRAM_VER_AD, y);     // GRAM Address Set (Vertical Address) (R21h)
  writeCommand(TFTLCD_RW_GRAM);            // Write Data to GRAM (R22h)
}

void TFTLCD::setCursor(uint16_t x, uint16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void TFTLCD::setTextSize(uint8_t s) {
  textsize = s;
}

void TFTLCD::setTextColor(uint16_t c) {
  textcolor = c;
}

//void TFTLCD::write(uint8_t c) {
//  if (c == '\n') {
//    cursor_y += textsize*8;
//    cursor_x = 0;
//  } else if (c == '\r') {
//    // skip em
//  } else {
//    drawChar(cursor_x, cursor_y, c, textcolor, textsize);
//    cursor_x += textsize*6;
//  }
//}

void TFTLCD::pushColors(uint16_t *data, uint8_t len,
			bool first) {
  uint16_t color;
//  uint8_t  hi, lo;

  if(first == true) {
    CS_ACTIVE
    CD_DATA
    RD_IDLE
    WR_IDLE
    setWriteDir();
  }

  while(len--) {
    color = *data++;
    writeData_unsafe(color);
  }
}

//void TFTLCD::drawString(uint16_t x, uint16_t y, char *c,
//			uint16_t color, uint8_t size) {
//  while (c[0] != 0) {
//    drawChar(x, y, c[0], color, size);
//    x += size*6;
//    c++;
//  }
//}

//// draw a character
//void TFTLCD::drawChar(uint16_t x, uint16_t y, char c,
//		      uint16_t color, uint8_t size)
//{
//  for (uint8_t i =0; i<5; i++ ) {
////      uint8_t line = pgm_read_byte(font+(c*5)+i);
//      uint8_t line = font[(c*5)+i];
//    for (uint8_t j = 0; j<8; j++) {
//      if (line & 0x1) {
//	if (size == 1) // default size
//	  drawPixel(x+i, y+j, color);
//	else {  // big size
//	  fillRect(x+i*size, y+j*size, size, size, color);
//	}
//      }
//      line >>= 1;
//    }
//  }
//}


// draw a triangle!
void TFTLCD::drawTriangle(uint16_t x0, uint16_t y0,
			  uint16_t x1, uint16_t y1,
			  uint16_t x2, uint16_t y2,
			  uint16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color); 
}

void TFTLCD::fillTriangle ( int32_t x0, int32_t y0,
			    int32_t x1, int32_t y1,
			    int32_t x2, int32_t y2,
			    uint16_t color)
{
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  int32_t dx1, dx2, dx3; // Interpolation deltas
  int32_t sx1, sx2, sy; // Scanline co-ordinates

  sx2=(int32_t)x0 * (int32_t)1000; // Use fixed point math for x axis values
  sx1 = sx2;
  sy=y0;

  // Calculate interpolation deltas
  if (y1-y0 > 0) dx1=((x1-x0)*1000)/(y1-y0);
    else dx1=0;
  if (y2-y0 > 0) dx2=((x2-x0)*1000)/(y2-y0);
    else dx2=0;
  if (y2-y1 > 0) dx3=((x2-x1)*1000)/(y2-y1);
    else dx3=0;

  // Render scanlines (horizontal lines are the fastest rendering method)
  if (dx1 > dx2)
  {
    for(; sy<=y1; sy++, sx1+=dx2, sx2+=dx1)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx2 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx2, sx2+=dx3)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
  else
  {
    for(; sy<=y1; sy++, sx1+=dx1, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
    sx1 = x1*1000;
    sy = y1;
    for(; sy<=y2; sy++, sx1+=dx3, sx2+=dx2)
    {
      drawHorizontalLine(sx1/1000, sy, (sx2-sx1)/1000, color);
    }
  }
}

uint16_t TFTLCD::Color565(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;

  return c;
}

// draw a rectangle
void TFTLCD::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
		      uint16_t color) {
  // smarter version
  drawHorizontalLine(x, y, w, color);
  drawHorizontalLine(x, y+h-1, w, color);
  drawVerticalLine(x, y, h, color);
  drawVerticalLine(x+w-1, y, h, color);
}

// draw a rounded rectangle
void TFTLCD::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color) {
  // smarter version
  drawHorizontalLine(x+r, y, w-2*r, color);
  drawHorizontalLine(x+r, y+h-1, w-2*r, color);
  drawVerticalLine(x, y+r, h-2*r, color);
  drawVerticalLine(x+w-1, y+r, h-2*r, color);
  // draw four corners
  drawCircleHelper(x+r, y+r, r, 1, color);
  drawCircleHelper(x+w-r-1, y+r, r, 2, color);
  drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
  drawCircleHelper(x+r, y+h-r-1, r, 8, color);
}


// fill a rounded rectangle
void TFTLCD::fillRoundRect(uint16_t x, uint16_t y,
			   uint16_t w, uint16_t h, uint16_t r,
			   uint16_t color)
{
  // smarter version
  fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  fillCircleHelper(x+r, y+r, r, 2, h-2*r-1, color);
}

// fill a circle
void TFTLCD::fillCircle(uint16_t x0, uint16_t y0,
			uint16_t r, uint16_t color)
{
  writeRegister(TFTLCD_ENTRY_MOD, 0x1030);
  drawVerticalLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}


// used to do circles and roundrects!
void TFTLCD::fillCircleHelper(uint16_t x0, uint16_t y0,
			      uint16_t r, uint8_t cornername,
			      uint16_t delta, uint16_t color)
{

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    if (cornername & 0x1) {
      drawVerticalLine(x0+x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawVerticalLine(x0-x, y0-y, 2*y+1+delta, color);
      drawVerticalLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}


// draw a circle outline

void TFTLCD::drawCircle(uint16_t x0, uint16_t y0, uint16_t r, 
			uint16_t color) {
  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  drawCircleHelper(x0, y0, r, 0xF, color);
}

void TFTLCD::drawCircleHelper(uint16_t x0, uint16_t y0,
			      uint16_t r, uint8_t cornername,
			      uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

// fill a rectangle
void TFTLCD::fillRect(uint16_t x, uint16_t y,
		      uint16_t w, uint16_t h, 
		      uint16_t fillcolor) {
  // smarter version
  while (h--)
    drawHorizontalLine(x, y++, w, fillcolor);
}


void TFTLCD::drawVerticalLine(uint16_t x, uint16_t y,
			      uint16_t length, uint16_t color)
{
  if (x >= _width) return;
  drawFastLine(x,y,length,color,1);
}

void TFTLCD::drawHorizontalLine(uint16_t x, uint16_t y,
				uint16_t length, uint16_t color)
{
  if (y >= _height) return;
  drawFastLine(x,y,length,color,(uint8_t)0);
}

void TFTLCD::drawFastLine(uint16_t x, uint16_t y, uint16_t length, 
			  uint16_t color, uint8_t rotflag)
{
  uint16_t newentrymod;

  switch (rotation) {
  case 0:
    if (rotflag)
      //      newentrymod = 0x1028;   // we want a 'vertical line'1028 = 4136   1000000101000
      newentrymod = 0x1038;   // we want a 'vertical line'1028 = 4136   1000000101000
    else 
      newentrymod = 0x1030;   // we want a 'horizontal line'1030 = 4144 1000000110000
    break;
  case 1:
    swap(x, y);
    // first up fix the X
    x = TFTWIDTH - x - 1;//
    if (rotflag)
      newentrymod = 0x1000;   // we want a 'vertical line'1000
    else 
      newentrymod = 0x1028;   // we want a 'horizontal line'1028
    break;
  case 2:
    x =  TFTWIDTH - x - 1;
    y =  TFTHEIGHT - y - 1;
    if (rotflag)
      newentrymod = 0x1008;   // we want a 'vertical line'1008
    else 
      newentrymod = 0x1020;   // we want a 'horizontal line'1020
    break;
  case 3:
    swap(x,y);
    y = TFTHEIGHT - y - 1;//TFTHEIGHT
    if (rotflag)
      newentrymod = 0x1030;   // we want a 'vertical line'1030
    else 
      newentrymod = 0x1008;   // we want a 'horizontal line'1008
    break;
  default:
      newentrymod = 0x0000;
      printf("error!");

  }
 
  writeRegister(TFTLCD_ENTRY_MOD, newentrymod);
  writeRegister(TFTLCD_GRAM_HOR_AD, x); // GRAM Address Set (Horizontal Address) (R20h)
  writeRegister(TFTLCD_GRAM_VER_AD, y); // GRAM Address Set (Vertical Address) (R21h)
  writeCommand(TFTLCD_RW_GRAM);  // Write Data to GRAM (R22h)

  CS_ACTIVE
  CD_DATA
  RD_IDLE
  WR_IDLE

  setWriteDir();
  while (length--) {
    writeData_unsafe(color); 
  }

  // set back to default
  CS_IDLE
  //  writeRegister(TFTLCD_ENTRY_MOD, 0x1028);
  writeRegister(TFTLCD_ENTRY_MOD, 0x1030);
}



// bresenham's algorithm - thx wikpedia
void TFTLCD::drawLine(int16_t x0, int16_t y0,
		      int16_t x1, int16_t y1, 
		      uint16_t color) 
{
  // if you're in rotation 1 or 3, we need to swap the X and Y's

  static int16_t steep;
  steep = abs(y1 - y0) > abs(x1 - x0);

  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  //dy = abs(y1 - y0);
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void TFTLCD::fillScreen(uint16_t color) 
{
  goHome();
  static uint32_t i;
  
  i = _height;
  i *= _width;
  
  CS_ACTIVE
  CD_DATA
  RD_IDLE
  WR_IDLE

  setWriteDir();
  while (i--) {
    writeData_unsafe(color); 
  }

//  CS_IDLE // trying to leave this out of the routines im using to speed things up
  // this is based on the assumption that since there is only 1 screen the
  // Chip Select can just stay active always
}

void TFTLCD::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  // check rotation, move pixel around if necessary
  switch (rotation) {
  case 1:
    swap(x, y);
    x = TFTWIDTH - x - 1;
    break;
  case 2:
    x = TFTWIDTH - x - 1;
    y = TFTHEIGHT - y - 1;
    break;
  case 3:
    swap(x, y);
    y = TFTHEIGHT - y - 1;
    break;
  }
    
  if ((x >= TFTWIDTH) || (y >= TFTHEIGHT)) return;
  writeRegister(TFTLCD_GRAM_HOR_AD, x);
  writeRegister(TFTLCD_GRAM_VER_AD, y);
  writeCommand(TFTLCD_RW_GRAM);
  writeData(color);
}

#define   DELAY  0xaf  // unused ST7781 command

//static const uint16_t _regValues[] =
//{
//     0x01, 0x0100 ,
//     0x02, 0x0700 ,
//     0x03, 0x1030 ,
//     0x04, 0x0000 ,
//     0x08, 0x0302 ,
//     0x0A, 0x0000 ,
//     0x0C, 0x0000 ,
//     0x0D, 0x0000 ,
//     0x0F, 0x0000 ,
//
//     DELAY,  100  ,
//
//     0x30, 0x0000 ,
//     0x31, 0x0405 ,
//     0x32, 0x0203 ,
//     0x35, 0x0004 ,
//     0x36, 0x0B07 ,
//     0x37, 0x0000 ,
//     0x38, 0x0405 ,
//     0x39, 0x0203 ,
//     0x3c, 0x0004 ,
//     0x3d, 0x0B07 ,
//     0x20, 0x0000 ,
//     0x21, 0x0000 ,
//     0x50, 0x0000 ,
//     0x51, 0x00ef ,
//     0x52, 0x0000 ,
//     0x53, 0x013f ,
//
//     DELAY,  100  ,
//
//     0x60, 0xa700 ,
//     0x61, 0x0001 ,
//     0x90, 0x0033 , // changed from 0x90, 0x003A
//     0x95, 0x021E ,
//     0x80, 0x0000 ,
//     0x81, 0x0000 ,
//     0x82, 0x0000 ,
//     0x83, 0x0000 ,
//     0x84, 0x0000 ,
//     0x85, 0x0000 ,
//     0xFF, 0x0001 ,
//     0xB0, 0x140D ,
//     0xFF, 0x0000 ,
//
//     DELAY,  100  ,
//
//     0x07, 0x0133 ,
//
//     DELAY,  50   ,
//
//     0x10, 0x14e0 ,
//
//     DELAY,  100  ,
//
//     0x07, 0x0133 ,
//};

static const uint16_t _regValues[] = {
TFTLCD_DRIV_OUT_CTRL, 0x0100,
TFTLCD_DRIV_WAV_CTRL, 0x0700,
TFTLCD_ENTRY_MOD,  0x1030,
TFTLCD_DISP_CTRL2, 0x0302,
TFTLCD_DISP_CTRL3, 0x0000,
TFTLCD_DISP_CTRL4, 0x0008,

//*******POWER CONTROL REGISTER INITIAL*******//
TFTLCD_POW_CTRL1, 0x0790,
TFTLCD_POW_CTRL2, 0x0005,
TFTLCD_POW_CTRL3, 0x0000,
TFTLCD_POW_CTRL4, 0x0000,

 //delayms(50,
//********POWER SUPPPLY STARTUP 1 SETTING*******//
TFTLCD_POW_CTRL1, 0x12B0,
// delayms(50,
 TFTLCD_POW_CTRL2, 0x0007,
 //delayms(50,
//********POWER SUPPLY STARTUP 2 SETTING******//
TFTLCD_POW_CTRL3, 0x008C,
TFTLCD_POW_CTRL4, 0x1700,
TFTLCD_VCOMH_CTRL, 0x0022,
// delayms(50,
//******GAMMA CLUSTER SETTING******//
TFTLCD_GAMMA_CTRL1, 0x0000,
TFTLCD_GAMMA_CTRL2, 0x0505,
TFTLCD_GAMMA_CTRL3, 0x0205,
TFTLCD_GAMMA_CTRL4, 0x0206,
TFTLCD_GAMMA_CTRL5, 0x0408,
TFTLCD_GAMMA_CTRL6, 0x0000,
TFTLCD_GAMMA_CTRL7, 0x0504,
TFTLCD_GAMMA_CTRL8, 0x0206,
TFTLCD_GAMMA_CTRL9, 0x0206,
TFTLCD_GAMMA_CTRL10, 0x0408,
// -----------DISPLAY WINDOWS 240*320-------------//
TFTLCD_HOR_START_AD, 0x0000,
TFTLCD_HOR_END_AD, 0x00EF,
TFTLCD_VER_START_AD, 0x0000,
TFTLCD_VER_END_AD, 0x013F,
//-----FRAME RATE SETTING-------//
TFTLCD_GATE_SCAN_CTRL1, 0xA700,
TFTLCD_GATE_SCAN_CTRL2, 0x0001,
TFTLCD_PANEL_IF_CTRL1, 0x0033, //RTNI setting
//-------DISPLAY ON------//
TFTLCD_DISP_CTRL1, 0x0133,
};

void TFTLCD::initDisplay(void) {
  uint16_t addr, data;
  uint16_t regSize =  sizeof(_regValues) / 2;
  reset();
  
  for (uint8_t i = 0; i < regSize; i += 2) {

      addr = _regValues[i];
      data = _regValues[i+1];

      if(addr == DELAY) {
          delay_ms(data);
          printf("DELAY\n");
      } else {
          writeRegister(addr, data);
          delay_ms(1);
//          printf("Reg#: %3d RegSize: %3d addr: 0x%04x data: 0x%04x\n",i, regSize,addr,data);
      }



  }
}

void TFTLCD::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
  if (rotation == 1 or rotation == 3){
    swap(x0, y0);
    swap(x1, y1);
  }
  writeRegister(TFTLCD_HOR_START_AD, x0);
  writeRegister(TFTLCD_HOR_END_AD, x1);
  writeRegister(TFTLCD_VER_START_AD, y0);
  writeRegister(TFTLCD_VER_END_AD, y1);
  writeCommand(TFTLCD_RW_GRAM);
  return;
}

uint8_t TFTLCD::getRotation(void) {
  return rotation;
}

void TFTLCD::setRotation(uint8_t x) {
  
  x %= 4;  // cant be higher than 3
  rotation = x;
  switch (x) {
  case 0: // up
    writeRegister(TFTLCD_ENTRY_MOD, 0x1030);
    _width = TFTWIDTH; 
    _height = TFTHEIGHT;
    break;
  case 1: // right 
    writeRegister(TFTLCD_ENTRY_MOD, 0x1028);
    _width = TFTHEIGHT; 
    _height = TFTWIDTH;
    break;
  case 2: // down
    writeRegister(TFTLCD_ENTRY_MOD, 0x1010);
    _width = TFTWIDTH; 
    _height = TFTHEIGHT;
    break;
  case 3: //  left
    writeRegister(TFTLCD_ENTRY_MOD, 0x1018);
    _width = TFTHEIGHT; 
    _height = TFTWIDTH;
    break;
 }
}

/********************************* low level pin initialization */

TFTLCD::TFTLCD(uint8_t cs, uint8_t cd, uint8_t wr,
	       uint8_t rd, uint8_t rst) : resetSelect(P1_31), chipSelect(P1_30), comm_data(P1_29), writeSelect(P1_28), readSelect(P1_23)  {
  
  resetSelect.setHigh();
  resetSelect.setAsOutput();

  chipSelect.setHigh();
  chipSelect.setAsOutput();

  comm_data.setHigh();
  comm_data.setAsOutput();

  writeSelect.setHigh();
  writeSelect.setAsOutput();

  readSelect.setHigh();
  readSelect.setAsOutput();



  rotation = 0;
  _width = TFTWIDTH;
  _height = TFTHEIGHT;

  // configure LCD data bus GPIO pins for output on P2.0 to P2.7
  LPC_PINCON->PINSEL4 &= ~(0xFFFF);
  LPC_PINCON->PINMODE4 &= ~(0xFFFF);

  // set them to write direction
  setWriteDir();

  cursor_y = cursor_x = 0;
  textsize = 1;
  textcolor = 0xFFFF;

  CS_ACTIVE // shortcut incase i can remove it from all the write commands.
}


/********************************** low level pin interface */

void TFTLCD::reset(void) {

    resetSelect.setLow();
    delay_ms(2);
    resetSelect.setHigh();
  // resync
  writeData(0);
  writeData(0);
  writeData(0);  
  writeData(0);
}

 inline void TFTLCD::setWriteDir(void) {

  LPC_GPIO2->FIODIR |= 0xFF; // sets pins 0-7 on Port 2 to output
//  LPC_GPIO0->FIODIR |= 0x04; // sets pins 0-1 on Port 0 to output


}

inline void TFTLCD::setReadDir(void) {

  LPC_GPIO2->FIODIR &= ~(0xFF); // sets pins 0-7 on Port 2 to input

}

inline void TFTLCD::write8(uint8_t d) {

    LPC_GPIO2->FIOCLR = 0xFF;
    LPC_GPIO2->FIOSET = d;

}

inline uint8_t TFTLCD::read8(void) {
 uint8_t d;

 // this should automatically truncate all values for pins above 7
 // which is what we want since we are only interested in 0-7 on P2
 d = LPC_GPIO2->FIOPIN;

 return d;
}

/********************************** low level readwrite interface */

// the C/D pin is high during write
inline void TFTLCD::writeData(uint16_t data) {

//  CS_ACTIVE // beign lazy to speed things up. this is initialize in the constructor
  CD_DATA
//  RD_IDLE
//  WR_IDLE
  RD_WR_IDLE

  //  setWriteDir(); // read operation is never used so write direction never changes. so we dont need to call it each time
  write8(data >> 8);

  WR_ACTIVE
  WR_IDLE

  write8(data);

  WR_ACTIVE


  WR_IDLE
//  CS_IDLE
//  CS_WR_IDLE



}

// this is a 'sped up' version, with no direction setting, or pin initialization
// not for external usage, but it does speed up stuff like a screen fill
inline void TFTLCD::writeData_unsafe(uint16_t data) {

  write8(data >> 8);

  WR_ACTIVE


  WR_IDLE


  write8(data);

  WR_ACTIVE


  WR_IDLE


}



// the C/D pin is low during write
inline void TFTLCD::writeCommand(uint16_t cmd) {

//  CS_ACTIVE // beign lazy to speed things up. this is initialize in the constructor
  CD_COMMAND
//  RD_IDLE
//  WR_IDLE
  RD_WR_IDLE

//  setWriteDir(); // read operation is never used so write direction never changes. so we dont need to call it each time
  write8((uint8_t)(cmd >> 8));

  WR_ACTIVE

  WR_IDLE


  write8((uint8_t)cmd);

  WR_ACTIVE

  WR_IDLE
//  CS_IDLE
//  CS_WR_IDLE

}

inline uint16_t TFTLCD::readData() {
  uint16_t d = 0;

  CS_ACTIVE
  CD_DATA
  RD_IDLE
  WR_IDLE
  
  setReadDir();

  RD_ACTIVE

  // delayMicroseconds(10);

  d = read8();
  d <<= 8;

  RD_IDLE
  RD_ACTIVE

  //delayMicroseconds(10);


  d |= read8();

  RD_IDLE  
  CS_IDLE
   
  return d;
}


/************************************* medium level data reading/writing */

inline uint16_t TFTLCD::readRegister(uint16_t addr) {
   writeCommand(addr);
   return readData();
}

inline void TFTLCD::writeRegister(uint16_t addr, uint16_t data) {
   writeCommand(addr);
   writeData(data);
}
