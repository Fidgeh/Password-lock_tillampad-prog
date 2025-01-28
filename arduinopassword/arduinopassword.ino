// Mitt projekt för att skapa ett password lock
#include <EEPROM.h>
#include <Servo.h>

Servo myServo;

const int buttonPins[] = {2, 3, 4, 5};
const int changeCodeButtonPin = 6;
const int piezoPin = 7;
const int servoPin = 9;

int inputCode[4];
int storedCode[4];
int inputIndex = 0;
bool codeMatched = false;
bool codeSet = false;
bool lockOpen = false;

int failedAttempts = 0;
const int maxFailedAttempts = 3;

enum Mode { NORMAL, VERIFYING, PROGRAMMING };
Mode currentMode = NORMAL;

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  myServo.write(0);

  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(changeCodeButtonPin, INPUT_PULLUP);
  pinMode(piezoPin, OUTPUT);
  digitalWrite(piezoPin, LOW);

  if (EEPROM.read(0) != 0xFF) {
    for (int i = 0; i < 4; i++) {
      storedCode[i] = EEPROM.read(i);
    }
    codeSet = true;
    Serial.println("Kod hittad i EEPROM.");
  } else {
    Serial.println("Ingen kid hittad. Du måste sätta en kod.");
  }

    if (!codeSet) {
    Serial.println("Startar i programmeringsläge för att sätta en ny kod.");
    currentMode = PROGRAMMING;
    setNewCode();
  } else {
    Serial.println("Normalläge: Mata in den lagrade koden med knapparna.");
  }

}

void loop() {
  switch (currentMode) {
    case NORMAL:
      if (digitalRead(changeCodeButtonPin) == LOW) {
        delay(200);
        Serial.println("Programmeringsknapp tryckt. Ange befintlig kod för att byta kod.");
        currentMode = VERIFYING;
        inputIndex = 0;
      } else {
        handleCodeEntry();
      }
      break;

    case VERIFYING:
      if (readInputCode()) {
        if (checkCode()) {
          Serial.println("Korrekt kod. Gå in i programmeringsläge.");
          currentMode = PROGRAMMING;
          setNewCode();
          currentMode = NORMAL;
          Serial.println("Återvänder till normalläge.");
        } else {
          Serial.println("Fel kod. Återvänder till normalläge.");
          currentMode = NORMAL;
        }
      }
      break;
    
    case PROGRAMMING:
      break;
  }
}


void handleCodeEntry() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(200);
      if (inputIndex < 4) {
        inputCode[inputIndex] = i + 1;
        inputIndex++;
        Serial.print("Siffra inmatad: ");
        Serial.println(i + 1);
      }
    }
  }

  if (inputIndex >= 4) {
    if (checkCode()) {
      Serial.println("Rätt kod!");
      failedAttempts = 0;
      if (lockOpen == false) {
        myServo.write(90);
        lockOpen = true;
      
      } else if (lockOpen == true) {
        myServo.write(0);
        lockOpen = false;
      }
      
    } else {
      Serial.println("Fel kod, försök igen!");
      failedAttempts++;
      if (failedAttempts >= maxFailedAttempts) {
        activatePiezo();
        failedAttempts = 0;
      }
    }
    inputIndex = 0;
  }

}


bool readInputCode() {
  while (inputIndex < 4) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(200);
        inputCode[inputIndex] = i + 1;
        inputIndex++;
        Serial.print("Inmatad siffra: ");
        Serial.println(i + 1);
        if (inputIndex >= 4) {
          break;
        }
      }
    }
  }
  return true;
}


void setNewCode() {
  int newCode[4];
  int newCodeIndex = 0;

  Serial.println("Mata in den nya koden:");

  while (newCodeIndex < 4) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(200);
        newCode[newCodeIndex] = i + 1;
        newCodeIndex++;
        Serial.print("Ny kodsiffra: ");
        Serial.println(i + 1);
        if (newCodeIndex >= 4) {
          break;
        }
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    EEPROM.write(i, newCode[i]);
    storedCode[i] = newCode[i];
  }
  codeSet = true;
  Serial.println("Ny kod satt och sparad i EEPROM.");
}


bool checkCode() {
  codeMatched = true;
  for (int i = 0; i < 4; i++) {
    if (inputCode[i] != storedCode[i]) {
      codeMatched = false;
      break;
    }
  }
  if (codeMatched) {
    return true;
  } else {
    return false;
  }
    
}

void activatePiezo() {
  Serial.println("Fel 3 gånger! Aktiverar piezo.");
  for (int i = 0; i < 3; i++) {
    tone(piezoPin, 1000);
    delay(500);
    noTone(piezoPin);
    delay(500);
  }
}






