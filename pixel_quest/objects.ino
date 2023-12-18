/////////////////////////////////////////////////////////////
/////////////////////////// Walls ///////////////////////////
/////////////////////////////////////////////////////////////


// Generate walls and return the number of walls generated
int generateWalls(byte matrix[matrixSize][matrixSize], byte matrixSize) {
  const int minWallsPercentage = 20;
  const int maxWallsPercentage = 30;

  // Calculate the minimum or maximul number of walls that can be genrated
  int minWalls = (minWallsPercentage * matrixSize * matrixSize) / 100;
  int maxWalls = (maxWallsPercentage * matrixSize * matrixSize) / 100;

  int numWalls = random(minWalls, maxWalls + 1);

  for (int i = 0; i < numWalls; i++) {
    int row = random(0, matrixSize);
    int col = random(0, matrixSize);
    matrix[row][col] = 1;
  }

  numWalls = 0;
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      numWalls += matrix[i][j];
    }
  }

  return numWalls;
}

// Break the wall in the direction of the player
void breakWalls(Player player, byte matrix[matrixSize][matrixSize]) {
  if (joystickButtonPress(pinSW) == LONG_PRESS) {
    if (player.direction == UP) {
      if (matrix[player.x - 1][player.y] == 1) {
        matrix[player.x - 1][player.y] = 0;
      }
    }
    else if (player.direction == DOWN) {
      if (matrix[player.x + 1][player.y] == 1) {
        matrix[player.x + 1][player.y] = 0;
      }
    }
    else if (player.direction == LEFT) {
      if (matrix[player.x][player.y - 1] == 1) {
        matrix[player.x][player.y - 1] = 0;
      }
    }
    else if (player.direction == RIGHT) {
      if (matrix[player.x][player.y + 1] == 1) {
        matrix[player.x][player.y + 1] = 0;
      }
    }
  }
}


/////////////////////////////////////////////////////////////
////////////////////////// Player ///////////////////////////
/////////////////////////////////////////////////////////////

Player generatePlayer(byte matrix[matrixSize][matrixSize], byte matrixSize) {
  int x = random(0, matrixSize);
  int y = random(0, matrixSize);

  while (matrix[x][y] != 0) {
    x = random(0, matrixSize);
    y = random(0, matrixSize);
  }

  return Player {x, y, 0};
}

unsigned long lastPlayerBlink;
unsigned long playerBlinkTime = 100;

void blinkPlayer(Player player) {
  if (millis() - lastPlayerBlink > playerBlinkTime) {
    matrix[player.x][player.y] = !matrix[player.x][player.y];
    lastPlayerBlink = millis();
  }
}

/////////////////////////////////////////////////////////////
//////////////////////// Coins //////////////////////////////
/////////////////////////////////////////////////////////////

// Places the coins on the map
void generateCoins(byte matrix[matrixSize][matrixSize], byte matrixSize, Item items[noOfCoins + noOfBombs], byte noOfCoins) {
  int itemsPlaced = 0;
  for (int i = 0; i < noOfCoins; i++) {
    int x = random(0, matrixSize);
    int y = random(0, matrixSize);

    while (matrix[x][y] != 0 && !checkValidPosition(items, x, y, itemsPlaced)) {
      x = random(0, matrixSize);
      y = random(0, matrixSize);
    }
    items[i] = Item {x, y, true, COIN, 0, 0, false, 0};
    itemsPlaced++;
  }
}

// Collect the coin that is near the player
void collectCoins(Player player, Item items[noOfItems], byte noOfCoins) {
  int collectCoinFrequency = 1500;
  int collectCoinSoundTime = 100;

  int coinIdx = checkNear(player, items, noOfCoins);
  if (coinIdx != 255 && joystickButtonPress(pinSW) && items[coinIdx].onMap) {
    matrix[items[coinIdx].x][items[coinIdx].y] = 0;
    items[coinIdx].onMap = false;
    score += coinScore;
    if (soundOn) {
      tone(buzzerPin, collectCoinFrequency, collectCoinSoundTime);
    }
    coinsLeft--;
  }
}

void coinDisappear(Item items[noOfItems], byte noOfCoins) {
  int disappearCoinFrequency = 300;
  int disappearCointSoundTime = 300;

  for (int i = 0; i < noOfCoins; i++) {
    if (items[i].revealTime != 0 && items[i].onMap) {
      if (millis() - items[i].revealTime > coinDisappearTime) {
        matrix[items[i].x][items[i].y] = 0;
        items[i].onMap = false;
        if (soundOn) {
          tone(buzzerPin, disappearCoinFrequency, disappearCointSoundTime);
        }
        coinsLeft--;
      }
    }
  }
}

/////////////////////////////////////////////////////////////
//////////////////////// Bombs //////////////////////////////
/////////////////////////////////////////////////////////////

// Places bombs on the map
void generateBombs(byte matrix[matrixSize][matrixSize], byte matrixSize, Item items[noOfItems], byte noOfCoins, byte noOfBombs) {
  int itemsPlaced = noOfCoins;
  for (int i = noOfCoins; i < noOfCoins + noOfBombs; i++) {
    int x = random(0, matrixSize);
    int y = random(0, matrixSize);

    while (matrix[x][y] != 0 && !checkValidPosition(items, x, y, itemsPlaced)) {
      x = random(0, matrixSize);
      y = random(0, matrixSize);
    }

    // Calculate the probability of the bomb being type I or II
    int prob = random(100);
    if (prob < 40) {
      items[i] = Item {x, y, true, BOMB_II, 0, 0, false, 0};
    }
    else {
      items[i] = Item {x, y, true, BOMB_I, 0, 0, false, 0};
    }

    itemsPlaced++;
  }
}



void bombExplosion(Item items[noOfItems], byte noOfCoins, byte noOfItems) {
  int explosionFrequency = 1000;
  int explosionSoundTime = 800;

  for (byte i = noOfCoins; i < noOfItems; i++) {
    if (items[i].revealTime != 0 && items[i].onMap) {
      // Serial.println("AICI");
      if (millis() - items[i].revealTime > bombExplosionTime) {
        matrix[items[i].x][items[i].y] = 0;
        items[i].onMap = false;
        items[i].explodeTime = millis();

        if (soundOn) {
          tone(buzzerPin, explosionFrequency, explosionSoundTime);
        }
        // Explosion animation
        explosion(i, 1);
      }
    }
  }
}

void explosion(int i, int state) {
  const int bombRange = 2;
  for (int j = 1; j <= bombRange; j++) {
    int directions = 4;
    int dx[] = { 0, 1, 0,-1 };
    int dy[] = { 1, 0,-1, 0 }; 

    for (int k = 0; k < directions; k++) {
      int newX = items[i].x + j * dx[k];
      int newY = items[i].y + j * dy[k];

      if (newX >= 0 && newX < matrixSize &&
          newY >= 0 && newY < matrixSize &&
          checkValidPosition(items, newX, newY, noOfItems)) {
        
        
        matrix[newX][newY] = state;
      
        if (player.x == newX && player.y == newY) {
          gameOver = true;
          lcd.clear();
        }
      }
    }
  }
  for (int j = 1; j <= bombRange - 1; j++) {
    int directions = 4;
    int dx[] = {1, 1,-1,-1};
    int dy[] = {1,-1, 1,-1};

    for (int k = 0; k < directions; k++) {
      int newX = items[i].x + j * dx[k];
      int newY = items[i].y + j * dy[k];

      if (newX >= 0 && newX < matrixSize &&
          newY >= 0 && newY < matrixSize &&
          checkValidPosition(items, newX, newY, noOfItems)) {
        
        matrix[newX][newY] = state;
      
        if (player.x == newX && player.y == newY) {
          gameOver = true;
          lcd.clear();
        }
      }
    }
  }
}

const int bombExplodedTime = 1000;

void explosionOff(Item items[noOfItems], byte noOfCoins, byte noOfItems) {
  for (int i = noOfCoins; i < noOfItems; i++) {
    if (items[i].explodeTime != 0 && !items[i].explosionDone) {
      if (millis() - items[i].explodeTime > bombExplodedTime) {

        items[i].explosionDone = true;
        explosion(i, 0);
      }
    }
  }
}

/////////////////////////////////////////////////////////////
//////////////////////// Items //////////////////////////////
/////////////////////////////////////////////////////////////

bool checkValidPosition(Item items[noOfItems], int x, int y, int itemsPlaced) {
  for (int i = 0; i < itemsPlaced; i++) {
    if (items[i].x == x && items[i].y == y && items[i].onMap) {
      return false;
    }
  }
  return true;
} 

// Return the index of the item that is near the player
byte checkNear(Player player, Item items[noOfItems], byte noOfItems) {
  int directions = 4;
  int dx[] = {-1, 0, 1, 0};
  int dy[] = { 0, 1, 0,-1};

  for (byte i = 0; i < directions; i++) {
    int posX = player.x + dx[i];
    int posY = player.y + dy[i];

    for (byte j = 0; j < noOfItems; j++) {
      if (posX == items[j].x && posY == items[j].y && items[j].onMap == true) {
        if (items[j].revealTime == 0) {
          items[j].revealTime = millis();
        }

        return j;
      }
    }
  }
  // Return a value that doesn't exist in the items array
  return 255;
}

int defuseFrequency = 1800;
int defuseSoundTime = 100;

unsigned long defuseStartTime = 0;
int defuseTime = 1000;

// Light the RGB LED acording to the item that is near the player
void lightLed(Player player, Item items[noOfItems], byte noOfItems, byte redPin, byte greenPin, byte bluePin) {
  byte index = checkNear(player, items, noOfItems);
  
  // Check if the index returned represents an items that is still on the map
  if (index != 255 && items[index].onMap) {
    if (items[index].type == COIN) {
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      digitalWrite(bluePin, LOW);
    }
    else if (items[index].type == BOMB_I) {
      // byte red = HIGH;
      if (millis() - lastLedBlink > ledBlinkTime) {
        red = !red;
        lastLedBlink = millis();
      }
      
      digitalWrite(redPin, red);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, LOW);
    }
    else if (items[index].type == BOMB_II) {
      // If the bomb is type II you can defuse it by holding the button for 1 second
      if (!digitalRead(pinSW)) {
        defusing = true;
        if (defuseStartTime == 0) {
          defuseStartTime = millis();
        }
      }
      else {
        if (defusing) {
          if (millis() - defuseStartTime > defuseTime) {
            // Serial.println("DEFUSED");
            score += defuseScore;
            if (soundOn) {
              tone(buzzerPin, defuseFrequency, defuseSoundTime);
            }
            items[index].onMap = false;
            matrix[items[index].x][items[index].x] = 0;
            updateMatrix(lc, matrix, matrixSize, fieldOfView);
          }
        }
        defusing = false;
        defuseStartTime = 0;
      }

      if (millis() - lastLedBlink > ledBlinkTime) {
        blue = red;
        red = !red;
        lastLedBlink = millis();
      }
      digitalWrite(redPin, red);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, blue);
    }
    else {
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, LOW);
   }
  }
  else {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }
}

unsigned long itemBlinkTime = 500;

void blinkItems(Item items[noOfItems], byte noOfItems, byte matrix[matrixSize][matrixSize]) {
  for (int i = 0; i < noOfItems; i++) {
    if (millis() - items[i].lastBlink > itemBlinkTime && items[i].onMap) {
      matrix[items[i].x][items[i].y] = !matrix[items[i].x][items[i].y];
      
      items[i].lastBlink = millis();
    }
  }
}


