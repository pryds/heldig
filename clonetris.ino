//Game variables
boolean clonetrisBoard[FRAME_WIDTH][FRAME_HEIGHT];
boolean clonetrisCurrentPiece[4][4];
int clonetrisCurrentPieceX;
int clonetrisCurrentPieceY;

unsigned int points;
unsigned int level;
int clearedLines;

//Game constants
const int POINTS_PER_LINE[4] = {40, 100, 300, 1200}; //per 1 line, 2 lines, 3 lines, 4 lines
const int INITIAL_PIECE_X = (FRAME_WIDTH / 2) - 2;
const int INITIAL_PIECE_Y = -1;

void clonetrisMain() {
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
  
  //Variables for timing falling pieces
  unsigned long currentTime;
  unsigned long lastTimePieceFell;
  const int PIECE_FALL_INTERVAL = 1000; //initial, in milliseconds
  
  boolean turnedPiece[4][4];
  
  //Variables for button handling
  boolean aButtonWasPressed = false;
  int leftButtonState = 0;
  int upButtonState = 0;
  int rightButtonState = 0;
  
  //(Re)set game variables for new game
  emptyFrame(clonetrisBoard);
  points = 0;
  level = 1;
  clearedLines = 0;
  boolean gameOver = false;
  
  while(!gameOver) {
    //New piece; initialise currentPiece
    copyPiece(piece[(int)random(7)], clonetrisCurrentPiece);
    clonetrisCurrentPieceX = INITIAL_PIECE_X;
    clonetrisCurrentPieceY = INITIAL_PIECE_Y;
    
    boolean boardHasChanged = true;
    boolean pieceHasLanded = false;
    
    while (!pieceHasLanded && !gameOver) {
      currentTime = millis();
      
      // has a button been pressed
      leftButtonState = digitalRead(LEFT_BUTTON_PIN);
      upButtonState = digitalRead(UP_BUTTON_PIN);
      rightButtonState = digitalRead(RIGHT_BUTTON_PIN);
      if (!aButtonWasPressed) { // if a button was not pressed at last check
        if (leftButtonState == HIGH) {
          if (!hasCollision(clonetrisCurrentPiece, clonetrisCurrentPieceX - 1, clonetrisCurrentPieceY)) {
            clonetrisCurrentPieceX--;
            boardHasChanged = true;
          }
          aButtonWasPressed = true;
        } else if (rightButtonState == HIGH) {
          if (!hasCollision(clonetrisCurrentPiece, clonetrisCurrentPieceX + 1, clonetrisCurrentPieceY)) {
            clonetrisCurrentPieceX++;
            boardHasChanged = true;
          }
          aButtonWasPressed = true;
        } else if (upButtonState == HIGH) {
          turnPieceToTheRight(clonetrisCurrentPiece, turnedPiece);
          if (!hasCollision(turnedPiece, clonetrisCurrentPieceX, clonetrisCurrentPieceY)) {
            copyPiece(turnedPiece, clonetrisCurrentPiece);
            boardHasChanged = true;
          }
          aButtonWasPressed = true;
        }
      } else if (leftButtonState == LOW && rightButtonState == LOW && upButtonState == LOW) {
        aButtonWasPressed = false;
      }
      
      // check if it's time to move piece down
      if (currentTime - lastTimePieceFell > PIECE_FALL_INTERVAL / level) {
        // will the next position cause a collision?
        if (hasCollision(clonetrisCurrentPiece, clonetrisCurrentPieceX, clonetrisCurrentPieceY + 1)) {
          //collision, so don't move piece
          mergePieceWithBoard();
          if (clonetrisCurrentPieceY == INITIAL_PIECE_Y) {
            gameOver = true;
          } else {
            boolean removedLines =
              removeFullLines();
            if (removedLines)
              boardHasChanged = true;
          }
          pieceHasLanded = true;
        } else {
          clonetrisCurrentPieceY++; // move piece 1 down
          lastTimePieceFell = currentTime;
          boardHasChanged = true;
        }
      }
      if (boardHasChanged) {
        drawBoardOnCurFrame();
        drawFrameOnDisplay();
        boardHasChanged = false;
      }
    }
  }
  delay(2000);
}

void mergePieceWithBoard() {
  // Copy falling piece to board
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      if (clonetrisCurrentPiece[x][y]) { // if the piece occupies this local coor
        //put content at board coor
        clonetrisBoard[clonetrisCurrentPieceX + x][clonetrisCurrentPieceY + y] = true;
      } // otherwise do nothing (might be something on the board there already)
    }
  }
}

boolean removeFullLines() {
  // Remove any full lines, and return true if at least one line was removed, false otherwise
  boolean oneOrMoreLinesWereRemoved = false;
  int consecutiveFullLines = 0;
  for (int line = FRAME_HEIGHT - 1; line >= 0; line--) {
    boolean lineFull = true;
    for (int column = 0; lineFull && column < FRAME_WIDTH; column++) {
      if (!clonetrisBoard[column][line]) {
        lineFull = false;
      }
    }
    if (lineFull) {
      clearedLines++; // total number of cleared lines in this game +1
      consecutiveFullLines++;
      removeLine(line);
      oneOrMoreLinesWereRemoved = true;
      line++; //since we just moved the contents of the line above to this one, make sure this line is checked again at next loop!
      // Create small "animation" by redrawing the board for each removed line:
      drawBoardOnCurFrame();
      drawFrameOnDisplay();
      delay(100);
    } else {
      if (consecutiveFullLines > 0) {
        level = (clearedLines / 10) + 1;
        int additionalPoints = POINTS_PER_LINE[consecutiveFullLines-1] * (level);
        points = points + additionalPoints;
        consecutiveFullLines = 0;
      }
    }
  }
  return oneOrMoreLinesWereRemoved;
}

void removeLine(int removeLine) {
  // Remove a particular line and move down all lines above it
  for (int line = removeLine; line >= 0; line--) {
    for (int column = 0; column < FRAME_WIDTH; column++) {
      clonetrisBoard[column][line] = (line==0) ? 0 : clonetrisBoard[column][line-1];
    }
  }
}

boolean hasCollision(boolean piece[4][4], int pieceNewX, int pieceNewY) {
  // Check whether given piece at given coordinates would cause a collision with board contents or walls
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      //only a collision risk if the piece occupies this local coordinate
      if (piece[x][y]) {
        // check if this coor of piece is outside of board on X axis
        if (pieceNewX + x < 0 || pieceNewX + x >= FRAME_WIDTH) {
          return true;
        }
        // check if this coor of piece is below board
        if (pieceNewY + y >= FRAME_HEIGHT) { //don't check top
          return true; //collision with bottom!
        }
        // only check for collision if this coor is inside board (not above)
        if (pieceNewY >= 0) {
          // check if this coor of piece collides with previous pieces on board
          if (clonetrisBoard[pieceNewX + x][pieceNewY + y]) {
            return true; // collision with something on board!
          }
        }
      }
    }
  }
  return false;
}

void drawBoardOnCurFrame() {
  // Copy contents of board plus falling piece to frame, ready for submission to display
  for (int x = 0; x < FRAME_WIDTH; x++) {
    for (int y = 0; y < FRAME_HEIGHT; y++) {
      if (clonetrisBoard[x][y]) { //something is on the board at this coordinate
        curFrame[x][y] = true;
      } else if ((x >= clonetrisCurrentPieceX && x < clonetrisCurrentPieceX+4)
                   &&
                 (y >= clonetrisCurrentPieceY && y < clonetrisCurrentPieceY+4)) {
        curFrame[x][y] = clonetrisCurrentPiece[x-clonetrisCurrentPieceX][y-clonetrisCurrentPieceY];
      } else {
        curFrame[x][y] = false;
      }
    }
  }
}

void copyPiece(boolean fromPiece[4][4], boolean toPiece[4][4]) {
  // Make an exact copy of one piece to another
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      toPiece[x][y] = fromPiece[x][y];
    }
  }
}

void turnPieceToTheRight(boolean origPiece[4][4], boolean turnedPiece[4][4]) {
  // Make a copy of a piece that is turned 90° clock-wise
  for (int x = 0; x < 4; x++) {
    for (int y = 0; y < 4; y++) {
      turnedPiece[3-y][x] = origPiece[x][y];
    }
  }
}

