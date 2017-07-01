#include "../i2c/i2c.h"
#include "../PCA9685/pca9685.h"
#include "gamepad/gamepad.h"

#define CONTROLLER GAMEPAD_0
#define KILL_SWITCH BUTTON_A
#define THROTTLE TRIGGER_RIGHT
#define REVERSE TRIGGER_LEFT
#define STEERING STICK_LEFT
#define LED_SWITCH BUTTON_RIGHT_SHOULDER

#define SERVO_CHANNEL 0
#define MOTOR_CHANNEL 1
#define LED_CHANNEL 2

#define MOTOR_CENTER 50
#define SERVO_CENTER 50
#define MOTOR_MAX_OFFSET 25
#define SERVO_MAX_OFFSET 10
#define LED_MAX 80

#define I2C_BUS "/dev/i2c-1"

struct control_params_t {
  int led_status;
  float motor_speed;
  float servo_angle;
};

i2c bus;
struct control_params_t params;

void failsafe(GAMEPAD_DEVICE dev);
void manual_control(GAMEPAD_DEVICE dev);
void apply_params();
