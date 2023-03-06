#include "config.h"
#ifdef USE_WIOT_BUTTONS

#include <Arduino.h>
// https://easybtn.earias.me/
#include <EasyButton.h>
extern "C" {
#include <uxn.h>
#include <devices/controller.h>
}

EasyButton up_button(WIO_5S_UP);
EasyButton down_button(WIO_5S_DOWN);
EasyButton left_button(WIO_5S_LEFT);
EasyButton right_button(WIO_5S_RIGHT);
EasyButton press_button(WIO_5S_PRESS);
EasyButton c_button(WIO_KEY_C);
EasyButton b_button(WIO_KEY_B);
EasyButton a_button(WIO_KEY_A);

int 
devctrl_init()
{
  up_button.begin();
  down_button.begin();
  left_button.begin();
  right_button.begin();
  press_button.begin();
  c_button.begin();
  b_button.begin();
  a_button.begin();
  return 1;
}

/**
 * For the controller layout, see https://compudanzas.net/uxn_tutorial_day_3.html
 * It follows the standard NES layout, see https://www.nesdev.org/wiki/Standard_controller 
 * 
 * 0 - A
 * 1 - B
 * 2 - Select
 * 3 - Start
 * 4 - Up
 * 5 - Down
 * 6 - Left
 * 7 - Right
 * 
 * On the Wio Terminal, pressing the joystick yields "Select".
 * The three buttons on the top sholder, from left to right, yield 
 * 
 * left   - A
 * middle - B
 * right  - Start 
 */
int
devctrl_handle(Device *d)
{
  up_button.read();
  if (up_button.wasPressed()) {
    controller_down(d, 0x10);
  }
  if (up_button.wasReleased()) {
    controller_up(d, 0x10);
  }
  down_button.read();
  if (down_button.wasPressed()) {
    controller_down(d, 0x20);
  }
  if (down_button.wasReleased()) {
    controller_up(d, 0x20);
  }
  left_button.read();
  if (left_button.wasPressed()) {
    controller_down(d, 0x40);
  }
  if (left_button.wasReleased()) {
    controller_up(d, 0x40);
  }
  right_button.read();
  if (right_button.wasPressed()) {
    controller_down(d, 0x80);
  }
  if (right_button.wasReleased()) {
    controller_up(d, 0x80);
  }
  press_button.read();
  if (press_button.wasPressed()) {
    controller_down(d, 0x04);
  }
  if (press_button.wasReleased()) {
    controller_up(d, 0x04);
  }
  c_button.read();
  if (c_button.wasPressed()) {
    controller_down(d, 0x08);
  }
  if (c_button.wasReleased()) {
    controller_up(d, 0x08);
  }
  b_button.read();
  if (b_button.wasPressed()) {
    controller_down(d, 0x02);
  }
  if (b_button.wasReleased()) {
    controller_up(d, 0x02);
  }
  a_button.read();
  if (a_button.wasPressed()) {
    controller_down(d, 0x01);
  }
  if (a_button.wasReleased()) {
    controller_up(d, 0x01);
  }
  return 1;
}

#endif