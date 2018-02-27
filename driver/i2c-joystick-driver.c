// http://elinux.org/Interfacing_with_I2C_Devices

#include <stdio.h>
#include <stdlib.h>

#include "I2C.h"
#include "JoystickDevice.h"

#define I2C_GAMEPAD_ADDRESS 0x18
#define UPDATE_FREQ 5000 // ms (200Hz)

typedef struct {
  uint16_t buttons; // button status
} I2CJoystickStatus;

int readI2CJoystick(int file, I2CJoystickStatus *status) {
  int s = readI2CSlave(file, I2C_GAMEPAD_ADDRESS, status, sizeof(I2CJoystickStatus));
  if(s != sizeof(I2CJoystickStatus))
    return -1; // error

  return 0; // no error
}

#define TestBitAndSendKeyEvent(oldValue, newValue, bit, event) if((oldValue & (1 << bit)) != (newValue & (1 << bit))) sendInputEvent(UInputFIle, EV_KEY, event, (newValue & (1 << bit)) == 0 ? 0 : 1);

void updateUInputDevice(int UInputFIle, I2CJoystickStatus *newStatus, I2CJoystickStatus *status) {
  // update button event
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 0, BTN_A);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 1, BTN_B);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 2, BTN_X);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 3, BTN_Y);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 4, BTN_UP);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 5, BTN_DOWN);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 6, BTN_LEFT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 7, BTN_RIGHT);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 8, BTN_START);
  TestBitAndSendKeyEvent(status->buttons, newStatus->buttons, 9, BTN_SELECT);
}

int main(int argc, char *argv[]) {
  // open I2C device  
  int I2CFile = openI2C(3);

  // current joystick status
  I2CJoystickStatus status;
  status.buttons = 0;

  // create uinput device
  int UInputFIle = createUInputDevice();

  printf("Driver ready\n");

  while(1) {
    // read new status from I2C
    I2CJoystickStatus newStatus;
    if(readI2CJoystick(I2CFile, &newStatus) != 0) {
      printf("can't read I2C device!\n");
    } else {
      // everything is ok
      updateUInputDevice(UInputFIle, &newStatus, &status);
      status = newStatus;
    }

    // sleep until next update
    usleep(UPDATE_FREQ);
  }

  // close file
  close(I2CFile);
  ioctl(UInputFIle, UI_DEV_DESTROY);
}
