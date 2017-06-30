#include "xbox360.h"

int main() {
  i2c bus = i2c_open(I2C_BUS);

  params.led_status = 0;
  params.motor_speed = 0;
  params.servo_angle = 0;

  GamepadInit();

  failsafe(CONTROLLER);

  return 0;
}
