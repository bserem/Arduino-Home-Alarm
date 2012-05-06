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

#define ZONE_CIRCUIT_OPEN    1
#define ZONE_CIRCUIT_CLOSED  0

// Alarm arm types
#define NOT_ARMED        0
#define ARM_1            1
#define ARM_2            2
#define ARM_3            3
#define ARM_4            4
#define ARMS_COUNT       4

// Other defines
#define PASSWORD_LENGTH   4
#define FACTORY_PASSWORD  {0, 0, 0, 0} // Should be of PASSWORD_LENGTH 
#define CELL_PHONE_LENGTH 10

#define ADDR_START        0
#define MD5_MAX_NUMBER    65521 // The biggest prime number under unsigned int = 65,535

// Helper structure to load and save everything at once
struct info_struct_prototype {
  char password[PASSWORD_LENGTH];
  char panic_password[PASSWORD_LENGTH];
  char cell_phone_1[CELL_PHONE_LENGTH];
  char cell_phone_2[CELL_PHONE_LENGTH];
  byte arms_mask[ARMS_COUNT];
  unsigned int zones_nc_volt[ALL_ZONES];
  unsigned int open_circuit_volt_threshold[ALL_ZONES];  
  unsigned int md5;
};

// Function prototypes

// Init all nececary libraries or external modules
void init_libraries();
// Check given password to eeprom stored one
boolean check_password(char check_password[PASSWORD_LENGTH], boolean panic_password);
// Load all info from memory
void load_info();
// Save all info to memory
void save_info();
// Reprogram the whole system (pass, zones, ..)
void reprogram_system();
// Test system, by opening and closing everything
void test_system();
// Prompt user with a message, show keys presses and a description
char prompt(char prompt_messafe[], boolean mask, char descr_message[], char term1, char term2, char message_pressed[], int max_keys, int timeout);
// Returns true or false if zone circuit is open
boolean is_zone_circuit_open();
// Functions for lcd screen
void lcd_print_string(char * print_string);
void lcd_clear();
void lcd_write_char(char c1);
// Functions for keyboard
boolean availiable_key();
char get_key();

struct info_struct_prototype info;

int LED = 11;
int ALARM = 10;
int muteButton = 2;
int MUTE = 0;
int pwm_value = 0;
int pwm_step = 1;

void setup() {
  // init_libraries();
  load_info();
  
  
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
  char value;
  int start_addr;
  
  for(int pos = 0; pos < PASSWORD_LENGTH; pos++) {
    if (panic_password) {
      value = info.panic_password[pos];
    }
    else {
      value = info.password[pos];
    }
      
    if (check_password[pos] != value) {
      return false;
    }
  }
  return true;
}

// Load all info from memory
void load_info() {
  // Idea from http://arduino.cc/playground/Code/EEPROMWriteAnything
  byte* p;
  
  p = (byte *)(void *)&info;
  for(int i = 0; i < sizeof(struct info_struct_prototype); i++) {
    *p++ = EEPROM.read(ADDR_START + i);
  }

  // data corruption is not allowed!
  int existing_md5 = info.md5;
  int new_md5 = 0; 
  info.md5 = 0;
  p = (byte *)(void *)&info;
  for(int i = 0; i < sizeof(struct info_struct_prototype); i++) {
    new_md5 += (new_md5 + *p++) % MD5_MAX_NUMBER;
  }
  info.md5 = existing_md5; // Putting back existing one, not the calculated one (not to forget that this is maybe corrupted)
  
  if (new_md5 != existing_md5) {
    // system is corrupted!
    // What should we do?
    // Inform the user? how?
    reprogram_system(); // Is this correct?
  }
  
}

// Save all info to memory
void save_info() {
  // Idea from http://arduino.cc/playground/Code/EEPROMWriteAnything
  const byte* p;
  
  // data corruption is not allowed!
  info.md5 = 0;
  int new_md5 = 0;
  p = (const byte *)(const void *)&info;
  for(int i = 0; i < sizeof(struct info_struct_prototype); i++) {
    new_md5 += (new_md5 + *p++) % MD5_MAX_NUMBER;
  }
  info.md5 = new_md5;
  
  p = (const byte *)(const void *)&info;
  for(int i = 0; i < sizeof(struct info_struct_prototype); i++) {
    EEPROM.write(ADDR_START + i, *p++);
  }
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
  
  // Save the system
  save_info();
  
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

char prompt(char prompt_message[], boolean mask, char descr_message[], char term1, char term2, char message_pressed[], int max_keys, int timeout) {
  char in_text[100]; // Apparently 100 is too much
  int cur_pos, itemp, start_time;
  char new_char;
  // Anything starting with '??' is code and has to be checked and solved
  
  new_char = 0;
  // ?? start_time = time();
  while(1) {
    // If timeout is <= 0, then time out is disabled
    // ?? if (time() - start_time > timeout && timeout > 0)
    // ??   break;
    
    // Lcd refresh:
    // lcd can keep from its own, all screen data.
    // .. so maybe there is no need to refresh screen
    // .. but only in case a new key has been pressed,
    // .. or time out has come.
    lcd_clear();                      // Clear screen
    lcd_print_string(prompt_message); // print prompt
    lcd_print_string(": ");
    
    // To avoid cases where no input is needed
    if (message_pressed && max_keys > 0) {
      if (mask == true) {
        itemp = 0;
        while(itemp < max_keys && message_pressed[itemp] != 0) {
          lcd_print_string("*");
          itemp++;
        }
      }
      else {
        lcd_print_string(message_pressed);
      }
    }
    lcd_print_string("\n");
    
    lcd_print_string(descr_message); // print description
    if (availiable_key()) {
      new_char = get_key();
      if (new_char == term1) // is this terminating character 1?
        return term1;
      if (new_char == term2) // is this terminating character 2?
        return term2;
      if (cur_pos + 1 < max_keys) { // + 1 goes for null terminating character
        message_pressed[cur_pos] = new_char;
        message_pressed[cur_pos + 1] = 0;
        cur_pos++;
      }
    }
  }
  
  // We have time out, just leave
  return 0;
}

// Returns true or false if zone circuit is open
boolean is_zone_circuit_open(int zone_number) {
  int circuit_value;
  switch(zone_number) {
    // If count of zones changes, we have to add/remove lines here
    case 1: circuit_value = analogRead(PIN_ZONE_1); break;
    case 2: circuit_value = analogRead(PIN_ZONE_2); break;
    case 3: circuit_value = analogRead(PIN_ZONE_3); break;
    case 4: circuit_value = analogRead(PIN_ZONE_4); break;
    case 5: circuit_value = analogRead(PIN_ZONE_5); break;
    case 6: circuit_value = analogRead(PIN_ZONE_6); break;
    default: return ZONE_CIRCUIT_OPEN; // Log this somewhere?
  }
  
  if (abs(circuit_value - info.zones_nc_volt[zone_number - 1]) > info.open_circuit_volt_threshold[zone_number - 1]) {
    return ZONE_CIRCUIT_OPEN;
  }
  
  return ZONE_CIRCUIT_CLOSED;
}

void lcd_print_string(char * print_string) {
  // ??
}

void lcd_clear() {
  // ??
}

void lcd_write_char(char c1) {
  // ??
}

boolean availiable_key() {
  // ??
  return false;
}

char get_key() {
  // ??
  return 0;
}

