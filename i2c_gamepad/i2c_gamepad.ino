#include <Wire.h>

// ------------------------------------------
// Configuration
// ------------------------------------------
#define I2C_ADDRESS 0x18
#define SWITCH_DEBOUNCE_TIME 50 // ms
#define POWER_LED_PIN 13

// switch type definition
#define BTN_LEFT        0x00
#define BTN_DOWN        0x01
#define BTN_UP          0x02
#define BTN_RIGHT       0x03
#define BTN_SELECT      0x04
#define BTN_START       0x05
#define BTN_A           0x06
#define BTN_B           0x07
#define BTN_X           0x08
#define BTN_Y           0x09

struct InputSwitch {
  unsigned char pin;
  unsigned char state;
  unsigned long time;
  unsigned char code;
};

// return true if switch state has changed!
bool updateSwitch(struct InputSwitch *sw) {
  int newState = digitalRead(sw->pin);

  if(newState != sw->state && millis() - sw->time > SWITCH_DEBOUNCE_TIME) {
    // change state!
    sw->state = newState;

    // record last update
    sw->time = millis();
    
    return true;
  }

  // else 
  return false;
}

// I2C data definition
struct I2CJoystickStatus {
  uint16_t buttons; // button status
};

I2CJoystickStatus joystickStatus;

// the 8 switches 
InputSwitch switches[] = {
  {3, HIGH, 0, BTN_LEFT},
  {4, HIGH, 0, BTN_DOWN},
  {5, HIGH, 0, BTN_UP},
  {6, HIGH, 0, BTN_RIGHT},
  {7, HIGH, 0, BTN_SELECT},
  {9, HIGH, 0, BTN_START},
  {12, HIGH, 0, BTN_A},
  {11, HIGH, 0, BTN_B},
  {10, HIGH, 0, BTN_X},
  {8, HIGH, 0, BTN_Y}
};

void (*stateFunction)(void);

void runState() {
  scanInput();
}

void setup()
{
  Wire.begin(I2C_ADDRESS);      // join i2c bus 
  Wire.onRequest(requestEvent); // register event

  // prints title with ending line break
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("I2C Gamepad controller");

  // default status
  joystickStatus.buttons = 0;
  
  // pin configuration
  for(int i = 0; i < sizeof(switches) / sizeof(InputSwitch); i++) {
    pinMode(switches[i].pin, INPUT_PULLUP);
  }

  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);

  // switch to run state
  stateFunction = runState;
}

void scanInput() {
  for(int i = 0; i < sizeof(switches) / sizeof(InputSwitch); i++) {
    if(updateSwitch(&switches[i])) {
      if(switches[i].state == HIGH) // button released
        joystickStatus.buttons &= ~(1 << switches[i].code);
      else // button pressed
        joystickStatus.buttons |= (1 << switches[i].code);
    }
  }

  static uint16_t oldButtons = 0;
  if(joystickStatus.buttons != oldButtons) {
    Serial.println(joystickStatus.buttons);
    oldButtons = joystickStatus.buttons;
  }
}

void loop() {
  // execute state function
  stateFunction();
  delay(10);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write((char *)&joystickStatus, sizeof(I2CJoystickStatus)); 
}