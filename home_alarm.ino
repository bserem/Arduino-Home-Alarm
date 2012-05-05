/*
  This might become a home alarm in the near future.
  At the moment it is just work in progress.
  
  Reads a digital input on pin 2, prints the result to the serial monitor 

*/

int LED = 11;
int ALARM = 10;
int muteButton = 2;
int MUTE = 0;
int pwm_value = 0;
int pwm_step = 1;

void setup() {
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
  
    
