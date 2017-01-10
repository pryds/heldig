boolean titresBoard[frameWidth][frameHeight];
boolean titresCurrentPiece[4][4];
int titresCurrentPieceX;
int titresCurrentPieceY;

unsigned int points = 0;
unsigned int level = 1;
int clearedLines = 0;
const int pointsPerLine[4] = {40, 100, 300, 1200};

void titresMain() {
  randomSeed(analogRead(0));
  random(1000);
  
  boolean piece[7][4][4] = { //[numPieces][width][height]
    {
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0}
    },
    {
      {0, 1, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {0, 1, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 1, 0},
      {0, 1, 1, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 0, 0},
      {0, 1, 1, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 1, 0},
      {0, 1, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 0}
    }
  };
  
  unsigned long currentTime;
  
  unsigned long lastTimePieceFell;
  int pieceFallInterval = 1000; //initial, in milliseconds
  
  boolean turnedPiece[4][4];
  
  boolean aButtonWasPressed = false;
  int leftButtonState = 0;
  int upButtonState = 0;
  int rightButtonState = 0;

  while(true) {
    //New piece; initialise currentPiece
    copyPiece(piece[(int)random(7)], titresCurrentPiece);
    titresCurrentPieceX = frameWidth/2-2;
    titresCurrentPieceY = -2;
    
    /*
    //check if we have a collision already
    if (hasCollision(titresCurrentPiece, titresCurrentPieceX, titresCurrentPieceY)) {
      // game over!
      emptyFrame(titresBoard);
      points = 0;
      clearedLines = 0;
    }
    */
    
    boolean pieceHasLanded = false;
    while (!pieceHasLanded) {
      currentTime = millis();
      
      // has a button been pressed
      leftButtonState = digitalRead(leftButtonPin);
      upButtonState = digitalRead(upButtonPin);
      rightButtonState = digitalRead(rightButtonPin);
      if (!aButtonWasPressed) { // if a button was not pressed at last check
        if (leftButtonState == HIGH) {
          if (!hasCollision(titresCurrentPiece, titresCurrentPieceX - 1, titresCurrentPieceY)) {
            titresCurrentPieceX--;
          }
          aButtonWasPressed = true;
        } else if (rightButtonState == HIGH) {
          if (!hasCollision(titresCurrentPiece, titresCurrentPieceX + 1, titresCurrentPieceY)) {
            titresCurrentPieceX++;
          }
          aButtonWasPressed = true;
        } else if (upButtonState == HIGH) {
          turnPieceToTheRight(titresCurrentPiece, turnedPiece);
          if (!hasCollision(turnedPiece, titresCurrentPieceX, titresCurrentPieceY)) {
            copyPiece(turnedPiece, titresCurrentPiece);
          }
          aButtonWasPressed = true;
        }
      } else if (leftButtonState == LOW && rightButtonState == LOW && upButtonState == LOW) {
        aButtonWasPressed = false;
      }
      
      // check if it's time to move piece down
      if (currentTime - lastTimePieceFell > pieceFallInterval / level) {
        // will the next position cause a collision?
        if (hasCollision(titresCurrentPiece, titresCurrentPieceX, titresCurrentPieceY + 1)) {
          //collision, so don't move piece
          mergePieceWithBoard();
          removeFullLines();
          pieceHasLanded = true;
        } else {
          titresCurrentPieceY++; // move piece 1 down
          lastTimePieceFell = currentTime;
        }
      }
      drawBoardOnCurFrame();
      drawFrameOnDisplay();
    }
  }
}

void mergePieceWithBoard() {
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      if (titresCurrentPiece[x][y]) { // if the piece occupies this local coor
        //put content at board coor
        titresBoard[titresCurrentPieceX + x][titresCurrentPieceY + y] = true;
      } // otherwise do nothing (might be something on the board there already)
    }
  }
}

void removeFullLines() {
  int consecutiveFullLines = 0;
  for (int line = frameHeight - 1; line >= 0; line--) {
    boolean lineFull = true;
    for (int column = 0; lineFull && column < frameWidth; column++) {
      if (!titresBoard[column][line]) {
        lineFull = false;
      }
    }
    if (lineFull) {
      clearedLines++;
      consecutiveFullLines++;
      // to-do: remove line!, e.g. removeLine(linenumber)
    } else {
      if (consecutiveFullLines > 0) {
        level = (clearedLines / 10) + 1;
        int additionalPoints = pointsPerLine[consecutiveFullLines-1] * (level);
        points = points + additionalPoints;
        consecutiveFullLines = 0;
      }
    }
  }
}

boolean hasCollision(boolean piece[4][4], int pieceNewX, int pieceNewY) {
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      //only a collision risk if the piece occupies this local coordinate
      if (piece[x][y]) {
        // check if this coor of piece is outside of board on X axis
        if (pieceNewX + x < 0 || pieceNewX + x >= frameWidth) {
          return true;
        }
        // check if this coor of piece is below board
        if (pieceNewY + y >= frameHeight) { //don't check top
          return true; //collision with bottom!
        }
        // only check for collision if this coor is inside board (not above)
        if (pieceNewY >= 0) {
          // check if this coor of piece collides with previous pieces on board
          if (titresBoard[pieceNewX + x][pieceNewY + y]) {
            return true; // collision with something on board!
          }
        }
      }
    }
  }
  return false;
}

void drawBoardOnCurFrame() {
  for (int x = 0; x < frameWidth; x++) {
    for (int y = 0; y < frameHeight; y++) {
      if (titresBoard[x][y]) { //something is on the board at this coordinate
        curFrame[x][y] = true;
      } else if ((x >= titresCurrentPieceX && x < titresCurrentPieceX+4)
                   &&
                 (y >= titresCurrentPieceY && y < titresCurrentPieceY+4)) {
        curFrame[x][y] = titresCurrentPiece[x-titresCurrentPieceX][y-titresCurrentPieceY];
      } else {
        curFrame[x][y] = false;
      }
      //curFrame[x][y] = titresBoard[x][y] && titresCurrentPiece[][];
    }
  }
}

void copyPiece(boolean fromPiece[4][4], boolean toPiece[4][4]) {
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      toPiece[x][y] = fromPiece[x][y];
    }
  }
}

void turnPieceToTheRight(boolean origPiece[4][4], boolean turnedPiece[4][4]) {
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      turnedPiece[3-y][x] = origPiece[x][y];
    }
  }
}

