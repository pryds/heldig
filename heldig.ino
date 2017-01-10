#include <LedControl.h>

LedControl lc1 = LedControl(6, 5, 4, 2); //(din, clk, cs, numBoards)

const int leftButtonPin = 11;
const int upButtonPin = 12;
const int rightButtonPin = 13;

int displayArrangement [2][1] = {
  {1}, //display Id/address
  {0}  //display Id/address
};

int getIdOfDisplayAt(int globalX, int globalY) {
  return displayArrangement[globalY/8][globalX/8];
}

const int frameWidth = 8;
const int frameHeight = 16;
boolean curFrame[8][16];
boolean lastFrame[8][16];

void setup() {
  Serial.begin(9600);
  pinMode(leftButtonPin, OUTPUT);
  pinMode(upButtonPin, OUTPUT);
  pinMode(rightButtonPin, OUTPUT);
  
  lc1.clearDisplay(0); //clear all leds
  lc1.clearDisplay(1); //clear all leds
  lc1.shutdown(0, false); //wake up from power-saving
  lc1.shutdown(1, false); //wake up from power-saving
  lc1.setIntensity(0, 8); //medium brightness
  lc1.setIntensity(1, 8); //medium brightness
  
  emptyFrame(curFrame);
  emptyFrame(lastFrame);
}

void loop() {
  clonetrisMain();
  /*
  for (int x = 0; x < frameWidth; x++) {
    for (int y = 0; y < frameHeight; y++) {
      curFrame[x][y] = true;
      curFrame[7-x][15-y] = true;
      curFrame[y*frameWidth/frameHeight][x*frameHeight/frameWidth] = true;
      curFrame[7-(y*frameWidth/frameHeight)][15-(x*frameHeight/frameWidth)] = true;
      //curFrame[7-y][15-x] = true;
      drawFrameOnDisplay();
      emptyFrame(curFrame);
      delay(20);
    }
  }*/
}

void drawFrameOnDisplay() {
  //lc1.shutdown(0, true);
  for (int x = 0; x < frameWidth; x++) {
    for (int y = 0; y < frameHeight; y++) {
      if (curFrame[x][y] != lastFrame[x][y]) {
        lc1.setLed(getIdOfDisplayAt(x, y), y%8, x%8, curFrame[x][y]);
      }
    }
  }
  //lc1.shutdown(0, false);
  copyFrameToFrame(curFrame, lastFrame);
}

void copyFrameToFrame(boolean fromFrame[8][16], boolean toFrame[8][16]) {
  for (int x = 0; x < frameWidth; x++) {
    for (int y = 0; y < frameHeight; y++) {
      toFrame[x][y] = fromFrame[x][y];
    }
  }
}

void emptyFrame(boolean frame[8][16]) {
  for (int w = 0; w < frameWidth; w++) {
    for (int h = 0; h < frameHeight; h++) {
      frame[w][h] = false;
    }
  }
}

