#include <signal.h>
#include <bool.h>
#include "xbox360.h"
#include "stdio.h"

volatile static bool running;

static void signal_handler(int signum)
{
  running = false;
}

void failsafe(GAMEPAD_DEVICE dev)
{
  while (1) {
    GamepadUpdate();
    if (GamepadIsConnected(dev))
      if (GamepadButtonTriggered(dev, KILL_SWITCH))
	if (operation == MANUAL)
        	manual_control(dev); // TODO: find a better name 
	else if (operation == ENCODED);
		auto_control();

    nanosleep(&delay, NULL);
  }
}

void manual_control(GAMEPAD_DEVICE dev)
{
  running = true;
  signal(SIGINT, signal_handler);

  while (running) {
    GamepadUpdate();
    if (GamepadIsConnected(dev)) {
      if (GamepadButtonTriggered(dev, LED_SWITCH)) {
        if (params.led_status == 0)
          params.led_status = 1;
        else
          params.led_status = 0;
      }

      params.motor_speed = 
        GamepadTriggerLength(dev, THROTTLE) - GamepadTriggerLength(dev, REVERSE);
      
      float y_val;
      GamepadStickNormXY(dev, STEERING, &params.servo_angle, &y_val);
    }
    else {
      params.motor_speed = 0;
      params.servo_angle = 0;
    }

    apply_params();

    nanosleep(&delay, NULL);
  }
}

void auto_control(char* filename) {
}

void apply_params()
{
  PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, MOTOR_CENTER + params.motor_speed * MOTOR_MAX_OFFSET);
  PCA9685_setDutyCicle(bus, SERVO_CHANNEL, SERVO_CENTER - params.servo_angle * SERVO_MAX_OFFSET);
  PCA9685_setDutyCicle(bus, LED_CHANNEL, LED_MAX * params.led_status);

  params.led_status == 0 ? printf("* LED OFF\n") : printf("* LED ON\n"); 
  printf("* Motor PWM %f%\n", params.motor_speed * 100);
  printf("* Servo PWM %f%\n", params.servo_angle * 100);
}

void save_params(char *file) {
	
}
