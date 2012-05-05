/*
  This might become a home alarm in the near future.
  At the moment it is just work in progress.
  
  Reads a digital input on pin 2, prints the result to the serial monitor 

*/

#include <EEPROM.h>

// We try to use defines as much as we can, instead of global variables
// ..to reduce even more memory footprint, apparently while programm is running
// ..no pin connection will change. We are really far away from plug'n'play pins!
// ..[arduino_program]/reference/Define.html

// Digital In & Out (Communication)
//  (I write from down to top, as to see is like arduino pins)
/*
#define *************** 14
#define *************** 13
#define *************** 12 // KEYPAD BEEPER?
*/
#define PIN_SERIAL_AT_2 11
#define PIN_SERIAL_AT_1 10
#define PIN_ALARM        9
#define PIN_SCREEN_5     8
#define PIN_SCREEN_4     7
#define PIN_SCREEN_3     6
#define PIN_SCREEN_2     5
#define PIN_SCREEN_1     4
#define PIN_KEYBOARD_1   3
#define PIN_KEYBOARD_2   2
#define PIN_SERIAL_RX    1
#define PIN_SERIAL_TX    0

// All Analog In & Out (Zones)
//  (I write from down to top, as to see is like arduino pins)
//  probably won't be used anywhere, however it should be clear
//  to understand how pins are used
#define PIN_ZONE_6      A5
#define PIN_ZONE_5      A4
#define PIN_ZONE_4      A3
#define PIN_ZONE_3      A2
#define PIN_ZONE_2      A1
#define PIN_ZONE_1      A0
#define ALL_ZONES        6

// Other defines
#define PASSWORD_LENGTH  4
#define FACTORY_PASSWORD {0, 0, 0, 0} // Should be of PASSWORD_LENGTH 

// Eeprom adresses
// ..No other way(?) than hardcoded addresses
// ..order matters, as each one is calculated from the previous one
#define ADDR_START              0
#define ADDR_PASSWORD           ADDR_START + 1
#define ADDR_PANIC_PASSWORD     ADDR_PASSWORD + PASSWORD_LENGTH * sizeof(byte)


// Function prototypes
// Init all nececary libraries or external modules
void init_libraries();
// Check given password to eeprom stored one
boolean check_password(byte check_password[PASSWORD_LENGTH], boolean panic_password);
// Reads zones information from eeprom to memory
void init_zones();
// Reprogram the whole system (pass, zones, ..)
void reprogram_system();
// Test system, by opening and closing everything
void test_system();

byte password[ PASSWORD_LENGTH ];
byte password_panic[ PASSWORD_LENGTH ];




int LED = 11;
int ALARM = 10;
int muteButton = 2;
int MUTE = 0;
int pwm_value = 0;
int pwm_step = 1;

void setup() {
  // init_libraries();
  // init_zones();
  
  
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(ALARM, OUTPUT);
  pinMode(muteButton, INPUT);
}

void mute() {
  MUTE = 1;
  analogWrite(ALARM, 0);
  analogWrite(LED, 0);
}

void reset() {
  MUTE = 0;
  analogWrite(ALARM, 0);
  analogWrite(LED, 0);
}

void sound_alarm() {
  analogWrite(ALARM, pwm_value);
  analogWrite(LED, pwm_value);
  pwm_value = pwm_value + pwm_step;
  if (pwm_value == 0 || pwm_value == 255) { pwm_step = -pwm_step; } //pwm is from 0 to 255 on arduino
}

void loop() {
  int zone0 = analogRead(A0);
  Serial.println(analogRead(A0));
  if (zone0 < 1000 && MUTE == 0) { //perimeter tampered, sound alarm
    sound_alarm();
  }
  if (zone0 > 1000) {
    reset();
  }
  
  if (digitalRead(muteButton) != 0) {
    mute();
  }
}

// Init all nececary libraries or external modules
void init_libraries() {
  
}

// Check given password to eeprom stored one
boolean check_password(byte check_password[PASSWORD_LENGTH], boolean panic_password) {
  byte value;
  int start_addr;
  
  if (panic_password) {
    start_addr = ADDR_PANIC_PASSWORD;
  }
  else {
    start_addr = ADDR_PASSWORD;
  }
  for(int pos = 0; pos < PASSWORD_LENGTH; pos++) {
    value = EEPROM.read(start_addr + pos);
    if (check_password[pos] != value) {
      return false;
    }
  }
  return true;
}

// Reads zones information from eeprom to memory
void init_zones() {
  
}

// Reprogram the whole system (pass, zones, ..)
void reprogram_system() {
  // Ask new password
  // ..save new password
  
  // Ask new panic password
  // .. save new panic password
  
  // Force user to close all doors and windows
  // .. read all analog inputs
  // .. save all inputs as NC values for all zones
  
  // Message: System is ready
}

// Test system, by opening and closing everything
void test_system() {
  // Check all zones
    // If a zone is open
    // .. beep
    
    // Read keypress from keypad
    // .. Stop until '*' is pressed
}

