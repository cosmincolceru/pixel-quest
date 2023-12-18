void printMenu(LiquidCrystal lcd, int currentOption) {
  String options[mainMenuOptions + 1] = {
    "Start game",
    "Highscores",
    "Settings",
    "About",
    "How to play",
    ""
  };

  lcd.setCursor(0, 0);
  lcd.print(F("> "));
  lcd.print(options[currentOption]);
  lcd.setCursor(0, 1);
  lcd.print(F(" "));
  lcd.print(options[currentOption + 1]);
  
  if (currentOption != 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }
  if (currentOption != mainMenuOptions - 1) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }
}

void printSettingsMenu(LiquidCrystal lcd, int currentOption) {\
  String settings[settingsMenuOptions + 1] = {
    "Difficulty",
    "LCD",
    "Matrix",
    "Sound ON/OFF",
    "Reset data",
    "Back",
    ""
  };
  lcd.setCursor(0, 0);
  lcd.print(F("> "));
  lcd.print(settings[currentOption]);
  lcd.setCursor(0, 1);
  lcd.print(F(" "));
  lcd.print(settings[currentOption + 1]);
  
  if (currentOption != 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }
  if (currentOption != settingsMenuOptions - 1) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }
}


void printDifficultyMenu(LiquidCrystal lcd, int currentOption) {
  String difficultySettings[4] = {
    "Easy",
    "Medium",
    "Hard",
    ""
  };

  lcd.setCursor(0, 0);
  lcd.print(F("> "));
  lcd.print(difficultySettings[currentOption]);
  lcd.setCursor(0, 1);
  lcd.print(F(" "));
  lcd.print(difficultySettings[currentOption + 1]);
  
  if (currentOption != 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }
  if (currentOption != 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }
}

String lcdSetting[4] = {
  "Contrast",
  "Brightness",
  "Back",
  ""
};

void printLcdMenu(LiquidCrystal lcd, int currentOption) {
  lcd.setCursor(0, 0);
  lcd.print(F("> "));
  lcd.print(lcdSetting[currentOption]);
  lcd.setCursor(0, 1);
  lcd.print(F(" "));
  lcd.print(lcdSetting[currentOption + 1]);
  
  if (currentOption != 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)0);
  }
  if (currentOption != 3) {
    lcd.setCursor(15, 1);
    lcd.write((byte)1);
  }
}




void printAboutMenu(LiquidCrystal lcd, int currentOption){
  String about[8] = {
    "Pixel Quest",
    "Author: Cosmin ",
    "Colceru", 
    "GitHub: https://",
    "github.com/cosmi",
    "ncolceru/pixel-q",
    "est/"
    ""
  };
  lcd.setCursor(0, 0);
  lcd.print(about[currentOption]);
  lcd.setCursor(0, 1);
  lcd.print(about[currentOption + 1]);
}



void printHowToMenu(LiquidCrystal lcd, int currentOption) {
  String howTo[4] = {
    "Collect coins",
    "Run from bomb I",
    "Defuse bomb II",
    ""
  };
  if (currentOption == 0) {
    lcd.setCursor(0, 0);
    lcd.print(F("Collect coins"));

    lcd.setCursor(15, 1);
    lcd.write((byte)1);

    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
  }
  else if (currentOption == 1) {
    lcd.setCursor(0, 0);
    lcd.print(F("Run from bomb I"));

    lcd.setCursor(15, 0);
    lcd.write((byte)0);
    lcd.setCursor(15, 1);
    lcd.write((byte)1);

    
    if (millis() - lastLedBlink > ledBlinkTime) {
      red = !red;
      lastLedBlink = millis();
    }
    
    digitalWrite(redPin, red);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print(F("Defuse bomb II"));

    lcd.setCursor(15, 0);
    lcd.write((byte)0);


    if (millis() - lastLedBlink > ledBlinkTime) {
      blue = red;
      red = !red;
      lastLedBlink = millis();
    }
    digitalWrite(redPin, red);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, blue);
  }
}