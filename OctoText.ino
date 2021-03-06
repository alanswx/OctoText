#include <OctoWS2811.h>
#include "C64FontUpper.h"
#include "C64FontLower.h"
#include "font5x7.h"
#include <avr/pgmspace.h>
const int ledsPerStrip = 30;

DMAMEM int displayMemory[ledsPerStrip * 6];
int drawingMemory[ledsPerStrip * 6];
// Add the font name as declared in the header file.  Remove as many as possible to get conserve FLASH memory.



#define TOTALFONTS    3
const unsigned char *fontsPointer[TOTALFONTS] = {
   font5x7
  , C64FontUpper
  , C64FontLower 
  };
  /*
const unsigned char afontsPointer[] = {
  font5x7
  , C64FontUpper
  , C64FontLower
  //  font5x7
   // ,font8x16
   // ,sevensegment
   // ,fontlargenumber
   // ,space01
   // ,space02
  //  ,space03
  
};
*/
uint8_t foreColor, drawMode, fontWidth, fontHeight, fontType, fontStartChar, fontTotalChar, cursorX, cursorY;
uint16_t fontMapWidth;


const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

void setup() {
  leds.begin();
  leds.show();
  setFontType(2);
    Serial.begin(115200);
     while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

Serial.println("Starting..");
}
#define FONTHEADERSIZE    6
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF
#define CBACK  0x70A4B2
#define CFORE  0x6C5EB5
void loop() {



  
  int microsec = 2000000 / leds.numPixels();  // change them all in 2 seconds

microsec*=20;
 // microsec = 1000000;
  int x = 0;
  int i;
  for (i=30;i>-200;i--)
  {
      int last = i;
      Serial.print("Blast:");
     Serial.println(last);
     last+=drawString(last,0,"Blake rocks! ",ORANGE);
      Serial.print("Alast:");
     Serial.println(last);    
     last+=drawChar(last,0, 'A', RED,0);
     last+=drawChar(last,0, 'l', GREEN,0);
     last+=drawChar(last,0, 'a', BLUE,0);
     last+=drawChar(last,0, 'n', YELLOW,0);
    // Serial.print("last:");
    // Serial.println(last);
     leds.show();
   delayMicroseconds(microsec);
  }
 /* delayMicroseconds(microsec);
  drawChar(0,0, 'l', RED,0);
  leds.show();
  delayMicroseconds(microsec);
  drawChar(0,0, 'a', RED,0);
  leds.show();
  delayMicroseconds(microsec);
  drawChar(0,0, 'n', RED,0);
  leds.show();
  delayMicroseconds(microsec);
  drawChar(0,0, 's', RED,0);
  leds.show();
  delayMicroseconds(microsec);
  delayMicroseconds(microsec);
  delayMicroseconds(microsec);
  delayMicroseconds(microsec);
  delayMicroseconds(microsec);
 */ 
  // uncomment for voltage controlled speed
  // millisec = analogRead(A9) / 40;
/*
  colorWipe(RED, microsec);
  colorWipe(GREEN, microsec);
  colorWipe(BLUE, microsec);
  colorWipe(YELLOW, microsec);
  colorWipe(PINK, microsec);
  colorWipe(ORANGE, microsec);
  colorWipe(WHITE, microsec);
  */
}

void colorWipe(int color, int wait)
{
  for (int i = 0; i < leds.numPixels(); i++) {
    leds.setPixel(i, color);
    leds.show();
    delayMicroseconds(wait);
  }
}


int drawString(uint8_t x, uint8_t y, String text, int color)
{
  // start at x, and call drawChar for each.. increment x, and return it..
  int length = text.length();
  int last = x;
  int totalWidth = 0;
  for (int i =0; i < length; i++)
  {
     int width=drawChar(last,0, text.charAt(i), color,0);
     last+=width;
     totalWidth+=width;
  }
 return totalWidth; 
}

/** \brief Draw character with color and mode.
  Draw character c using color and draw mode at x,y.
*/
int  drawChar(uint8_t x, uint8_t y, uint8_t c, int color, uint8_t mode) {
  // TODO - New routine to take font of any height, at the moment limited to font height in multiple of 8 pixels

  uint8_t rowsToDraw, row, tempC;
  uint8_t i, j, temp;
  uint16_t charPerBitmapRow, charColPositionOnBitmap, charRowPositionOnBitmap, charBitmapStartPosition;

  if ((c < fontStartChar) || (c > (fontStartChar + fontTotalChar - 1))) // no bitmap for the required c
    return fontWidth;

  tempC = c - fontStartChar;

  // each row (in datasheet is call page) is 8 bits high, 16 bit high character will have 2 rows to be drawn
  rowsToDraw = fontHeight / 8; // 8 is LCD's page size, see SSD1306 datasheet
  if (rowsToDraw <= 1) rowsToDraw = 1;

  // the following draw function can draw anywhere on the screen, but SLOW pixel by pixel draw
  if (rowsToDraw == 1) {
    for  (i = 0; i < fontWidth + 1; i++) {
      if (i == fontWidth) // this is done in a weird way because for 5x7 font, there is no margin, this code add a margin after col 5
        temp = 0;
      else
        temp = pgm_read_byte(fontsPointer[fontType] + FONTHEADERSIZE + (tempC * fontWidth) + i);

      for (j = 0; j < 8; j++) { // 8 is the LCD's page height (see datasheet for explanation)
        if (temp & 0x1) {
          pixel(x + i, y + j, color, mode);

        }
        else {
          pixel(x + i, y + j, !color, mode);

        }

        temp >>= 1;
      }
    }
    return fontWidth;
  }

  // font height over 8 bit
  // take character "0" ASCII 48 as example
  charPerBitmapRow = fontMapWidth / fontWidth; // 256/8 =32 char per row
  charColPositionOnBitmap = tempC % charPerBitmapRow; // =16
  charRowPositionOnBitmap = int(tempC / charPerBitmapRow); // =1
  charBitmapStartPosition = (charRowPositionOnBitmap * fontMapWidth * (fontHeight / 8)) + (charColPositionOnBitmap * fontWidth) ;

  // each row on LCD is 8 bit height (see datasheet for explanation)
  for (row = 0; row < rowsToDraw; row++) {
    for (i = 0; i < fontWidth; i++) {
      temp = pgm_read_byte(fontsPointer[fontType] + FONTHEADERSIZE + (charBitmapStartPosition + i + (row * fontMapWidth)));
      for (j = 0; j < 8; j++) { // 8 is the LCD's page height (see datasheet for explanation)
        if (temp & 0x1) {
          pixel(x + i, y + j + (row * 8), color, mode);

        }
        else {
          pixel(x + i, y + j + (row * 8), !color, mode);

        }
        temp >>= 1;
      }
    }
  }

  /*
    fast direct memory draw but has a limitation to draw in ROWS
    // only 1 row to draw for font with 8 bit height
    if (rowsToDraw==1) {
    for (i=0; i<fontWidth; i++ ) {
      screenmemory[temp + (line*LCDWIDTH) ] = pgm_read_byte(fontsPointer[fontType]+FONTHEADERSIZE+(c*fontWidth)+i);
      temp++;
    }
    return;
    }
    // font height over 8 bit
    // take character "0" ASCII 48 as example
    charPerBitmapRow=fontMapWidth/fontWidth;  // 256/8 =32 char per row
    charColPositionOnBitmap=c % charPerBitmapRow;  // =16
    charRowPositionOnBitmap=int(c/charPerBitmapRow); // =1
    charBitmapStartPosition=(fontMapWidth * (fontHeight/8)) + (charColPositionOnBitmap * fontWidth);

    temp=x;
    for (row=0; row<rowsToDraw; row++) {
    for (i=0; i<fontWidth; i++ ) {
      screenmemory[temp + (( (line*(fontHeight/8)) +row)*LCDWIDTH) ] = pgm_read_byte(fontsPointer[fontType]+FONTHEADERSIZE+(charBitmapStartPosition+i+(row*fontMapWidth)));
      temp++;
    }
    temp=x;
    }
  */
 return fontWidth;
}

void pixel(uint8_t x, uint8_t y,  int color, uint8_t mode)
{
  uint8_t i = (y * ledsPerStrip + x);
  if (x>= ledsPerStrip)
  {
     /*Serial.print("overflow:");
     Serial.print(x);
     Serial.print(",");
     Serial.println(y);
     */
     return;
  }

 leds.setPixel(i, color);
 // if (color) leds.setPixel(i, CFORE);
 // else leds.setPixel(i,CBACK);
 /*
Serial.print("setPixel:");
Serial.print(i);
Serial.print("color: ");
Serial.println(color, HEX);
*/
}

uint8_t setFontType(uint8_t type) {
  if ((type >= TOTALFONTS) || (type < 0))
    return false;

  fontType = type;
  fontWidth = pgm_read_byte(fontsPointer[fontType] + 0);
  fontHeight = pgm_read_byte(fontsPointer[fontType] + 1);
  fontStartChar = pgm_read_byte(fontsPointer[fontType] + 2);
  fontTotalChar = pgm_read_byte(fontsPointer[fontType] + 3);
  fontMapWidth = (pgm_read_byte(fontsPointer[fontType] + 4) * 100) + pgm_read_byte(fontsPointer[fontType] + 5); // two bytes values into integer 16
  return true;
}

