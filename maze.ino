#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6

// PROGRAMMING: USE OLD BOOTLOADER

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

// matrix
const int height = 8;
const int width = 8;
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(width, height, PIN, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

int pinX = A1;
int pinY = A2;
int pinZ = A3;
int rawX = 0;
int rawY = 0;
int cursorX = 0;
int cursorY = 0;
uint16_t cursorColor = matrix.Color(180, 180, 200);
int cursorOnPlayer = false;
int cursorOnWall = false;

int playerX = 0;
int playerY = 0;
uint16_t playerColor = matrix.Color(0, 0, 190);
uint16_t playerColorBright = matrix.Color(0, 0, 255);

int board[width][height] = {
  {0,0,0,0,0,1,0,0},
  {0,0,1,0,0,1,0,1},
  {1,0,1,0,0,1,0,1},
  {0,0,1,0,0,1,0,0},
  {1,0,0,1,1,1,1,1},
  {0,1,0,0,0,1,0,0},
  {0,1,1,1,0,1,0,1},
  {0,0,1,0,0,0,0,2},
};
uint16_t wallColor = matrix.Color(120, 0, 0);
uint16_t wallColorBright = matrix.Color(255, 0, 0);
uint16_t goalColor = matrix.Color(255, 255, 0);
int bright = 50;

int textX = 0;
uint16_t textColor = matrix.Color(0, 0, 200);

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(bright);
  matrix.setTextColor(textColor);
  Serial.begin(9600);

  // duplicate of input parsing with no gating, so that initial cursor is set correctly
  rawX = analogRead(pinX);
  cursorX = 7 - map(rawX, 0, 860, 0, 1024)/128;
  rawY = analogRead(pinY);
  cursorY = 7 - map(rawY, 0, 860, 0, 1024)/128;
}

void loop() {
  matrix.fillScreen(0);

  // check for win
  if (board[playerX][playerY] == 2) {
    win();
  } else { 
    game();
  }

  matrix.show();
  delay(10);
}

void game() {
  // handle and denoise input
  
  if (abs(analogRead(pinX) - rawX) > 30) {
    cursorX = 7 - map(rawX, 0, 860, 0, 1024) / 128;
    rawX = analogRead(pinX);
  }
  if (abs(analogRead(pinY) - rawY) > 30) {
    cursorY = 7 - map(rawY, 0, 860, 0, 1024) / 128;
    rawY = analogRead(pinY);
  }

  // draw board
  cursorOnWall = false;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      if (board[i][j] == 1) {
        if (i == cursorX && j == cursorY) {
          matrix.drawPixel(i, j, wallColorBright);
          cursorOnWall = 1;
        } else {
          matrix.drawPixel(i, j, wallColor); 
        }
      } else if (board[i][j] == 2) {
        matrix.drawPixel(i, j, goalColor);
      }
    }
  }

  // handle player movement
  if (cursorOnPlayer && (cursorX != playerX || cursorY != playerY)) {
    int dist = abs(cursorX - playerX) + abs(cursorY - playerY);
    if (dist == 1 && board[cursorX][cursorY] != 1) {
      playerX = cursorX;
      playerY = cursorY;
    } else {
      cursorOnPlayer = false;
    }
  }
  if (cursorX == playerX && cursorY == playerY) {
    cursorOnPlayer = true;
  }

  // draw player & cursor
  if (cursorOnPlayer) {
    matrix.drawPixel(playerX, playerY, playerColorBright); 
  } else {
    if (!cursorOnWall) {
      matrix.drawPixel(cursorX, cursorY, cursorColor); 
    }
    matrix.drawPixel(playerX, playerY, playerColor); 
  }
}

void win() {
  matrix.setCursor(textX, 0);
  matrix.print(F("Victory! 412"));
  if(--textX < -66) {
    textX = width;
  }
  delay(100);
}
