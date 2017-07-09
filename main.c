#include "xbox360.h"
#include "err.h"

int main(int argc, char** args) {
	if (argc < 2)
		err(EXIT_FAILURE, USAGE_STRING);

	if (operation == MANUAL || operation == ENCODED)
		operation = atoi(args[1]);
  else if (operation == ENCODED)
    if (argc < 3)
      err(EXIT_FAILURE, USAGE_STRING);
    else
     // filename certo 
	else 
		err(EXIT_FAILURE, USAGE_STRING);
		
  bus = i2c_open(I2C_BUS);

  delay.tv_sec = 0;
  delay.tv_nsec = 10000000L;

  PCA9685_setFreq(bus, 320);
  PCA9685_init(bus);

  PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, 50);
  PCA9685_setDutyCicle(bus, SERVO_CHANNEL, 50);
  PCA9685_setDutyCicle(bus, LED_CHANNEL, 50);

  params = (control_params_t*)malloc(sizeof(control_params_t) * MAX_SIZE);

  params[0].led_status = 0;
  params[0].motor_speed = 0;
  params[0].servo_angle = 0;

  GamepadInit();

  failsafe(CONTROLLER);

  PCA9685_stop(bus);

  return 0;
}
