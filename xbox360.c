#include <signal.h>
#include <stdbool.h>
#include "xbox360.h"
#include "stdio.h"

volatile static bool running;
volatile static bool add_to_buffer;

static void signal_handler(int signum)
{
	running = false;
}

void failsafe(GAMEPAD_DEVICE dev)
{
	struct control_params_t P;
	P.motor_speed = P.servo_angle = P.led_status = 0;
	apply_params(P);

	while (1) {
		GamepadUpdate();
		if (GamepadIsConnected(dev)) {
			if (GamepadButtonTriggered(dev, MANUAL_CONTROL)) {
				add_to_buffer = false;
				manual_control(dev);
			}
			if (GamepadButtonTriggered(dev, ENCODE_READ)) {
				struct auto_params_t* apt = read_from_file(filename);
				auto_control(apt);
			}
			else if (GamepadButtonTriggered(dev, ENCODE_WRITE)) {
				add_to_buffer = true;
				params = (struct control_params_t*)malloc(sizeof(struct control_params_t)*1024);
				int size = manual_control(dev);
				write_to_file(params, size, interval, filename);
			}
		}
	}
}

int manual_control(GAMEPAD_DEVICE dev)
{
	running = true;
	signal(SIGINT, signal_handler);

	static struct control_params_t current_params;

	current_params.led_status = 0;
	current_params.motor_speed = 0;
	current_params.servo_angle = 0;

	struct timespec ts1, ts2;
	int i;

	for (i = 0; running; i++) {
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

		params[i].led_status = current_params.led_status;
		params[i].motor_speed = current_params.motor_speed;
		params[i].servo_angle = current_params.servo_angle;

		clock_gettime(CLOCK_MONOTONIC, &ts2);
		long delta = (ts2.tv_sec * 1000000000L + ts2.tv_nsec) - 
			(ts1.tv_sec * 1000000000L + ts1.tv_nsec);

		if (interval > delta) {
			ts1.tv_sec = 0;
			ts1.tv_nsec = interval * 1000000L - delta;
			nanosleep(&ts1, NULL); // TODO: timespec
		}
	}

	return i;
}

void auto_control(struct auto_params_t* autoDef) {
	struct timespec ts1, ts2;

	int i;
	for (i = 0; i < autoDef->n; i++) {
		clock_gettime(CLOCK_MONOTONIC, &ts1);

		/* DOES THE MAGIC */
		apply_params(autoDef->params[i]);

		clock_gettime(CLOCK_MONOTONIC, &ts2);
		long delta = (ts2.tv_sec * 1000000000L + ts2.tv_nsec) - 
			(ts1.tv_sec * 1000000000L + ts1.tv_nsec);

		long interval = delay.tv_sec * 1000000000L + delay.tv_nsec;

		if (interval > delta) {
			ts1.tv_sec = 0;
			ts1.tv_nsec = interval * 1000000L - delta;
			nanosleep(&ts1, NULL); // TODO: timespec
		}
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
