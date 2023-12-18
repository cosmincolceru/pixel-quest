// Potentiometers variables
int minThreshold = 200;
int maxThreshold = 800;
bool joyMoved = false;

// Button variables
byte lastReading = LOW;
byte swState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

unsigned long lastJoyMoved = 0;
unsigned long moveTime = 600;

const int longPressTime = 1000;
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

// Move the player with the joystick
void readJoystickMovement(Player& player, int pinX, int pinY, byte matrixSize) {
  // Read the values from the potentiometers
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);

  // check if the joystick moved up, down, left or right
  if (xValue > maxThreshold && joyMoved == false) {
    // Move player 1 position right
    player.direction = RIGHT;

    lastJoyMoved = millis();

    int newY = min(player.y + 1, matrixSize - 1);
    if (matrix[player.x][newY] == 0 && checkValidPosition(items, player.x, newY, noOfItems)) {
      matrix[player.x][player.y] = 0; 
      player.y = newY;
    }

    joyMoved = true;
  }
  if (xValue < minThreshold && joyMoved == false) {
    // Move player 1 position left
    player.direction = LEFT;

    lastJoyMoved = millis();
    
    int newY = max(player.y - 1, 0);
    if (matrix[player.x][newY] == 0 && checkValidPosition(items, player.x, newY, noOfItems)) {
      matrix[player.x][player.y] = 0; 
      player.y = newY;
    }
    
    joyMoved = true;
  }  
  if (yValue < minThreshold && joyMoved == false) {
    // Move player 1 position down
    player.direction = DOWN;

    lastJoyMoved = millis();

    int newX = min(player.x + 1, matrixSize - 1);
    if (matrix[newX][player.y] == 0 && checkValidPosition(items, newX, player.y, noOfItems)) {
      matrix[player.x][player.y] = 0;
      player.x = newX;
    }
    
    joyMoved = true;
  }
  if (yValue > maxThreshold && joyMoved == false) {
    // Move player 1 position up
    player.direction = UP;

    lastJoyMoved = millis();

    int newX = max(player.x - 1, 0);
    if (matrix[newX][player.y] == 0 && checkValidPosition(items, newX, player.y, noOfItems)) {
      matrix[player.x][player.y] = 0;
      player.x = newX;
    }
    
    joyMoved = true;
  }

  if (millis() - lastJoyMoved > moveTime) {
    joyMoved = false;
    lastJoyMoved = millis();
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && 
      yValue >= minThreshold && yValue <= maxThreshold) {
    
    joyMoved = false;
  }
}


// Check if the button was pressed and return if it was a short or a long press
int joystickButtonPress(int pinSW) {
  // Read the button state
  byte reading = !digitalRead(pinSW);

  // Debounce the button and check if the press is long or short
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }
   if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != swState) {
      swState = reading;

      if (swState == HIGH) {
        // button is pressed
        pressedTime = millis();
      }
      if (swState == LOW) {
        // button is released
        releasedTime = millis();

        // check how long the button press was
        long duration = releasedTime - pressedTime;
      
        if (duration < longPressTime) {
          lastReading = reading;
          return SHORT_PRESS;
        }
        else {
          lastReading = reading;
          return LONG_PRESS;
        }
      }
    }
  }

  lastReading = reading;
  return 0;
}

int menuMovementTone = 2000;
int menuMovementSoundTime = 100;

int readJoystickMovementMenu(int pinX, int pinY, int& currentOption, int numOptions, LiquidCrystal lcd) {
  // Read the values from the potentiometers
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);
 
  if (yValue < minThreshold && joyMoved == false) {
    lcd.clear();
    currentOption = min(numOptions - 1, currentOption + 1);    
    joyMoved = true;
    
    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }
  if (yValue > maxThreshold && joyMoved == false) {
    lcd.clear();
    currentOption = max(0, currentOption - 1);
    
    joyMoved = true;

    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && 
      yValue >= minThreshold && yValue <= maxThreshold) {
    
    joyMoved = false;
  }

  return -1;
}

byte setLcdContrastWithJoystick(int pinX, int pinY, byte contrast, LiquidCrystal lcd) {
  // Read the values from the potentiometers
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);

  if (yValue < minThreshold && joyMoved == false) {
    contrast = max(0, contrast - 10);

    EEPROM.put(lcdContrastAddress, contrast);
    joyMoved = true;
    lcd.clear();

    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }
  if (yValue > maxThreshold && joyMoved == false) {
    contrast = min(180, contrast + 10);
    EEPROM.put(lcdContrastAddress, contrast);
    joyMoved = true;
    lcd.clear();

    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && 
      yValue >= minThreshold && yValue <= maxThreshold) {
    
    joyMoved = false;
  }
  
  return contrast;
}

byte setLcdBrightnessWithJoystick(int pinX, int pinY, byte lcdBrightness, LiquidCrystal lcd) {
  // Read the values from the potentiometers
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);

  if (yValue < minThreshold && joyMoved == false) {
    lcdBrightness = max(0, lcdBrightness - 15);

    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
    joyMoved = true;
    lcd.clear();

    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }
  if (yValue > maxThreshold && joyMoved == false) {
    lcdBrightness = min(255, lcdBrightness + 15);
    EEPROM.put(lcdBrightnessAddress, lcdBrightness);
    joyMoved = true;
    lcd.clear();

    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && 
      yValue >= minThreshold && yValue <= maxThreshold) {
    
    joyMoved = false;
  }
  
  return lcdBrightness;
}

byte setMatrixBrightnessWithJoystick(int pinX, int pinY, byte matrixBrightness, LiquidCrystal lcd) {
  // Read the values from the potentiometers
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);

  if (yValue < minThreshold && joyMoved == false) {
      matrixBrightness = max(0, matrixBrightness - 1);
    EEPROM.put(matrixBrightnessAddress, matrixBrightness);
    joyMoved = true;
    lcd.clear();

    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }
  if (yValue > maxThreshold && joyMoved == false) {
    matrixBrightness = min(15, matrixBrightness + 1);
    EEPROM.put(matrixBrightnessAddress, matrixBrightness);
    joyMoved = true;
    lcd.clear();

    if (soundOn) {
      tone(buzzerPin, menuMovementTone, menuMovementSoundTime);
    }
  }

  if (xValue >= minThreshold && xValue <= maxThreshold && 
      yValue >= minThreshold && yValue <= maxThreshold) {
    
    joyMoved = false;
  }
  
  return matrixBrightness;
}

char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char getSelectedLetter() {
  return alphabet[currentLetterIndex];
}

void displayEnteredName() {
  lcd.setCursor(0, 0);
  lcd.print("Enter name:");
  lcd.setCursor(0, 1);
  lcd.print(enteredName);
  lcd.print(getSelectedLetter());
}

void setNameJoystick() {
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);


  if (yValue > maxThreshold && joyMoved == false) {
    // Letter up
    lcd.clear();
    currentLetterIndex = (currentLetterIndex + 1) % 26;

    joyMoved = true;
  }
  // check if the joystick moved up, down, left or right
  else if (yValue < minThreshold && joyMoved == false) {
    // Letter down
    lcd.clear();
    currentLetterIndex = (currentLetterIndex - 1 + 26) % 26;
    
    joyMoved = true;
  }
  else if (xValue > maxThreshold && joyMoved == false && enteredName.length() <= 10) {
    // Go to next letter
    lcd.clear();
    enteredName += getSelectedLetter();
    currentLetterIndex = 0;  // Reset to 'A'
    Serial.println("aici");
    
    joyMoved = true;
  }
  else if (xValue < minThreshold && joyMoved == false && enteredName.length() > 0) {
    // Delete letter
    char letter = enteredName[enteredName.length() - 1];
    enteredName.remove(enteredName.length() - 1);
    currentLetterIndex = letter - 'A';

    lcd.clear();
    joyMoved = true;
  }  

  if (xValue >= minThreshold && xValue <= maxThreshold && 
      yValue >= minThreshold && yValue <= maxThreshold) {
    
    joyMoved = false;
  }

  displayEnteredName();
}
