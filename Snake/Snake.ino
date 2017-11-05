// Classic Snake Game, with a twist - the snake can wrap around to the other side of the board!
// Author: Robert Warmka
// 2017.11.5

// This project uses the Arduino Pro Mini, with the Atmel ATmega328 (5V, 16MHz),
// an 8x8 LED Dot Matrix, and two 74HC595 shift registers to drive the LED matrix.
//
// The code in this project is mostly portable to any size dot matrix, but the display code
// is not portable, and has been written under the assumption that the board size is 8x8.
// Any other board size or differing number of shift registers used will require a code rework.
//
// The circuit diagram has been lifted from SunFounder's Dot-matrix Display tutorial,
// available on their website (under the tutorial: Super Kit V2.0 for Arduino)
// and is included in the base directory of this project as reference.

// Credit to SunFounder for the original 8x8 LED Dot Matrix display tutorial and for the circuit diagram
// Email:support@sunfounder.com
// Website:www.sunfounder.com

// Define pins
const int latchPin = 11;//Pin connected to ST_CP of 74HC595
const int clockPin = 12;//Pin connected to SH_CP of 74HC595 
const int dataPin = 10; //Pin connected to DS of 74HC595
const int leftButton = 6; // Pin connected to the left button that makes the snake turn left
const int rightButton = 7; // Pin connected to the right button that makes the snake turn right

// Define board and timing constants
const int xMax = 8, yMax = 8; // Board size
long lastLoop = 0;
const long startingTimeDelta = 500; // How long to delay for, 2fps starting
const long endingTimeDelta = 150; // How long to delay for, 6.66fps ending
long timeDelta = startingTimeDelta;
long drawDelta = 2;

// Define game matrix
int matrix[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

// Define snake and apple classes
class SnakePart {
public:
  SnakePart() {}
  SnakePart(int x, int y) : posX(x), posY(y) {}
  void pos(int x, int y) {
    posX = x;
    posY = y;
  }
  int x() const { return posX; }
  int y() const { return posY; }
private:
  int posX, posY;
};

class Snake {
public:
  Snake(int boardX, int boardY) {
    maxSize = boardX * boardY;
    xMax = boardX;
    yMax = boardY;
    snakeList = new SnakePart[maxSize];
    reset();
  }
  
  ~Snake() {
    delete [] snakeList;
    size = 0;
  }
  
  const int getSize() const { return size; }
  const SnakePart* getSnakeList() const { return snakeList; }
  void reset() {
    snakeList[0].pos(0, yMax / 2);
    size = 1;
    direction = 0;
  }
  
  void addSnake() {
    if(size >= maxSize) {
      return;
    }
    snakeList[size].pos(lastX, lastY);
    ++size;
  }
  
  bool eat(int appleX, int appleY) {
    if(snakeList[0].x() == appleX && snakeList[0].y() == appleY) {
      addSnake();
      return true;
    }
    return false;
  }
  
  void move() {
    lastX = snakeList[size - 1].x();
    lastY = snakeList[size - 1].y();
    for(int i = size - 1; i > 0; --i) {
      // Update snake positions backwards
      snakeList[i].pos(snakeList[i-1].x(), snakeList[i-1].y());
    }
    int newX = (xMax + snakeList[0].x() + xDelta()) % xMax;
    int newY = (yMax + snakeList[0].y() + yDelta()) % yMax;
    snakeList[0].pos(newX, newY);
  }

  bool hitSelf() {
    for(int i = 1; i < size; ++i) {
      if(snakeList[0].x() == snakeList[i].x() && snakeList[0].y() == snakeList[i].y()) {
        return true;
      }
    }
    return false;
  }
  
  int xDelta() {
    switch(direction) {
    case 0:
      return 1;
      break;
    case 2:
      return -1;
      break;
    }
    return 0;
  }
  
  int yDelta() {
    switch(direction) {
    case 1:
      return 1;
      break;
    case 3:
      return -1;
      break;
    }
    return 0;
  }
  
  void left() {
    direction = (4 + (direction + 1)) % 4;
  }
  
  void right() {
    direction = (4 + (direction - 1)) % 4;
  }
private:
  SnakePart* snakeList;
  int size;
  int maxSize;
  int direction;
  int xMax, yMax;
  int lastX, lastY;
};

class Apple {
public:
  Apple(int x, int y) : xPos(x), yPos(y) {}
  void x(int x) { xPos = x; }
  void y(int y) { yPos = y; }
  int x() { return xPos; }
  int y() { return yPos; }
private:
  int xPos, yPos;
};

// Declare and initialize snake and apple objects
Snake snake(xMax, yMax);
Apple apple(0, 0);
bool win = false;
bool lose = false;
bool condition;

// Setup
void setup () {
  // Set pins to output
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(dataPin,OUTPUT);
  // Set button input pins
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);
  // Set random seed from analog read of unconnected analog pin
  randomSeed(analogRead(A1));
}

// Loop
void loop() {
  // Generate a new apple location
  newApple();
  updateBoard();
  lastLoop = millis();
  condition = game();
  if(condition) {
    setWin();
  } else {
    setLoss();
  }
  pauseState();
  reset();
}

void reset() {
  timeDelta = startingTimeDelta;
  snake.reset();
}

void setWin() {
  matrix[0] = 0x7F;
  matrix[1] = 0xFF;
  matrix[2] = 0xC0;
  matrix[3] = 0x7E;
  matrix[4] = 0x7E;
  matrix[5] = 0xC0;
  matrix[6] = 0xFF;
  matrix[7] = 0x7F;
}

void setLoss() {
  matrix[0] = 0xFF;
  matrix[1] = 0xFF;
  matrix[2] = 0xC0;
  matrix[3] = 0xC0;
  matrix[4] = 0xC0;
  matrix[5] = 0xC0;
  matrix[6] = 0xC0;
  matrix[7] = 0xC0;
}

void pauseState() {
  while(true) {
    if(handleStartButtons()) {
      return;
    }
    // Draw the board (shift data out to LED matrix via 74HC595s)
    drawBoard();
    // Wait for the draw delta before drawing again
    delay(drawDelta);
  }
}

bool game() {
  while(true) {
    // Handle button input for game
    handleGameButtons();
    // If enough time has passed to update the game state
    if((millis() - lastLoop) > timeDelta) {
      snake.move();
      lose = snake.hitSelf(); // If the snake hits itself, lose condition
      if(snake.eat(apple.x(), apple.y())) {
        // Speed up the game
        timeDelta -= ((startingTimeDelta - endingTimeDelta) / (xMax * yMax));
        if(timeDelta < endingTimeDelta) {
          timeDelta = endingTimeDelta;
        }
        // If we can't find a new apple position, then win condition
        win = !newApple();
      }
      // Update the board state with current snake and apple positions
      updateBoard();
      if(win) {
        return true;
      }
      if(lose) {
        return false;
      }
      lastLoop = millis();
    }
    // Draw the board (shift data out to LED matrix via 74HC595s)
    drawBoard();
    // Wait for the draw delta before drawing again
    delay(drawDelta);
  }
}

void updateBoard() {
  memset(matrix, 0, sizeof(matrix));
  const SnakePart* snakeList = snake.getSnakeList();
  const int snakeSize = snake.getSize();
  for(int i = 0; i < snakeSize; ++i) {
    matrix[snakeList[i].x()] = matrix[snakeList[i].x()] | (0x01 << (yMax - snakeList[i].y() - 1));
  }
  matrix[apple.x()] = matrix[apple.x()] | (0x01 << (yMax - apple.y() - 1));
}

void drawBoard() {
  for(int loopVar = 0; loopVar < 1; ++loopVar) {
    int col = 0x01;
    for(int row = 0; row < 8; ++row) {
      shiftOut(dataPin, clockPin, MSBFIRST, matrix[row]);
      shiftOut(dataPin, clockPin, MSBFIRST, ~col);
      digitalWrite(latchPin, HIGH); // pull the latchPin to save the data
      //delay(2); // wait for a microsecond
      digitalWrite(latchPin, LOW); // ground latchPin and hold low for as long as you are transmitting
      col = col << 1; // shift col over by 1
    }
  }
}

bool handleStartButtons() {
  if(leftPushed() || rightPushed()) {
    return true;
  }
  return false;
}

// Handle button pushing logic
void handleGameButtons() {
  if(leftPushed()) {
    snake.left();
  }
  if(rightPushed()) {
    snake.right();
  }
}

bool newApple() {
  int appleLoc[xMax][yMax];
  memset(appleLoc, 0, sizeof(appleLoc[0][0]) * xMax * yMax);
  const SnakePart* snakeList = snake.getSnakeList();
  int snakeSize = snake.getSize();
  for(int i = 0; i < snakeSize; ++i) {
    appleLoc[snakeList[i].x()][snakeList[i].y()] = 1;
  }
  int loc = random((xMax * yMax) - snakeSize); // random location from 0 to (boardSize - snakeSize - 1)
  int appleCounter = 0;
  for(int a = 0; a < xMax; ++a) {
    for(int b = 0; b < yMax; ++b) {
      if(appleLoc[a][b] == 0) {
        if(appleCounter < loc) {
          ++appleCounter;
        } else if(appleCounter == loc) {
          apple.x(a);
          apple.y(b);
          return true;
        }
      }
    }
  }
  return false;
}

// Debounce left button
bool leftPushed() {
  static uint16_t state = 0;
  state = (state << 1) | !digitalRead(leftButton) | 0xE000;
  if(state == 0xF000) {
    return true;
  }
  return false;
}

// Debounce right button
bool rightPushed() {
  static uint16_t state = 0;
  state = (state << 1) | !digitalRead(rightButton) | 0xE000;
  if(state == 0xF000) {
    return true;
  }
  return false;
}
