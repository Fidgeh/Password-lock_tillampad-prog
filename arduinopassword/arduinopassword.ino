// Mitt projekt för att skapa ett password lock
#include <EEPROM.h>


const int buttonPins[] = {2, 3, 4, 5};
const int programButtonPin = 6;

int inputCode[4];
int storedCode[4];
int inputIndex = 0;
bool codeMatched = false;
bool codeSet = false;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(programButtonPin, INPUT_PULLUP);

  if (EEPROM.read(0) != 0xFF) {
    for (int i = 0; i < 4; i++) {
      storedCode[i] = EEPROM.read(i);
    }
    codeSet = true;
    Serial.println("Kod hittad i EEPROM.");
  } else {
    Serial.println("Ingen kid hittad. Du måste sätta en kod.");
  }

  if (digitalRead(programButtonPin) == LOW) {
    Serial.println("Programmeringsläge aktiverat: Mata in ny kod.");
    setNewCode();
  } else {
    if (!codeSet) {
      Serial.println("Ingen kod är satt. Starta om och håll nere programmeringsknappen.");
    } else {
      Serial.println("Normalläge: mata in den lagrade koden med knapparna.");
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!codeSet) {
    return;
  }
  
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
    checkCode();
    inputIndex = 0;
  }

}

// funktion för att skapa en ny kod


void setNewCode() {
  int newCode[4];
  int newCodeIndex = 0;

  while (newCodeIndex < 4) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(200);
        newCode[newCodeIndex] = i + 1;
        Serial.print("Ny kodsiffra: ");
        Serial.println(i + 1);
        newCodeIndex++;
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


void checkCode() {
  codeMatched = true;
  for (int i = 0; i < 4; i++) {
    if (inputCode[i] != storedCode[i]) {
      codeMatched = false;
      break;
    }
  }
  if (codeMatched) {
    Serial.println("Rätt kod!");
  } else {
    Serial.println("Fel kod, försök igen!");
  }
    
}
// funktion för att skriva in och kolla om lösenordet är rätt

// funktion för att öppna dörr
