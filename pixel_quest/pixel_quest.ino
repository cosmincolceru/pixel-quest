#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "LedControl.h" 

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

#define SHORT_PRESS 1
#define LONG_PRESS 2

#define COIN 0
#define BOMB_I 1
#define BOMB_II 2

/////////////////////////////////////////////////////////////
/////////////////////////// PINS ////////////////////////////
/////////////////////////////////////////////////////////////

// Matrix driver pins
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;

// Joystick pins
const byte pinSW = 2; 
const byte pinX = A0;
const byte pinY = A1;

// LCD pins
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = A5;
const byte d6 = 3;
const byte d7 = 4;
const byte contrastPin = 5;
const byte lcdBrightnessPin = 6;

// LED pins
const byte redPin = A2;
const byte greenPin = A3; 
const byte bluePin = A4;

// Buzzer
const byte buzzerPin = 13;

/////////////////////////////////////////////////////////////
///////////////// LCD and Led Control ///////////////////////
/////////////////////////////////////////////////////////////

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); 

byte matrixBrightness;
byte lcdContrast;
byte lcdBrightness;

// Images for the matrix when moving in the menu
const uint64_t IMAGES[] PROGMEM = {
  0x040c1c3c3c1c0c04,
  0x3c3c18183c7effff,
  0x03077efccc8c1c38,
  0x1818181818001818,
  0x1000101020444438,
};

// Custom characters for the LCD
byte upArrow[8] = {
  0b00100,
	0b01110,
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000
}; 
byte downArrow[8] = {
  0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b01110,
	0b00100
};

/////////////////////////////////////////////////////////////
//////////////////////// Matrix /////////////////////////////
/////////////////////////////////////////////////////////////

const byte matrixSize = 16;
const byte fieldOfView = 8;
byte matrix[matrixSize][matrixSize] = {0};

/////////////////////////////////////////////////////////////
//////////////////////// EEPROM /////////////////////////////
/////////////////////////////////////////////////////////////

byte lcdContrastAddress = 0;      // byte
byte matrixBrightnessAddress = 1; // byte
byte soundOnAddress = 2;          // bool
byte highscoreAddress = 3;        // int*3
byte name1Address = 9;            // String
byte name2Address = 25;           // String
byte name3Address = 41;           // String
byte difficultyAddress = 60;      // byte
byte lcdBrightnessAddress = 61;

/////////////////////////////////////////////////////////////
//////////////////////// Structs ////////////////////////////
/////////////////////////////////////////////////////////////

struct Player {
  byte x, y;
  byte direction;
} player;

const byte noOfCoins = 10;
byte coinsLeft = noOfCoins;
const byte noOfBombs = 10;
const byte noOfItems = noOfCoins + noOfBombs;
struct Item {
  byte x, y;
  bool onMap;
  byte type;  
  long revealTime;
  long explodeTime;
  bool explosionDone;
  unsigned long lastBlink;
} items[noOfItems];


bool gameOver = false;
bool gameStarted = false;
bool defusing = false;
int score = 0;
int coinScore = 10;
int defuseScore = 50;
bool soundOn = true;
bool displayIntro = true;
bool introDisplayed = false;
bool levelInitiated = false;
bool displaySettings = false;
bool setLcdContrast = false;
bool setMatrixBrightness = false;
bool printAbout = false;
bool printHighscores = false;
bool setDifficulty = false;
bool printLcdSettings = false;
bool setLcdBrightness = false;
bool printHowTo = false;
bool updateHighscore = false;
bool newHighscore = false;
bool setName = false;
bool newHighscoreSet = false;


int introMessageTime = 2000;
unsigned long elapsedIntroTime = 0;

int currentOption = 0;

const byte mainMenuOptions = 5;
const byte settingsMenuOptions = 6;

String enteredName = "";
int currentLetterIndex = 0;

int coinDisappearTime = 1000; 
int bombExplosionTime = 1000;

int selectMenuOptionFrequency = 2500;
int selectMenuOptionTime = 100;

unsigned long lastLedBlink = 0;
byte ledBlinkTime = 100;

byte red = HIGH;
byte blue = LOW;

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);

  // Initiate joystick
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  // Initiate the matrix
  lc.shutdown(0, false);                 
  EEPROM.get(matrixBrightnessAddress, matrixBrightness);
  lc.setIntensity(0, matrixBrightness);  
  lc.clearDisplay(0);  

  // Initiate the LCD
  pinMode(contrastPin, OUTPUT);
  EEPROM.get(lcdContrastAddress, lcdContrast);
  analogWrite(contrastPin, lcdContrast);
  lcd.begin(16, 2);
  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  EEPROM.get(lcdBrightnessAddress, lcdBrightness);
  analogWrite(lcdBrightnessPin, lcdBrightness);

  // Initiate the RGB LED
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Buzzer
  pinMode(buzzerPin, OUTPUT);
  EEPROM.get(soundOnAddress, soundOn);

  // Set difficulty
  byte difficulty;
  EEPROM.get(difficultyAddress, difficulty);
  if (difficulty == 0) {
    // Easy
    coinDisappearTime = 2000; 
    bombExplosionTime = 2000;
  }
  else if (difficulty == 1) {
    // Medium
    coinDisappearTime = 1000; 
    bombExplosionTime = 1000;
  }
  else if (difficulty == 3) {
    // Hard
    coinDisappearTime = 500; 
    bombExplosionTime = 500;
  }
}

void loop() {
  // Display the welcome message
  if (elapsedIntroTime > introMessageTime) {
    displayIntro = false;    
  } else {
    elapsedIntroTime = millis();
  }
  if (displayIntro) {
    lcd.setCursor(3, 0);
    lcd.print(F("Welcome to"));
    lcd.setCursor(2, 1);
    lcd.print(F("Pixel Quest!"));
  }
  else {
    // After displaying the intro message clear the lcd
    if (!introDisplayed) {
      lcd.clear();
      introDisplayed = true;
    }

    // Print the main menu
    if (!gameStarted && !gameOver && !displaySettings && !setLcdContrast && !setLcdBrightness && !setMatrixBrightness && 
        !printAbout && !printHighscores && !setDifficulty && !printLcdSettings && !printHowTo) {
      
      printMenu(lcd, currentOption);

      readJoystickMovementMenu(pinX, pinY, currentOption, mainMenuOptions, lcd);

      // Display an image on the matrix
      uint64_t image;
      memcpy_P(&image, &IMAGES[currentOption], 8);
      displayImage(image);

      // Main menu options
      if (joystickButtonPress(pinSW)) {
        switch (currentOption) {
          case 0: {
            // Start game
            gameStarted = true;
            levelInitiated = false;
            displayImage(0x0000000000000000);
            lcd.clear();

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 1: {
            // Highscores
            printHighscores = true;
            lcd.clear();
            currentOption = 0;

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 2: {
            // Settings
            displaySettings = true;
            lcd.clear();
            currentOption = 0;

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 3: {
            // About
            printAbout = true;
            currentOption = 0;
            lcd.clear();

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }

          case 4: {
            // How to play
            printHowTo = true;
            currentOption = 0;
            lcd.clear();

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
        }
      }
    }

    if (printHighscores) {  
      // Read the highscores from the EEPROM
      int highscore1, highscore2, highscore3; 

      EEPROM.get(highscoreAddress, highscore1);
      EEPROM.get(highscoreAddress + sizeof(int), highscore2);
      EEPROM.get(highscoreAddress + 2*sizeof(int), highscore3);

      // Read the names from the EEPROM
      String name1;
      String name2;
      String name3;

      char chr;

      EEPROM.get(name1Address, chr);
      int idx = 1;
      while (chr != '0') {
        name1 += chr;
        EEPROM.get(name1Address + idx, chr);
        idx++;
      }

      EEPROM.get(name2Address, chr);
      idx = 1;
      while (chr != '0') {
        name2 += chr;
        EEPROM.get(name2Address + idx, chr);
        idx++;
      }

      EEPROM.get(name3Address, chr);
      idx = 1;
      while (chr != '0') {
        name3 += chr;
        EEPROM.get(name3Address + idx, chr);
        idx++;
      }

      // Find how many highscores are saved
      int highscores = 3;
      if (highscore3 == 0) {
        highscores = 2;
      }
      if (highscore2 == 0) {
        highscores = 1;
      }
      if (highscore1 == 0) {
        highscores = 0;
      }

      if (highscores == 0) {
        lcd.setCursor(0, 0);
        lcd.print(F("No highscores!"));
      }

      readJoystickMovementMenu(pinX, pinY, currentOption, highscores, lcd);

      // Display the highscores    
      if (highscores != 0) {
        if (currentOption == 0) {
          lcd.setCursor(0, 0);
          lcd.print(F("1: "));
          lcd.print(name1); 
          lcd.setCursor(0, 1);   
          lcd.print(F("Score: "));
          lcd.print(highscore1);
          if (highscore2 != 0) {
            lcd.setCursor(15, 1);
            lcd.write((byte)1);
          }
        }
      
        if (currentOption == 1) {
          lcd.setCursor(0, 0);
          lcd.print(F("2: "));
          lcd.print(name2); 
          lcd.setCursor(0, 1);   
          lcd.print(F("Score: "));
          lcd.print(highscore2); 

          lcd.setCursor(15, 0);
          lcd.write((byte)0); 
          if (highscore3 != 0) {
            lcd.setCursor(15, 1);
            lcd.write((byte)1);
          }
        }
      
        if (currentOption == 2) {
          lcd.setCursor(0, 0);
          lcd.print(F("3: "));
          lcd.print(name3); 
          lcd.setCursor(0, 1);   
          lcd.print(F("Score: "));
          lcd.print(highscore3);
          
          lcd.setCursor(15, 0);
          lcd.write((byte)0);
        }
      }

      // Exit
      if (joystickButtonPress(pinSW)) {
        lcd.clear();
        printHighscores = false;
        currentOption = 0;
        
        // Clear the matrix
        displayImage(0x0000000000000000);

        if (soundOn) {
          tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
        }
      }
    }
    
    // Print the settings menu
    if (displaySettings) {
      readJoystickMovementMenu(pinX, pinY, currentOption, settingsMenuOptions, lcd);
      printSettingsMenu(lcd, currentOption);

      if (joystickButtonPress(pinSW)) {
        switch (currentOption) {
          case 0: {
            // Set difficulty
            currentOption = 0;
            setDifficulty = true;
            displaySettings = false;
            lcd.clear();

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 1: {
            // Go to LCD settings menu
            currentOption = 0;
            printLcdSettings = true;
            displaySettings = false;
            lcd.clear();

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 2: {
            // Set matrix brightness
            displayImage(0x0000000000000000);
            currentOption = 0;
            setMatrixBrightness = true;
            displaySettings = false;
            lcd.clear();

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 3: {
            // Toggle sound on/off
            currentOption = 0;
            displaySettings = false;
            soundOn = !soundOn;
            EEPROM.put(soundOnAddress, soundOn);
            lcd.clear();
            displayImage(0x0000000000000000);

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 4: {
            // Reset highscores
            int highscore = 0;
            EEPROM.put(highscoreAddress, highscore);
            EEPROM.put(highscoreAddress + sizeof(int), highscore);
            EEPROM.put(highscoreAddress + 2*sizeof(int), highscore);   

            char chr = '0';
            EEPROM.put(name1Address, chr);
            EEPROM.put(name2Address, chr);
            EEPROM.put(name3Address, chr);  

            currentOption = 0;
            displaySettings = false;
            lcd.clear();
            displayImage(0x0000000000000000);

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
          case 5: {
            // Back
            displayImage(0x0000000000000000);
            currentOption = 0;
            displaySettings = false;
            currentOption = 0;
            lcd.clear();

            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
            break;
          }
        }
      }
    }

    if (printLcdSettings) {
      printLcdMenu(lcd, currentOption);
      readJoystickMovementMenu(pinX, pinY, currentOption, 3, lcd);

      if (joystickButtonPress(pinSW)) {
        if (currentOption == 0) {
          // Set LCD contrast
          setLcdContrast = true;
          printLcdSettings = false;
          displaySettings = false;
          currentOption = 0;
          lcd.clear();
          
          if (soundOn) {
            tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
          }

        }
      
        else if (currentOption == 1) {
          // Set LCD brightness
          setLcdBrightness = true;
          printLcdSettings = false;
          displaySettings = false;
          currentOption = 0;
          lcd.clear();
          
          if (soundOn) {
            tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
          }
        }
        else {
            // Back
            printLcdSettings = false;
            displaySettings = true;
            currentOption = 0;
            lcd.clear();
            if (soundOn) {
              tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
            }
        }
      }
    }
    if (setLcdContrast) {
      lcd.setCursor(0, 0);
      lcd.print(F("Contrast"));
      if (lcdContrast != 180) {
        lcd.setCursor(15, 0);
        lcd.write((byte)0);
      }
      if (lcdContrast != 0) {
        lcd.setCursor(15, 1);
        lcd.write((byte)1);
      }

      lcdContrast = setLcdContrastWithJoystick(pinX, pinY, lcdContrast, lcd);
      analogWrite(contrastPin, lcdContrast);
      lcd.setCursor(0, 1);
      lcd.print(lcdContrast);

      if (joystickButtonPress(pinSW)) {
        setLcdContrast = false;
        printLcdSettings = true;
        currentOption = 0;
        lcd.clear();

        if (soundOn) {
          tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
        }
      }
    }

    if (setLcdBrightness) {
      lcd.setCursor(0, 0);
      lcd.print(F("Brightness"));
      if (lcdBrightness != 255) {
        lcd.setCursor(15, 0);
        lcd.write((byte)0);
      }
      if (lcdBrightness != 0) {
        lcd.setCursor(15, 1);
        lcd.write((byte)1);
      }

      lcdBrightness = setLcdBrightnessWithJoystick(pinX, pinY, lcdBrightness, lcd);
      analogWrite(lcdBrightnessPin, lcdBrightness);
      lcd.setCursor(0, 1);
      lcd.print(lcdBrightness);

      if (joystickButtonPress(pinSW)) {
        setLcdBrightness = false;
        printLcdSettings = true;
        currentOption = 0;
        lcd.clear();

        if (soundOn) {
          tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
        }
      }
    }

    if (setDifficulty) {
      printDifficultyMenu(lcd, currentOption);
      readJoystickMovementMenu(pinX, pinY, currentOption, 3, lcd);

      if (joystickButtonPress(pinSW)) {
        if (currentOption == 0) {
          coinDisappearTime = 2000;
          bombExplosionTime = 2000;
        }
        else if (currentOption == 1) {
          coinDisappearTime = 1000;
          bombExplosionTime = 1000;
        }
        else {
          coinDisappearTime = 500;
          bombExplosionTime = 500;
        }
        EEPROM.put(difficultyAddress, currentOption);
        currentOption = 0;
        setDifficulty = false;
        displaySettings = true;
        lcd.clear();

        if (soundOn) {
          tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
        }
      }
    }


    if (setMatrixBrightness) {
      lcd.setCursor(15, 0);
      lcd.write((byte)0);
      lcd.setCursor(15, 1);
      lcd.write((byte)1);
      
      matrixBrightness = setMatrixBrightnessWithJoystick(pinX, pinY, matrixBrightness, lcd);
      lc.setIntensity(0, matrixBrightness);  
      lcd.setCursor(0, 0);
      lcd.print(matrixBrightness);

      matrixOn(matrix);
      updateMatrix(lc, matrix, matrixSize, fieldOfView);

      if (joystickButtonPress(pinSW)) {
        setMatrixBrightness = false;
        displaySettings = true;
        lcd.clear();
        clearMatrix(matrix);
        updateMatrix(lc, matrix, matrixSize, fieldOfView);

        uint64_t image;
        memcpy_P(&image, &IMAGES[2], 8);
        displayImage(image);

        if (soundOn) {
          tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
        }
      }
    }

    if (printAbout) {
      readJoystickMovementMenu(pinX, pinY, currentOption, 7, lcd);
      printAboutMenu(lcd, currentOption);

      if (joystickButtonPress(pinSW)) {
        printAbout = false;
        currentOption = 0;
        lcd.clear();

        if (soundOn) {
          tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
        }
      }
    }

    if (printHowTo) {
      readJoystickMovementMenu(pinX, pinY, currentOption, 3, lcd);
      printHowToMenu(lcd, currentOption);

      if (joystickButtonPress(pinSW)) {
        printHowTo = false;
        currentOption = 0;
        lcd.clear();

        if (soundOn) {
          tone(buzzerPin, selectMenuOptionFrequency, selectMenuOptionTime);
        }

        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, LOW);
      }
    }

    // Initiate the level
    if (gameStarted && !levelInitiated) {
      generateWalls(matrix, matrixSize);
      generateCoins(matrix, matrixSize, items, noOfCoins);
      generateBombs(matrix, matrixSize, items, noOfCoins, noOfBombs);
      player = generatePlayer(matrix, matrixSize);

      coinsLeft = noOfCoins;
      levelInitiated = true;
    }


    // Start the game
    if (gameStarted && levelInitiated) {
      if (!gameOver) {
        lcd.setCursor(0, 0);
        lcd.print(F("   Score: "));
        lcd.print(score);
        lcd.setCursor(0, 1);
        lcd.print(F("  Coins: "));
        if (coinsLeft < 10) {
          lcd.print(" ");
        }
        lcd.print(coinsLeft);
        lcd.print(F("/"));
        lcd.print(noOfCoins);
      }

      if (coinsLeft == 0) {
        gameOver = true;
        lcd.clear();
      }
      collectCoins(player, items, noOfCoins);
      // Coin disappears 1 second after finding it if it is not collected
      coinDisappear(items, noOfCoins);

      if (!defusing) {
        // Bomb explodes 1 second after finding it
        bombExplosion(items, noOfCoins, noOfItems);
      }

      // Turn off the explosion leds
      explosionOff(items, noOfCoins, noOfItems);

      breakWalls(player, matrix);
      
      readJoystickMovement(player, pinX, pinY, matrixSize);

      blinkItems(items, noOfItems, matrix);
      blinkPlayer(player);

      lightLed(player, items, noOfItems, redPin, greenPin, bluePin);

      updateMatrix(lc, matrix, matrixSize, fieldOfView);
    }

    if (gameOver) {
      gameStarted = false;

      // Read the highscores from the EEPROM
      int highscore1, highscore2, highscore3; 

      EEPROM.get(highscoreAddress, highscore1);
      EEPROM.get(highscoreAddress + sizeof(int), highscore2);
      EEPROM.get(highscoreAddress + 2*sizeof(int), highscore3);

      // Read the names from the EEPROM
      String name1;
      String name2;
      String name3;

      char chr;
      EEPROM.get(name1Address, chr);
      int idx = 1;
      while (chr != '0') {
        name1 += chr;
        EEPROM.get(name1Address + idx, chr);
        idx++;
      }

      EEPROM.get(name2Address, chr);
      idx = 1;
      while (chr != '0') {
        name2 += chr;
        EEPROM.get(name2Address + idx, chr);
        idx++;
      }

      EEPROM.get(name3Address, chr);
      idx = 1;
      while (chr != '0') {
        name3 += chr;
        EEPROM.get(name3Address + idx, chr);
        idx++;
      }
 
      if (score > highscore3 && !setName) {
        newHighscore = true;
      }

      if (newHighscore) {
          lcd.setCursor(0, 0);
          lcd.print(F(" New highscore!"));
          lcd.setCursor(0, 1);
          lcd.print(F("   Score: "));
          lcd.print(score);

          if (joystickButtonPress(pinSW)) {
            newHighscore = false;
            lcd.clear();
            setName = true;
          }
      }
        
      if (setName) {
        lcd.setCursor(0, 0);
        lcd.print(F("Enter name:"));

        setNameJoystick();

        if (joystickButtonPress(pinSW)) {
          String name = enteredName + getSelectedLetter() + '0';
          Serial.println("NUme setat");
          Serial.println(name);

          // Update highscores
          if (score > highscore1) {
            highscore3 = highscore2;
            highscore2 = highscore1;
            highscore1 = score;

            name3 = name2;
            name2 = name1;
            name1 = name;

            newHighscoreSet = true;
          }
          else if (score > highscore2) {
            highscore3 = highscore2;
            highscore2 = score;

            name3 = name2;
            name2 = name;

            newHighscoreSet = true;
          }
          else if (score > highscore3) {
            highscore3 = score;

            name3 = name;

            newHighscoreSet = true;
            }
          
          EEPROM.put(highscoreAddress, highscore1);
          EEPROM.put(highscoreAddress + sizeof(int), highscore2);
          EEPROM.put(highscoreAddress + 2*sizeof(int), highscore3);

          name1 = name1 + '0';
          name2 = name2 + '0';
          name3 = name3 + '0';

          for (int i = 0; i < name1.length(); i++) {
            EEPROM.put(name1Address + i, name1[i]);
          }
          for (int i = 0; i < name2.length(); i++) {
            EEPROM.put(name2Address + i, name2[i]);
          }
          for (int i = 0; i < name3.length(); i++) {
            EEPROM.put(name3Address + i, name3[i]);
          }

          lcd.clear();
          setName = false;
          newHighscoreSet = false;
          levelInitiated = false;
          gameOver = false;
          updateHighscore = false;
          enteredName = "";
          currentLetterIndex = 0;
          clearMatrix(matrix);
          updateMatrix(lc, matrix, matrixSize, fieldOfView);
          score = 0;
        }
      }

      if (!newHighscore && !setName) {
        if (coinsLeft == 0) {
          lcd.setCursor(0, 0);
          lcd.print(F(" No coins left!"));
        }
        else {
          lcd.setCursor(0, 0);
          lcd.print(F("   You died!"));
        }
        lcd.setCursor(0, 1);
        lcd.print(F("   Score: "));
        lcd.print(score);
      
        if (joystickButtonPress(pinSW)) {
          levelInitiated = false;
          gameOver = false;
          updateHighscore = false;
          clearMatrix(matrix);
          updateMatrix(lc, matrix, matrixSize, fieldOfView);
          lcd.clear();
          score = 0;
        }
      }
    }
  }
}

void updateMatrix(LedControl lc, byte matrix[matrixSize][matrixSize], byte matrixSize, byte fieldOfView) {
  byte minX = max(0, player.x - 3);
  byte minY = max(0, player.y - 3);
  
  minX = min(matrixSize - fieldOfView, minX);
  minY = min(matrixSize - fieldOfView, minY);

  for (int row = 0; row < fieldOfView; row++) {
    for (int col = 0; col < fieldOfView; col++) {
      lc.setLed(0, row, col, matrix[row + minX][col + minY]);
    }
  }
}

void clearMatrix(byte matrix[matrixSize][matrixSize]) {
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      matrix[i][j] = 0;
    }
  }
}

void matrixOn(byte matrix[matrixSize][matrixSize]) {
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      matrix[i][j] = 1;
    }
  }
}

void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}
