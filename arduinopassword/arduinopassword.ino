/*
File: arduinopassword
Name: Figge Hellstrom

*/

/*
EEPROM stores a password without a continuos power supply. Servo is the motor used for the lock.
Here I define my variables.

*/

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

// Creates "Mode" which has three different possible values. Is used to let the program know what mode it is in.
enum Mode { NORMAL, VERIFYING, CODE_CHANGE };
Mode currentMode = NORMAL;


/*

Define INPUT/OUTPUT for pins. 
Puts Servo at 0 degrees.
Checks if a code is Stored in the EEPROM memory.

*/
void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  myServo.write(0);

  

  // Pins 1-4 become INPUT.
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(changeCodeButtonPin, INPUT_PULLUP);
  pinMode(piezoPin, OUTPUT);
  digitalWrite(piezoPin, LOW);

  // Checks if there is something stored in first position of EEPROM. If something is stored the first 4 positions will be stored in storedCode.
  if (EEPROM.read(0) != 0xFF) {
    for (int i = 0; i < 4; i++) {
      storedCode[i] = EEPROM.read(i);
    }
    codeSet = true;
    Serial.println("Code found in EEPROM.");
  } else {
    Serial.println("No code found. You have to enter a code");
  }

  for (int i = 0; i < 4; i++) {
  Serial.print(storedCode[i]);
  Serial.print(" ");
}
Serial.println();

  // lets you put in a new code if no code was found in EEPROM memory.  
  if (!codeSet) {
    Serial.println("Entering Code Change mode. Enter a code.");
    currentMode = CODE_CHANGE;
    setNewCode();
  } else {
    Serial.println("Normal mode: Enter code:");
  }

}


/*
Starts void loop.
Checks if the code change button is being pressed. 

*/
void loop() {
  switch (currentMode) {
    case NORMAL:
      if (digitalRead(changeCodeButtonPin) == LOW) {
        delay(200);
        Serial.println("Code change button pressed. Entering Code change mode.");
        currentMode = VERIFYING;
        inputIndex = 0;
      } else {
        handleCodeEntry();
      }
      break;

    // in the VERIFYING mode. Checking if code is correct so that code can be changed.
    case VERIFYING:
      if (readInputCode()) {
        if (checkCode()) {
          Serial.println("Correct code.");
          currentMode = CODE_CHANGE;
          setNewCode();
          currentMode = NORMAL;
          Serial.println("Returning to normal mode:");
        } else {
          Serial.println("Wrong code. Returning to normal mode:");
          currentMode = NORMAL;
        }
      }
      break;
    
    case CODE_CHANGE:
      break;
  }
}

// Handels the code entry by checking if the buttons are being pressed.
void handleCodeEntry() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(200);
      if (inputIndex < 4) {
        inputCode[inputIndex] = i + 1;
        inputIndex++;
        Serial.print("Number entered: ");
        Serial.println(i + 1);
      }
    }
  }
  // Checks if code matches stored code when 4 digits have been entered.
  if (inputIndex >= 4) {
    if (checkCode()) {
      Serial.println("Correct!");
      failedAttempts = 0;
      // opens lock if it is closed
      if (lockOpen == false) {
        myServo.write(90);
        lockOpen = true;
        // closes lock if it is open.
      } else if (lockOpen == true) {
        myServo.write(0);
        lockOpen = false;
      }
      
      // Prints wrong code if the digits do not match the stored code.
    } else {
      Serial.println("Wrong Code! Try again.");
      failedAttempts++;
      if (failedAttempts >= maxFailedAttempts) {
        activatePiezo();
        failedAttempts = 0;
      }
    }
    inputIndex = 0;
  }

}

// Reads the password by checking button presses. Stops reading if 4 digits have been entered.
bool readInputCode() {
  while (inputIndex < 4) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(200);
        inputCode[inputIndex] = i + 1;
        inputIndex++;
        Serial.print("Entered digit: ");
        Serial.println(i + 1);
        if (inputIndex >= 4) {
          break;
        }
      }
    }
  }
  return true;
}

// Function for setting a new password and storing it in EEPROM.
void setNewCode() {
  int newCode[4];
  int newCodeIndex = 0;

  Serial.println("Enter new password");

  while (newCodeIndex < 4) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(200);
        newCode[newCodeIndex] = i + 1;
        newCodeIndex++;
        Serial.print("New number: ");
        Serial.println(i + 1);
        if (newCodeIndex >= 4) {
          break;
        }
      }
    }
  }
  // storing the password in EEPROM.
  for (int i = 0; i < 4; i++) {
    EEPROM.write(i, newCode[i]);
    storedCode[i] = newCode[i];
  }
  codeSet = true;
  Serial.println("New password saved in EEPROM!");
}

// Checks if the entered password matches the stored password.
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

// Function that activates a piezo if the password is wrong 3 times in a row.
void activatePiezo() {
  Serial.println("Wrong password three times! Activating piezo!");
  for (int i = 0; i < 3; i++) {
    tone(piezoPin, 1000);
    delay(500);
    noTone(piezoPin);
    delay(500);
  }
}






