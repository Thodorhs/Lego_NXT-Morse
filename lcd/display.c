#include "../AT91SAM7S256.h"
#include "display.h"
#include "../lcd/spi.h"
#include <string.h>

#define   DISPLAY_HEIGHT      64        // Y pixels
#define   DISPLAY_WIDTH       100       // X pixels

#define	  CMD		0
#define	  DAT		1
#define   DISP_LINES    (DISPLAY_HEIGHT/8)

static my_abs(int x){
  if(x<0)
    return x*(-1);
  return x;
}

static struct {
  UBYTE   *Display;
  UBYTE   DataArray[DISPLAY_HEIGHT / 8][DISPLAY_WIDTH];
} IOMapDisplay;

UBYTE DisplayInitCommands[] =
{
  0xEB, // LCD bias: 1/9=0xEB
  0x2F, // pump control: set build-in=0x2F
  0xA4, // all pixels: off=0xA4, on=0xA5
  0xA6, // inverse: off=0xA6, on=0xA7
  0x40, // set scroll line: 0=0x40-63=0x7F
  0x81, // set Vbias potentiometer (2-byte command): 0x81 
  0x5A, //      -"-         		             : on=0x5A, off=0x00
  0xC4, // LCD mapping: regular=0xC4, row-mirror=bit 2, col-mirror=bit 3, e.g. col-mirror=0xC2
  0x27, // set temp comp.: -0.20%/C=0x27
  0x29, // panel loading: <=15nF=0x28, >15nF=0x29
  0xA0, // framerate: 76fps=0xA0, 95fps=0xA1
  0x88, // RAM address control: no wrap around+no autoincremet=0x88
  0x23, // set multiplex rate: 1:65=0x23
  0xAF  // set display: on=0xAF, off=0xAE
};

typedef struct
{
  UBYTE   ItemPixelsX;
  UBYTE   ItemPixelsY;
  UBYTE   Data[];
} __attribute__((__packed__)) FONT, ICON;



const ICON Font = {
  // each character is 6x8 pixels represented as 6 bytes, where each byte is a "column" of 8 pixels
  0x06,      // Graphics Width
  0x08,      // Graphics Height
  {/* 32 first non-printable characters */
  0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x7F,0x3E,0x1C,0x08, 0x08,0x1C,0x3E,0x7F,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,
  /* rest printable characters */
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x5F,0x06,0x00,0x00,0x07,0x03,0x00,0x07,0x03,0x00,0x24,0x7E,0x24,0x7E,0x24,0x00,0x24,0x2B,0x6A,0x12,0x00,0x00,0x63,0x13,0x08,0x64,0x63,0x00,0x30,0x4C,0x52,0x22,0x50,0x00,0x00,0x07,0x03,0x00,0x00,0x00,0x00,0x3E,0x41,0x00,0x00,0x00,0x00,0x41,0x3E,0x00,0x00,0x00,0x08,0x3E,0x1C,0x3E,0x08,0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x80,0x60,0x60,0x00,0x00,0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,0x00,
  0x3E,0x51,0x49,0x45,0x3E,0x00,0x00,0x42,0x7F,0x40,0x00,0x00,0x62,0x51,0x49,0x49,0x46,0x00,0x22,0x49,0x49,0x49,0x36,0x00,0x18,0x14,0x12,0x7F,0x10,0x00,0x2F,0x49,0x49,0x49,0x31,0x00,0x3C,0x4A,0x49,0x49,0x30,0x00,0x01,0x71,0x09,0x05,0x03,0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x06,0x49,0x49,0x29,0x1E,0x00,0x00,0x6C,0x6C,0x00,0x00,0x00,0x00,0xEC,0x6C,0x00,0x00,0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x24,0x24,0x24,0x24,0x24,0x00,0x00,0x41,0x22,0x14,0x08,0x00,0x02,0x01,0x59,0x09,0x06,0x00,
  0x3E,0x41,0x5D,0x55,0x1E,0x00,0x7E,0x11,0x11,0x11,0x7E,0x00,0x7F,0x49,0x49,0x49,0x36,0x00,0x3E,0x41,0x41,0x41,0x22,0x00,0x7F,0x41,0x41,0x41,0x3E,0x00,0x7F,0x49,0x49,0x49,0x41,0x00,0x7F,0x09,0x09,0x09,0x01,0x00,0x3E,0x41,0x49,0x49,0x7A,0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00,0x41,0x7F,0x41,0x00,0x00,0x30,0x40,0x40,0x40,0x3F,0x00,0x7F,0x08,0x14,0x22,0x41,0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x7F,0x02,0x04,0x02,0x7F,0x00,0x7F,0x02,0x04,0x08,0x7F,0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,
  0x7F,0x09,0x09,0x09,0x06,0x00,0x3E,0x41,0x51,0x21,0x5E,0x00,0x7F,0x09,0x09,0x19,0x66,0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x1F,0x20,0x40,0x20,0x1F,0x00,0x3F,0x40,0x3C,0x40,0x3F,0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x07,0x08,0x70,0x08,0x07,0x00,0x71,0x49,0x45,0x43,0x00,0x00,0x00,0x7F,0x41,0x41,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00,0x41,0x41,0x7F,0x00,0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x80,0x80,0x80,0x80,0x80,0x00,
  0x00,0x02,0x05,0x02,0x00,0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x7F,0x44,0x44,0x44,0x38,0x00,0x38,0x44,0x44,0x44,0x28,0x00,0x38,0x44,0x44,0x44,0x7F,0x00,0x38,0x54,0x54,0x54,0x08,0x00,0x08,0x7E,0x09,0x09,0x00,0x00,0x18,0x24,0xA4,0xA4,0xFC,0x00,0x7F,0x04,0x04,0x78,0x00,0x00,0x00,0x00,0x7D,0x40,0x00,0x00,0x40,0x80,0x84,0x7D,0x00,0x00,0x7F,0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x7F,0x40,0x00,0x00,0x7C,0x04,0x18,0x04,0x78,0x00,0x7C,0x04,0x04,0x78,0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00,
  0xFC,0x44,0x44,0x44,0x38,0x00,0x38,0x44,0x44,0x44,0xFC,0x00,0x44,0x78,0x44,0x04,0x08,0x00,0x08,0x54,0x54,0x54,0x20,0x00,0x04,0x3E,0x44,0x24,0x00,0x00,0x3C,0x40,0x20,0x7C,0x00,0x00,0x1C,0x20,0x40,0x20,0x1C,0x00,0x3C,0x60,0x30,0x60,0x3C,0x00,0x6C,0x10,0x10,0x6C,0x00,0x00,0x9C,0xA0,0x60,0x3C,0x00,0x00,0x64,0x54,0x54,0x4C,0x00,0x00,0x08,0x3E,0x41,0x41,0x00,0x00,0x00,0x00,0x77,0x00,0x00,0x00,0x00,0x41,0x41,0x3E,0x08,0x00,0x02,0x01,0x02,0x01,0x00,0x00,0x10,0x20,0x40,0x38,0x07,0x00}
};

const ICON Logo = {
  20,
  24,
  {
  /* LOGO T */
  0x00, 0x00, 0x00,
  0x3E, 0x00, 0x00,
  0x0E, 0x00, 0x00,
  0x06, 0x00, 0x00,
  0x06, 0x00, 0x00,
  0x06, 0x00, 0x00,
  0x06, 0x00, 0x1C,
  0x16, 0x00, 0x18,
  0x2E, 0x00, 0x10,

  0xFE, 0xFF, 0x1F,
  0xFE, 0xFF, 0x1F,

  0x2E, 0x00, 0x10,
  0x16, 0x00, 0x18,
  0x06, 0x00, 0x1C,
  0x06, 0x00, 0x00,
  0x06, 0x00, 0x00,
  0x06, 0x00, 0x00,
  0x0E, 0x00, 0x00,
  0x3E, 0x00, 0x00,
  0x00, 0x00, 0x00,
  /* LOGO D */
  0x00, 0x00, 0x00,
  0xFE, 0x3F, 0x1F,
  0xFE, 0x7F, 0x1E,
  0xFE, 0xFF, 0x1C,
  0x0E, 0x00, 0x18,
  0x0E, 0x00, 0x10,
  0x0E, 0x00, 0x04,
  0x0E, 0x00, 0x0C,
  0x0E, 0x00, 0x1C,
  0x0E, 0x00, 0x1C,
  0x0E, 0x00, 0x1C,
  0x0E, 0x00, 0x1C,
  0x0E, 0x00, 0x1C,

  0x1E, 0x00, 0x1E,
  0x3C, 0x00, 0x0F,
  0x78, 0x80, 0x07,
  0xF0, 0xC0, 0x03,
  0xE0, 0xFF, 0x01,
  0x80, 0x7F, 0x00,
  0x00, 0x00, 0x00,
  /* LOGO A */
  0x00, 0x00, 0x00,

  0x00, 0xFC, 0x1F,
  0x80, 0xFF, 0x1F,
  0xE0, 0xFF, 0x1F,
  0xF0, 0x3F, 0x00,
  0xF8, 0x3F, 0x00,
  0xFC, 0x38, 0x00,
  0x3E, 0x38, 0x00,
  0x1E, 0x38, 0x00,

  0x3E, 0x38, 0x00,
  0xFC, 0x38, 0x00,
  0xF8, 0x3F, 0x00,
  0xF0, 0x3F, 0x00,
  0xE0, 0xFF, 0x1F,
  0x80, 0xFF, 0x1F,
  0x00, 0xFC, 0x1F,

  0x00, 0x00, 0x00,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0x00
  }
};

UBYTE DisplayLineString[DISP_LINES][3] =
{
  { 0xB0,0x10,0x00 },
  { 0xB1,0x10,0x00 },
  { 0xB2,0x10,0x00 },
  { 0xB3,0x10,0x00 },
  { 0xB4,0x10,0x00 },
  { 0xB5,0x10,0x00 },
  { 0xB6,0x10,0x00 },
  { 0xB7,0x10,0x00 }
};



void DisplayMenu(UBYTE cmd, UBYTE** words, UBYTE len){
  UBYTE x, y;
  UBYTE i;

  y = ((float)DISPLAY_HEIGHT/2) - 10*((float)len/2);
  for(i = 0; i < len; i++){
    x = ((float)DISPLAY_WIDTH/2) - Font.ItemPixelsX*((float)strlen((char*)words[i])/2) - 1;
    DisplayString(cmd, x, y, (UBYTE*)words[i]);
    y = y + 10;
  }
 
  return;
}


void DisplaySelectWord(UBYTE cmd, UBYTE** words, UBYTE len, UBYTE selected_word_index){
  UBYTE x, y;
  if(words == NULL || len == 1)
    return;
  x = ((float)DISPLAY_WIDTH/2) - Font.ItemPixelsX*((float)strlen((char*)words[selected_word_index])/2) - 1 - 8;
  y = ((float)DISPLAY_HEIGHT/2) - 10*((float)len/2) + (10*selected_word_index);
  DisplayChar(cmd, x, y, LEFT_ARROW);

  x = ((float)DISPLAY_WIDTH/2) - Font.ItemPixelsX*((float)strlen((char*)words[selected_word_index])/2) - 1 - 2;
  x += strlen((char*)words[selected_word_index]) * Font.ItemPixelsX + 3;
  DisplayChar(cmd, x, y, RIGHT_ARROW);
  return;
}


void DisplayLogo(UBYTE letter, UBYTE cmd, UBYTE X, UBYTE Y){
  int i, j, k;
  const ICON *pLogo = &Logo;
  UBYTE *pSource = (UBYTE*)&pLogo->Data[letter * Logo.ItemPixelsX * 3];

  for(i=0; i<Logo.ItemPixelsX; i++){
    j = 0;
    for(k=0; k<Logo.ItemPixelsY; k++){

      if(((*pSource) & (1<<j)) && cmd == SET)
        DisplaySetPixel(SET, X, Y+k);
      else
        DisplaySetPixel(CLEAR, X, Y+k);

      if(j == 7){
        pSource++;
        j = 0;
      }else
        j++;
    }

    X++;
  }

  return;
}


void DisplayInit(void){
  SPIInit();
  return;
}

void DisplayExit(void){

  return;
}

void DisplayErase(void){
  UBYTE i, j;
  for(i = 0; i<DISPLAY_HEIGHT; i++)
    for(j = 0; j<DISPLAY_WIDTH; j++)
      IOMapDisplay.DataArray[i][j] = 0;
  return;
}

UBYTE DisplayWrite(UBYTE type, UBYTE *buf, UWORD len){
  if(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY){
    if(!type)
      SPIPIOClearData();
    else
      SPIPIOSetData();
  
    SPIWriteDMA(buf, len);
    return 1;
  }
  return 0;
}

void DisplayUpdateSync(void){
  UBYTE i;
  UBYTE *pImage = (UBYTE*)IOMapDisplay.DataArray;
  while(!DisplayWrite(CMD, DisplayInitCommands, sizeof(DisplayInitCommands)));

  for(i=0; i<8; i++){
    /* SEND COMMAND TO LCD */
    while(!DisplayWrite(CMD, DisplayLineString[i], 3));

    /* SEND DATA TO DISPLAY */
    while(!DisplayWrite(DAT, &pImage[i * DISPLAY_WIDTH], DISPLAY_WIDTH));
  }
  return;
}



void DisplayChar(UBYTE cmd, UBYTE X,UBYTE Y,UBYTE Char){
  int i, j;
  const ICON *pFont = &Font;
  UBYTE *pSource = (UBYTE*)&pFont->Data[Char*Font.ItemPixelsX];
  
  for(i=0; i<Font.ItemPixelsX; i++){
    for(j=0; j<Font.ItemPixelsY; j++){
      if(((*pSource) & (1<<j)) && cmd == SET){
        DisplaySetPixel(SET, X, Y+j);
      }else{
        DisplaySetPixel(CLEAR, X, Y+j);
      }
    }
    X++;
    pSource++;
  }

  return;
}



void DisplayNum(UBYTE cmd, UBYTE X,UBYTE Y,ULONG Num){
  UWORD t1=0;
  UBYTE t2=0,ch=0;
  static UBYTE digits=0,i=0;
  t1 = Num;
  t2 = t1%10;
  t1 = t1/10;
  if(t1>0){
	  digits++;
    DisplayNum(cmd, X,Y,t1);
  }else{
	  i = digits;
  }
  ch = t2 + '0';
  DisplayChar(cmd, X + (Font.ItemPixelsX*(digits-i--)), Y, ch);  
  return;
}


void DisplayString(UBYTE cmd, UBYTE X,UBYTE Y,UBYTE *pString){
  UBYTE i;
  for(i=0; *pString!='\0'; i++){
    DisplayChar(cmd, X, Y, *pString);
    X += Font.ItemPixelsX;
    pString++;
  }
  return;
}

void DisplaySetPixel(UBYTE cmd, UBYTE X,UBYTE Y){
  UBYTE page = Y/8;
  UBYTE bit_in_byte = Y%8;
  UBYTE mask;
  if(cmd == SET){
    mask = 1<<bit_in_byte;
    IOMapDisplay.DataArray[page][X] |= mask;
  }else{
    mask = ~(1 << bit_in_byte);
    IOMapDisplay.DataArray[page][X] &= mask;
  }
  return; 
}


void DisplayLineX(UBYTE cmd, UBYTE X1, UBYTE X2, UBYTE Y){
  UBYTE i;
  for(i = X1; i<=X2; i++)
    DisplaySetPixel(cmd, i, Y);
  return;
}

void DisplayLineY(UBYTE cmd, UBYTE X, UBYTE Y1, UBYTE Y2){
  UBYTE i;
  for(i = Y1; i<=Y2; i++)
    DisplaySetPixel(cmd, X, i);
  return;
}




void draw_line_h(int cmd, int _x0, int _y0, int _x1, int _y1){
    int dx = _x1-_x0;
    int dy = _y1-_y0;
    int xi =1;
    int D;
    int _x;
    int _y;

    if(dx <0){
        xi =-1;
        dx = -dx;
    }
    D = (2*dx)-dy;
    _x = _x0;
    for(_y = _y0; _y<_y1; _y++){
        DisplaySetPixel(cmd, _x, _y);
        if(D>0){
            _x = _x+xi;
            D = D+(2*(dx-dy));
        }else{
            D = D +2*dx;
        }
    }
    return;
}


void draw_line_l(int cmd, int _x0, int _y0, int _x1, int _y1){
    int dx = _x1-_x0;
    int dy = _y1-_y0;
    int yi =1;
    int _x;
    int D;
    int _y;

    if(dy <0){
        yi =-1;
        dy = -dy;
    }
    D = (2*dy)-dx;
    _y = _y0;
    for(_x = _x0; _x<_x1; _x++){
        DisplaySetPixel(cmd, _x, _y);
        if(D>0){
            _y = _y+yi;
            D = D+(2*(dy-dx));
        }else{
            D = D +2*dy;
        }
    }
    return;
}

/* SET IN COMMAND TO SET THE LINE. CLEAR IN COMMAND TO CLEAR THE LINE */
void draw_line(int cmd, int weight, int _x0, int _y0, int _x1, int _y1){
    int i;
    for(i=0; i<weight; i++){

        if(my_abs(_y1-_y0) < my_abs(_x1-_x0)){
            if(_x0>_x1)
                draw_line_l(cmd, _x1,_y1,_x0,_y0);
            else
                draw_line_l(cmd, _x0,_y0,_x1,_y1);
            
        }else{
            if(_y0>_y1)
                draw_line_h(cmd, _x1,_y1,_x0,_y0);
            else
                draw_line_h(cmd, _x0,_y0,_x1,_y1);
            
        }
        if(_y0 == _y1){           /* HORIZONTAL */
            _y0++;
            _y1++;
        }else{
            _x0++;
            _x1++;
        }
        
    }   
    return;
}



void DisplayWelcomeFrame(UBYTE cmd, UBYTE X, UBYTE Y, UBYTE height, UBYTE width){
    UBYTE first_pix = (float)width*(float)55/100;
    UBYTE mid_pix = (float)width*(float)30/100;
    UBYTE x0, x1, y0, y1, x, y;

    // 2 HORIONTAL BORDER LINES
    x0 = X + 2;
    x1 = X + width - 2;
    y = Y;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);


    x1 = X + first_pix;
    y = Y + height;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y - 1);


    x0 = X + first_pix + mid_pix;
    x1 = X + width - 2;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y - 1);


    // 2 VERTICAL BORDER LINES
    x = X;
    y0 = Y + 2;
    y1 = Y + height - 2;
    DisplayLineY(cmd, x, y0, y1);
    DisplayLineY(cmd, x + 1, y0, y1);


    x = X + width;
    DisplayLineY(cmd, x, y0, y1);
    DisplayLineY(cmd, x - 1, y0, y1);


    // 4 CORNERS
    x = X + 1;
    y = Y + 1;
    DisplaySetPixel(cmd, x, y);
    y = Y + height - 1;
    DisplaySetPixel(cmd, x, y);
    x = X + width - 1;
    y = Y + 1;
    DisplaySetPixel(cmd, x, y);
    x = X + width - 1;
    y = Y + height - 1;
    DisplaySetPixel(cmd, x, y);

    // CHAT ARROW
    x0 = X + first_pix;
    y0 = Y + height;
    x1 = X + first_pix + (float)mid_pix/2;
    y1 = Y + height + 15;
    draw_line(cmd, 2, x0, y0, x1, y1);

    x0 = X + first_pix + mid_pix;
    draw_line(cmd, 2, x0, y0, x1, y1);
    return;
}

void DisplayWelcomeLines(UBYTE cmd, UBYTE X, UBYTE Y, UBYTE height, UBYTE width){
    UBYTE x0, x1, y;

    // FIRST DASH
    x0 = X + (float)width/4 - 3;
    x1 = x0 + (float)width/4;
    y = Y + (float)height/6 + 3;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // FIRST DOT
    x0 = x1 + (float)width/8;
    x1 = x0 + 1;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // SECOND DOT
    x0 = x1 + (float)width/8;
    x1 = x0 + 1;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // THIRD DOT
    x0 = x1 + (float)width/8;
    x1 = x0 + 1;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // FOURTH DOT
    x0 = X + (float)width/4 - 3;
    x1 = x0 + 1;
    y = y + (float)height/3 - 3;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // SECOND DASH
    x0 = X + (float)width/3 - 2;
    x1 = x0 + (float)width/4;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // THIRD DASH
    x0 = x1 + (float)width/8;
    x1 = x0 + (float)width/4;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // FOURTH DASH
    x0 = X + (float)width/4 - 3;
    x1 = x0 + (float)width/4;
    y = Y + (float)height - (float)height/6 - 3;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // 5TH DOT
    x0 = x1 + (float)width/8 - 2;
    x1 = x0 + 1;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // 6TH DOT
    x0 = x1 + (float)width/8 - 2;
    x1 = x0 + 1;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // 7TH DOT
    x0 = x1 + (float)width/8 - 2;
    x1 = x0 + 1;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);

    // 8TH DOT
    x0 = x1 + (float)width/8 - 2;
    x1 = x0 + 1;
    DisplayLineX(cmd, x0, x1, y);
    DisplayLineX(cmd, x0, x1, y + 1);    
    return;
}



