#include <signal.h>
#include <stdbool.h>
#include "xbox360.h"
#include "stdio.h"
#include "TCS3472/tcs3472.h"

volatile static bool running, add_to_buffer, rgb_is_running;

void timespec_diff(struct timespec *start, struct timespec *stop, struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }   

    return;
}

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
				params = (struct control_params_t*)malloc(sizeof(struct control_params_t)*SAMPLE_NUM);
				if (params == NULL)
					err(EXIT_FAILURE, "Malloc failed\n");
				int size = manual_control(dev);
				printf("Write finished with %d samples.\n", size); 

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

	struct timespec ts1;
	int i;

	for (i = 0; running; i++) {
		clock_gettime(CLOCK_BOOTTIME, &ts1);

		GamepadUpdate();
		if (GamepadIsConnected(dev)) {
			if (GamepadButtonTriggered(dev, STOP_BUTTON)) {
				running = false;
				continue;
			}
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

		if (add_to_buffer) {
			params[i].led_status = current_params.led_status;
			params[i].motor_speed = current_params.motor_speed;
			params[i].servo_angle = current_params.servo_angle;
		}

		ts1.tv_nsec += interval;
		
		while (ts1.tv_nsec >= 1e9) {
			ts1.tv_nsec -= 1e9;
			ts1.tv_sec++;
		}
		
		clock_nanosleep(CLOCK_BOOTTIME, TIMER_ABSTIME, &ts1, NULL);
	}

	return i;
}

void auto_control(struct auto_params_t* autoDef) {
	struct timespec ts1;

	int i;
	/*TODO: init. rgb_is_running*/
	for (i = 0; i < autoDef->n; i++) {
		clock_gettime(CLOCK_BOOTTIME, &ts1);

		/* DOES THE MAGIC */
		apply_params(autoDef->params[i]);

		ts1.tv_nsec += interval;
		
		while (ts1.tv_nsec >= 1e9) {
			ts1.tv_nsec -= 1e9;
			ts1.tv_sec++;
		}
		
		clock_nanosleep(CLOCK_BOOTTIME, TIMER_ABSTIME, &ts1, NULL);
	}
}

void apply_params(struct control_params_t P)
{
	// PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, MOTOR_CENTER + P.motor_speed * MOTOR_MAX_OFFSET);

	if (P.motor_speed > 0)
		PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, MOTOR_OFFSET);
	else if (P.motor_speed < 0)
		PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, -MOTOR_OFFSET);
	else
		PCA9685_setDutyCicle(bus, MOTOR_CHANNEL, MOTOR_CENTER);;

	PCA9685_setDutyCicle(bus, SERVO_CHANNEL, SERVO_CENTER - P.servo_angle * SERVO_MAX_OFFSET);
//	PCA9685_setDutyCicle(bus, LED_CHANNEL, LED_MAX * P.led_status);

	if(P.led_status) {
		rgb_is_running = true;
	} else {
		rgb_is_running = false;
	}

/*	P.led_status == 0 ? printf("* LED OFF\n") : printf("* LED ON\n"); 
	printf("* Motor PWM %f%\n", P.motor_speed * 100);
	printf("* Servo PWM %f%\n", P.servo_angle * 100); */
}

void *rgb_thread(void *args)
{
	struct timespec ts;
	color currentColor;

	ts.tv_sec = 0;
	ts.tv_nsec += interval;

	TCS3472_setIntegrationTime(bus, TCS3472_ATIME_24MS);
	TCS3472_powerOn(bus);

	/*TODO: define better loop cond*/
	while(true) {
		while(!rgb_is_running) clock_nanosleep(CLOCK_BOOTTIME, 0, &ts, NULL);

		do {
			TCS3472_getColor(bus, &currentColor);
		} while(currentColor.red < WHITE_TRESHOLD
				&& currentColor.green < WHITE_TRESHOLD
				&& currentColor.blue < WHITE_TRESHOLD);

		PCA9685_setDutyCicle(bus, LED_CHANNEL, LED_MAX);
	}
}
