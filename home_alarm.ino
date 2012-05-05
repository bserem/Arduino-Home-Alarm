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
//  (I write from down to top, as to see it like arduino pins)
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
//  ..to understand how pins are used
#define PIN_ZONE_6      A5
#define PIN_ZONE_5      A4
#define PIN_ZONE_4      A3
#define PIN_ZONE_3      A2
#define PIN_ZONE_2      A1
#define PIN_ZONE_1      A0
#define ALL_ZONES        6

// Alarm arm types
#define ARM_ALL          0
#define ARM_1            1
#define ARM_2            2
#define ARM_3            3

// Other defines
#define PASSWORD_LENGTH   4
#define FACTORY_PASSWORD  {0, 0, 0, 0} // Should be of PASSWORD_LENGTH 
#define CELL_PHONE_LENGTH 10

// Eeprom adresses
// ..No other way(?) than hardcoded addresses
// ..order matters, as each one is calculated from the previous one
#define ADDR_START              0
#define ADDR_PASSWORD           ADDR_START + 1
#define ADDR_PANIC_PASSWORD     ADDR_PASSWORD + PASSWORD_LENGTH * sizeof(char)
#define CELL_PHONE_1            ADDR_PANIC_PASSWORD + PASSWORD_LENGTH * sizeof(char)  // In case wirh AT commands we can communicate with a cell phone
#define CELL_PHONE_2            CELL_PHONE_1 + CELL_PHONE_LENGTH * sizeof(char)
#define ADDR_ARM_ALL_MASK       CELL_PHONE_2 + CELL_PHONE_LENGTH * sizeof(char)       // With one byte we can store zone masks
#define ADDR_ARM_1_MASK         ADDR_ARM_ALL_MASK + sizeof(byte)                      // ..assuming that zones won't be more than 8
#define ADDR_ARM_2_MASK         ADDR_ARM_ARM_1_MASK + sizeof(byte)                    // ..and store each zone mask to one bit
#define ADDR_ARM_3_MASK         ADDR_ARM_ARM_2_MASK + sizeof(byte)
#define ADDR_ZONES_NC_VOLT      ADDR_ARM_ARM_3_MASK + sizeof(byte)                    // Starting point, helper define for use in for-loops
#define ADDR_ZONE_1_NC_VOLT     ADDR_ZONES_NC_VOLT
#define ADDR_ZONE_2_NC_VOLT     ADDR_ZONE_1_NC_VOLT + sizeof(int)
#define ADDR_ZONE_3_NC_VOLT     ADDR_ZONE_2_NC_VOLT + sizeof(int)
#define ADDR_ZONE_4_NC_VOLT     ADDR_ZONE_3_NC_VOLT + sizeof(int)
#define ADDR_ZONE_5_NC_VOLT     ADDR_ZONE_4_NC_VOLT + sizeof(int)
#define ADDR_ZONE_6_NC_VOLT     ADDR_ZONE_5_NC_VOLT + sizeof(int)

// Function prototypes
// Init all nececary libraries or external modules
void init_libraries();
// Check given password to eeprom stored one
boolean check_password(char check_password[PASSWORD_LENGTH], boolean panic_password);
// Reads zones information from eeprom to memory
void init_zones();
// Reprogram the whole system (pass, zones, ..)
void reprogram_system();
// Test system, by opening and closing everything
void test_system();
// Prompt user with a message, show keys presses and a description
char prompt(char prompt_messafe[], boolean mask, char descr_message[], char term1, char term2, char message_pressed[], int max_keys, int timeout);
// Show message
// void message();

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
boolean check_password(char check_password[PASSWORD_LENGTH], boolean panic_password) {
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
    // type casting from (byte)value to (char)value
    // ..apparently this is not an error, however it should be done
    // ..for better programming code style
    if (check_password[pos] != (char)value) {
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
  // Clean eeprom with zeros (Check EEPROM example 'eeprom_clear')
  
  // Set password to factory reset
  
  // Ask new password
  // ..save new password
  
  // Ask new panic password
  // .. save new panic password
  
  // Ask user for cell phone 1
  // ..store cell phone 1
  // Ask user for cell phone 2
  // ..store cell phone 2
  
  // Ask user how many zones exist
  
  // Force user to close all doors and windows
  // .. read all analog inputs
  // .. save all inputs as NC values for all zones
  
  // We assume ARM_ALL is for all doors, windows closed
  // For ARM_1, we ask user to press number from 1 to ALL_ZONES
  // .. and we toggle screen number to show which zones will be checked
  // .. for example screen shows   '1.2.3.4.5.6'
  // .. user press 4, screen gets  '1.2.3.-.5.6'
  // .. user press 6, screen gets  '1.2.3.-.5.-'
  // .. user press 4, screen gets  '1.2.3.4.5.-'
  // .. user press '*', we lock 12345 zones and ignore zone 6
  
  // Do the same for ARM_2, and ARM_3
  
  // Message: System is ready
}

// Test system, by opening and closing everything
void test_system() {
  // Not to forget a message before each action!
  
  // Set all pixels of screen to black-white-black-white, 3 seconds each
  // .. to test screen good working status
  
  // Try to send sms to cell phone 1
  // Try to send sms to cell phone 2
  
  // Beep 3 times to test beeper good working status
  // Set alarm for 5 seconds to test alarm siren good working status
  
  // Check all zones
    // If a zone is open
    // .. beep (keypad beeper, or lcd message - not home siren alarm)
    
    // Read keypress from keypad
    // .. Stop until '*' is pressed
}

// Prompt user with a message, show keys presses and a description
// char prompt_messafe[]  : A null terminating pointer to the text we want to prompt the user
// boolean mask           : true, if we don't want to print users input (for example password)
// char descr_message[]   : A null terminating pointer to the description to show, for example "Press * to cancel or # to accept"
// char term1             : If this key is pressed, funcion will stop and return term1 value
// char term2             : If this key is pressed, funcion will stop and return term2 value
// char message_pressed[] : Whatever user inputs, it will be stored here
// int max_keys           : We won't accept more than max_keys as we may step into other variables in memory
// int timeout            : If we have timeout, we stop and return 0. Set 0 to disable

// Example 1, password input: (assume all variables already exist)
// prompt("Please give password\0", true, "Press # to accept or * to cancel", '#', '*', return_value, PASSWORD_LENGTH, 30);
// Example 2, accept to test: (assume all variables already exist, here 'return_value' is reduntunt)
// prompt("Close all doors and windows\0", false, "Press # to test system or * to cancel test", '#', '*', return_value, 0, 30);

char prompt(char prompt_messafe[], boolean mask, char descr_message[], char term1, char term2, char message_pressed[], int max_keys, int timeout) {
  char in_text[100]; // Apparently 100 is too much
  int cur_pos, itemp, start_time;
  char new_char;
  
  new_char = 0;
  // ?? start_time = time();
  while(1) {
    // If timeout is <= 0, then time out is disabled
    // ?? if (time() - start_time > timeout && timeout > 0)
    // ??   break;
    // ?? lcd.clean(); // Clean screen
    // ?? lcd.print(prompt_messafe); // print prompt
    // ?? lcd.print(": ");
    
    // To avoid cases where no input is needed
    if (message_pressed && max_keys > 0) {
      if (mask == true) {
        itemp = 0;
        while(itemp < max_keys && message_pressed[itemp] != 0) {
          // ?? lcd.print_one_char('*');
          itemp++;
        }
      }
      else {
        // ?? lcd.print(message_pressed);
      }
    }
    // ?? lcd.print("\n");
    
    // ?? lcd.print(descr_message); // print description
    // ?? if (keyborad.there_is_availiable_character()) {
    // ??   new_char = keyboard.get_availiable_character();
    // ??   if (new_char == term1) // is this terminating character 1?
    // ??     return term1;
    // ??   if (new_char == term2) // is this terminating character 2?
    // ??     return term2;
    // ??   if (cur_pos + 1 < max_keys) { // + 1 goes for null terminating character
    // ??     message_pressed[cur_pos] = new_char;
    // ??     message_pressed[cur_pos + 1] = 0;
    // ??     cur_pos++;
    // ??   }
    // ?? }
  }
  
  // We have time out, just leave
  return 0;
}

