#include "encoder.h"
#include "../i2c/i2c.h"
#include "../PCA9685/pca9685.h"
#include "gamepad/gamepad.h"

#define CONTROLLER GAMEPAD_0
#define THROTTLE TRIGGER_RIGHT
#define REVERSE TRIGGER_LEFT
#define STEERING STICK_LEFT
#define LED_SWITCH BUTTON_RIGHT_SHOULDER

#define MANUAL_CONTROL BUTTON_A
#define ENCODE_WRITE BUTTON_B
#define ENCODE_READ BUTTON_X

#define SERVO_CHANNEL 0
#define MOTOR_CHANNEL 1
#define LED_CHANNEL 2

#define MOTOR_CENTER 50
#define SERVO_CENTER 50
#define MOTOR_MAX_OFFSET 15
#define SERVO_MAX_OFFSET 10
#define LED_MAX 100

#define MANUAL 0
#define ENCODED 1

#define I2C_BUS "/dev/i2c-1"

#define USAGE_STRING "seu cretino, executou errado, porra!\n" // TODO: change

i2c bus;
struct control_params_t* params;
char* filename;
char operation;
short interval;
struct timespec delay;

void failsafe(GAMEPAD_DEVICE dev);
int manual_control(GAMEPAD_DEVICE dev);
struct control_params_t* load_params(char* filename);
void auto_control(struct auto_params_t* params);
void apply_params(struct control_params_t P);
