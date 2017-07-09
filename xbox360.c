#include <signal.h>
#include <stdbool.h>
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
				else if (operation == ENCODED) {
					// struct auto_params*;
					auto_control(read_from_file(auto_params, "file.txt"), auto_params);
				}

		nanosleep(&delay, NULL);
	}
}

void manual_control(GAMEPAD_DEVICE dev)
{
	running = true;
	signal(SIGINT, signal_handler);

	static struct control_params_t current_params;

	current_params.led_status = 0;
	current_params.motor_speed = 0;
	current_params.servo_angle = 0;

	struct timespec ts1, ts2;
	while (running) {
		clock_gettime(CLOCK_MONOTONIC, &ts1);

		GamepadUpdate();
		if (GamepadIsConnected(dev)) {
			if (GamepadButtonTriggered(dev, LED_SWITCH)) {
				if (current_params.led_status == 0)
					current_params.led_status = 1;
				else
				  current_params.led_status = 0;
			}

			current_params.motor_speed =
			GamepadTriggerLength(dev, THROTTLE) - GamepadTriggerLength(dev, REVERSE);

			float y_val;
			GamepadStickNormXY(dev, STEERING, &current_params.servo_angle, &y_val);
		}
		else {
			current_params.motor_speed = 0;
			current_params.servo_angle = 0;
		}

		apply_params(current_params);

		clock_gettime(CLOCK_MONOTONIC, &ts2);
		long delta = (ts2.tv_sec * 1000000000L + ts2.tv_nsec) - 
			(ts1.tv_sec * 1000000000L + ts1.tv_nsec);

		long interval = delay.tv_sec * 1000000000L + delay.tv_nsec;

		if (interval > delta)
			nanosleep(interval - delta, NULL); // TODO: timespec
	}
}

void auto_control(struct auto_params_t* params) {
	struct timespec ts1, ts2;

	int i;
	for (i = 0; i < params->size; i++) {
		clock_gettime(CLOCK_MONOTONIC, &ts1);

		apply_params(params[i]);

		clock_gettime(CLOCK_MONOTONIC, &ts2);
		long delta = (ts2.tv_sec * 1000000000L + ts2.tv_nsec) - 
			(ts1.tv_sec * 1000000000L + ts1.tv_nsec);

		long interval = delay.tv_sec * 1000000000L + delay.tv_nsec;

		if (interval > delta)
			nanosleep(interval - delta, NULL); // TODO: timespec
	}
}

void apply_params(struct control_params_t P)
{
	PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, MOTOR_CENTER + P.motor_speed * MOTOR_MAX_OFFSET);
	PCA9685_setDutyCicle(bus, SERVO_CHANNEL, SERVO_CENTER - P.servo_angle * SERVO_MAX_OFFSET);
	PCA9685_setDutyCicle(bus, LED_CHANNEL, LED_MAX * P.led_status);

	P.led_status == 0 ? printf("* LED OFF\n") : printf("* LED ON\n"); 
	printf("* Motor PWM %f%\n", P.motor_speed * 100);
	printf("* Servo PWM %f%\n", P.servo_angle * 100);
}
