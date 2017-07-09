#include "xbox360.h"
#include "err.h"

int main(int argc, char** args)
{
	if (argc < 3)
		err(EXIT_FAILURE, USAGE_STRING);

	interval = atoi(args[1]) * 1000000L; // Received interval in ms to ns
	filename = args[2];
	
	bus = i2c_open(I2C_BUS);

	PCA9685_setFreq(bus, 320);
	PCA9685_init(bus);

	GamepadInit();

	if (operation == MANUAL)
		failsafe(CONTROLLER);
	else
		err(EXIT_FAILURE, USAGE_STRING); 

	PCA9685_stop(bus);

	return 0;
}
