/**
  * Code used to animate a robot costume for my nephew. Written in my sister's great 
  * room with family and kid's friends visiting. Wholely unoptimized, glued together
  * from other code, but works and the result was very impressive on Halloween.
  * Video demo: http://www.youtube.com/watch?v=A2VtEYe8-CI
  */
#include <Keypad.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <Adafruit_NeoPixel.h>
#include <Bounce.h>

// Animation tick
const int animStep = 15;
// Keep track of ticks for LED matrices animation.
byte subAnimStep = 0;
// Keep track of output from millis() on last tick.
unsigned long lastAnimStep = 0;
// Last color used on the LED matrices.
byte matrixColor = 0; 

// Flag if we are scrolling text on the LED matrices.
boolean drawingText = false;
// The text that we are scrolling.
char* scrollingText;
// The length of the text we are scrolling.
int scrollLength;
// Where we are in the scroll.
int scrollOffset;
// The color of the text.
byte scrollColor;
// The number of ticks since the last scroll.
int scrollStep = 0;

// The timeout in milliseconds for the keypad.
const int keypadTimeout = 1000;
// The time from millis() when the last key was pressed on the keypad.
unsigned long keypadTime = 0;
// Are we processing the keypad? IOW, are we waiting on another press or the timeout
boolean keypadUse = false;
// Had a piezo speaker wired in, but not used.
int beepLen = 200;
// The default display on the matrices when we start processing the keypad.
char code[3] = {'-', '-', '-'};
// How many keypresses? (zero indexed)
byte codePtr = -1;
boolean fullCode;
boolean goodCode;

// Initialize the membrane keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {12, 11, 10, 9}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); 

// Initialize our 3 bi-color LED matrices.
Adafruit_BicolorMatrix matrix0 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix1 = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix2 = Adafruit_BicolorMatrix();

// Using the faces from Adafruit's demo code.
static const uint8_t PROGMEM
  smile_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100 },
  neutral_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 },
  frown_bmp[] = 
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10011001,
    B10100101,
    B01000010,
    B00111100 };
  
// Keep track of our animation frames.  
const int bmpSize = 3;
int lastBmp = -1;

// Neopixel rings are on pin 13.
#define NEO_PIN            13

// Using rainbow effect from demo code. (My family really liked it and it saved me time)
#define TOP_LED_FIRST  0 // Change these if the first pixel is not
#define TOP_LED_SECOND 0 // at the top of the first and/or second ring.

#define EFFECT         RAINBOW // Choose a visual effect from the names below

#define RAINBOW        0
#define ECTO           1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(32, NEO_PIN, NEO_GRB + NEO_KHZ800);

const int8_t PROGMEM
  yCoord[] = { // Vertical coordinate of each pixel.  First pixel is at top.
    127,117,90,49,0,-49,-90,-117,-127,-117,-90,-49,0,49,90,117 },
  sine[] = { // Brightness table for ecto effect
    0, 28, 96, 164, 192, 164, 96, 28, 0, 28, 96, 164, 192, 164, 96, 28 };

// Eyelid vertical coordinates.  Eyes shut slightly below center.
#define upperLidTop     130
#define upperLidBottom  -45
#define lowerLidTop     -40
#define lowerLidBottom -130

// Gamma correction improves appearance of midrange colors
const uint8_t PROGMEM gamma8[] = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
      1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,
      3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,
      7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12,
     13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20,
     20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29,
     30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42,
     42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
     58, 59, 60, 61, 62, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75,
     76, 77, 78, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
     97, 99,100,102,103,105,106,108,109,111,112,114,115,117,119,120,
    122,124,125,127,129,130,132,134,136,137,139,141,143,145,146,148,
    150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,
    182,184,186,188,191,193,195,197,199,202,204,206,209,211,213,215,
    218,220,223,225,227,230,232,235,237,240,242,245,247,250,252,255
};

uint32_t
  iColor[16][3];      // Background colors for eyes
int16_t
  hue          =   0; // Initial hue around perimeter (0-1535)
uint8_t
  iBrightness[16],    // Brightness map -- eye colors get scaled by these
  brightness   = 220, // Global brightness (0-255)
  blinkFrames  =   5, // Speed of current blink
  blinkCounter =  30, // Countdown to end of next blink
  eyePos       = 192, // Current 'resting' eye (pupil) position
  newEyePos    = 192, // Next eye position when in motion
  gazeCounter  =  75, // Countdown to next eye movement
  gazeFrames   =  50; // Duration of eye movement (smaller = faster)
int8_t
  eyeMotion    =   0; // Distance from prior to new position

// LED on momentary switch on pin 4 and the switch on pin 1.
int ledPin    = 4;
int switchPin = 1;
boolean ledOn = true;
unsigned long ledStart;
Bounce bouncer(switchPin, 20);

void setup() {
  // refer to the matrices documentation. I left one unjumpered (0x70), A0 jumpered (0x71), and A1 jumpered (0x72)
  matrix0.begin(0x70);  // pass in the address
  matrix1.begin(0x71);  // pass in the address
  matrix2.begin(0x72);  // pass in the address
  
  // Matrices were mounted on a 90� rotation.
  matrix0.setRotation(1);
  matrix1.setRotation(1);
  matrix2.setRotation(1);

  // From NeoPixel ring demo code (seed the PRNG with noise from analog pin 0)
  randomSeed(analogRead(A0));
  pixels.begin();
  
  // Turn on the switch's led and set pin modes.
  pinMode(ledPin, OUTPUT);
  pinMode(switchPin, INPUT);
  digitalWrite(ledPin, HIGH);
}

void loop() {
  // Keep track of the time.  
  unsigned long currentTime = millis();
  
  // We want to be responsive to inputs. Handle them on every time loop() is called.

  // Handle the momentary switch.
  handleButton();
  
  // Read the keypad.
  char customKey = keypad.getKey();
  
  // Handle a key if it was pushed.
  if (customKey){
    keypadUse = true;
    drawingText = false;
    keypadTime = currentTime;
    
    handleKey(customKey);
  }
  
  // Handle displaying scroll text after a code was entered or timing out the 
  if(currentTime - keypadTime > keypadTimeout && keypadUse) {
    keypadUse = false;
    if(fullCode) {
      if(goodCode) {
        scrollText("You win!!!!", 90, LED_GREEN);
      } else {
        scrollText("That is not my code.", 140, LED_YELLOW);
      }
      fullCode = false;
    }
  }
  

  // Handle the false power button. 
  if (!ledOn) {
    if(currentTime - ledStart > 1000) {
      ledOn = true;
      digitalWrite(ledPin, HIGH);
      scrollText("You can't turn me off!!!", 176, LED_RED);
    }
  }

  // Animations aren't handled every time. This loop creates a 'tick'. The NeoPixels are updated every tick.
  // The scrolling text and matrix animations are only handled after a certain number of ticks. 
  if(currentTime - lastAnimStep > animStep) {
    lastAnimStep = currentTime;
    
    // Are we scrolling text?
    if(drawingText) {
      handleScroll();
    }
    
    // Code for NeoPixel ring eyes.
    uint8_t i, r, g, b, a, c, inner, outer, ep;
    int     y1, y2, y3, y4, h;
    int8_t  y;
  
    for(h=hue, i=0; i<16; i++, h += 96) {
      a = h;
      switch((h >> 8) % 6) {
        case 0: iColor[i][0] = 255; iColor[i][1] =   a; iColor[i][2] =   0; break;
        case 1: iColor[i][0] =  ~a; iColor[i][1] = 255; iColor[i][2] =   0; break;
        case 2: iColor[i][0] =   0; iColor[i][1] = 255; iColor[i][2] =   a; break;
        case 3: iColor[i][0] =   0; iColor[i][1] =  ~a; iColor[i][2] = 255; break;
        case 4: iColor[i][0] =   a; iColor[i][1] =   0; iColor[i][2] = 255; break;
        case 5: iColor[i][0] = 255; iColor[i][1] =   0; iColor[i][2] =  ~a; break;
      }
    }
    hue += 7;
    if(hue >= 1536) hue -= 1536;
  
    if(blinkCounter <= blinkFrames * 2) { // In mid-blink?
      if(blinkCounter > blinkFrames) {    // Eye closing
        outer = blinkFrames * 2 - blinkCounter;
        inner = outer + 1;
      } else {                            // Eye opening
        inner = blinkCounter;
        outer = inner - 1;
      }
      y1 = upperLidTop    - (upperLidTop - upperLidBottom) * outer / blinkFrames;
      y2 = upperLidTop    - (upperLidTop - upperLidBottom) * inner / blinkFrames;
      y3 = lowerLidBottom + (lowerLidTop - lowerLidBottom) * inner / blinkFrames;
      y4 = lowerLidBottom + (lowerLidTop - lowerLidBottom) * outer / blinkFrames;
      for(i=0; i<16; i++) {
        y = pgm_read_byte(&yCoord[i]);
        if(y > y1) {        // Above top lid
          iBrightness[i] = 0;
        } else if(y > y2) { // Blur edge of top lid in motion
          iBrightness[i] = brightness * (y1 - y) / (y1 - y2);
        } else if(y > y3) { // In eye
          iBrightness[i] = brightness;
        } else if(y > y4) { // Blur edge of bottom lid in motion
          iBrightness[i] = brightness * (y - y4) / (y3 - y4);
        } else {            // Below bottom lid
          iBrightness[i] = 0;
        }
      }
    } else { // Not in blink -- set all 'on'
      memset(iBrightness, brightness, sizeof(iBrightness));
    }

    if(--blinkCounter == 0) { // Init next blink?
      blinkFrames  = random(4, 8);
      blinkCounter = blinkFrames * 2 + random(5, 180);
    }

    // Calculate current eye movement, possibly init next one
    if(--gazeCounter <= gazeFrames) { // Is pupil in motion?
      ep = newEyePos - eyeMotion * gazeCounter / gazeFrames; // Current pos.
      if(gazeCounter == 0) {                   // Last frame?
          eyePos      = newEyePos;               // Current position = new pos
          newEyePos   = random(16) * 16;         // New pos. (always pixel center)
          eyeMotion   = newEyePos - eyePos;      // Distance to move
          gazeFrames  = random(10, 20);          // Duration of movement
          gazeCounter = random(gazeFrames, 130); // Count to END of next movement
        }
      } else ep = eyePos; // Not moving -- fixed gaze

      // Draw pupil -- 2 pixels wide, but sup-pixel positioning may span 3.
      a = ep >> 4;         // First candidate
      b = (a + 1)  & 0x0F; // 1 pixel CCW of a
      c = (a + 2)  & 0x0F; // 2 pixels CCW of a
      i = ep & 0x0F;       // Fraction of 'c' covered (0-15)
      iBrightness[a] = (iBrightness[a] *       i ) >> 4;
      iBrightness[b] = 0;
      iBrightness[c] = (iBrightness[c] * (16 - i)) >> 4;

      // Merge iColor with iBrightness, issue to NeoPixels
      for(i=0; i<16; i++) {
        a = iBrightness[i] + 1;
        // First eye
      r = iColor[i][0];            // Initial background RGB color
      g = iColor[i][1];
      b = iColor[i][2];
      if(a) {
        r = (r * a) >> 8;          // Scale by brightness map
        g = (g * a) >> 8;
        b = (b * a) >> 8;
      }
      pixels.setPixelColor(((i + TOP_LED_FIRST) & 15),
      pgm_read_byte(&gamma8[r]), // Gamma correct and set pixel
      pgm_read_byte(&gamma8[g]),
      pgm_read_byte(&gamma8[b]));

      // Second eye uses the same colors, but reflected horizontally.
      // The same brightness map is used, but not reflected (same left/right)
      r = iColor[15 - i][0];
      g = iColor[15 - i][1];
      b = iColor[15 - i][2];
      if(a) {
        r = (r * a) >> 8;
        g = (g * a) >> 8;
        b = (b * a) >> 8;
      }
      pixels.setPixelColor(16 + ((i + TOP_LED_SECOND) & 15),
      pgm_read_byte(&gamma8[r]),
      pgm_read_byte(&gamma8[g]),
      pgm_read_byte(&gamma8[b]));
    }
    pixels.show();
  
    // Every 33 ticks we update the frame of animation being shown on the LED matrices.
    // Unless we are working with the keypad, scrolling text, or doing the false power button.
    subAnimStep++;
    if(subAnimStep == 33 && !keypadUse && ! drawingText && ledOn) {
      subAnimStep = 0;
      lastBmp++;

      // My sister asked if it could scroll my nephew's name after the faces display so I wedged it in
      // here as an after thought. TODO: update to AnnaBot in a few years when my niece inherits the
      // costume.
      if(lastBmp == -1) {
        lastBmp = 0;
        scrollText("My name is AnthonyBot", 150, LED_GREEN);
      }
      // Intended to add more animation frames. TODO: add more in the future.
      if(lastBmp == bmpSize) {
        lastBmp = -2;
      }
    
      switch(lastBmp) {
        case 0:
          draw(smile_bmp);
          break;
        case 1:
          draw(neutral_bmp);
          break;
        case 2:
          draw(frown_bmp);
          break;
      }
    }
  }  
}

void clearDisplays() {
  matrix0.clear();
  matrix1.clear();
  matrix2.clear();
}

void draw(const uint8_t *bitmap) {
  byte color = 3 - (lastBmp % 3);

  clearDisplays();
 
  matrix0.drawBitmap(0, 0, bitmap, 8, 8, color);
  matrix1.drawBitmap(0, 0, bitmap, 8, 8, color);
  matrix2.drawBitmap(0, 0, bitmap, 8, 8, color);
  
  matrix0.writeDisplay();
  matrix1.writeDisplay();
  matrix2.writeDisplay();
}

// Handle the false power button.
void handleButton() {
  
  // Ask the bouncer to update and if we're on the falling edge,
  // flip the state of the LED. Falling edge returning true means
  // the prior time update was called the button read HIGH
  // (pushed in our case), but now it reads LOW.
  bouncer.update(); 
  if(bouncer.fallingEdge() && ledOn) {
    ledOn = false;
    digitalWrite(ledPin, LOW);
    
    ledStart = millis();
    drawingText = false;
    
    clearDisplays();
    matrix0.setTextColor(LED_RED);
    matrix0.setTextSize(1);
    matrix0.setCursor(3,1);
    matrix1.setTextColor(LED_RED);
    matrix1.setTextSize(1);
    matrix1.setCursor(3,1);
    matrix2.setTextColor(LED_RED);
    matrix2.setTextSize(1);
    matrix2.setCursor(3,1);
    
    matrix0.print('O');
    matrix1.print('f');
    matrix2.print('f');
    
    matrix0.writeDisplay();
    matrix1.writeDisplay();
    matrix2.writeDisplay();
  }
}

void handleKey(char key) {
  clearDisplays();
  
  codePtr++;
  
  if(codePtr < 3) {
    fullCode = false;
    code[codePtr] = key;
    
    matrix0.setTextColor(LED_GREEN);
    matrix0.setTextSize(1);
    matrix0.setCursor(3,1);
    matrix1.setTextColor(LED_GREEN);
    matrix1.setTextSize(1);
    matrix1.setCursor(3,1);
    matrix2.setTextColor(LED_GREEN);
    matrix2.setTextSize(1);
    matrix2.setCursor(3,1);
    
    matrix0.print(code[0]);
    matrix1.print(code[1]);
    matrix2.print(code[2]);
    
    matrix0.writeDisplay();
    matrix1.writeDisplay();
    matrix2.writeDisplay();
  }
  
  if(codePtr == 2) {
    fullCode = true;
    if(code[0] == '1' && code[1] == '5' && code[2]  == '9') {
      goodCode = true;
    } else {
      goodCode = false;
    }
    
    // First dash is never displayed. Optimizing away unnecessary code (TODO: untested).
    //code[0] = '-';
    code[1] = '-';
    code[2] = '-';
    codePtr = -1;
  }
}

// We start the scroll offset at 0 and decrement. This will cause the text to scroll right to left.
void scrollText(char* message, int messageSize, byte color) {
  drawingText = true;
  scrollingText = message;
  scrollLength = messageSize;
  scrollOffset = 0;
  scrollColor = color;
  scrollStep = 0;
}

void handleScroll() {
  scrollStep++;
  // Scroll the text every 4 ticks.
  if(scrollStep == 4) {
    scrollStep = 0;
    clearDisplays();
  
    // Should probably be moved to setup as it never changes. (TODO: optimization?)
    matrix0.setTextWrap(false);
    matrix1.setTextWrap(false);
    matrix2.setTextWrap(false);
    
    // Since we are scrolling on 3 8 LED wide matrices, offset the text 8, 16, and 24 pixels.
    // This will make it appear to be scrolling across all 3.
    matrix0.setCursor(scrollOffset+24, 0);
    matrix1.setCursor(scrollOffset+16, 0);
    matrix2.setCursor(scrollOffset+8, 0);
   
    matrix0.setTextColor(scrollColor);
    matrix1.setTextColor(scrollColor);
    matrix2.setTextColor(scrollColor);
    
    matrix0.print(scrollingText);
    matrix1.print(scrollingText);
    matrix2.print(scrollingText);
    
    matrix0.writeDisplay();
    matrix1.writeDisplay();
    matrix2.writeDisplay();
    
    scrollOffset--;
    
    // Once the text is fully off the displays, we want to go right back into the animation.
    if(scrollOffset == -1 * scrollLength) {
      drawingText = false;
      subAnimStep = 32;
    }
  }
}
