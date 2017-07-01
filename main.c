#include "xbox360.h"

int main() {
  bus = i2c_open(I2C_BUS);
  
  PCA9685_setFreq(bus, 300);
  PCA9685_init(bus);

  PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, 50);
  PCA9685_setDutyCicle(bus, SERVO_CHANNEL, 50);
  PCA9685_setDutyCicle(bus, LED_CHANNEL, 50);

  params.led_status = 0;
  params.motor_speed = 0;
  params.servo_angle = 0;

  GamepadInit();

  failsafe(CONTROLLER);

  return 0;
}
