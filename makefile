MOTOR = "-DMOTOR_OFFSET=6"
SAMPLES = "-DSAMPLE_NUM=20480"

ALL:
	gcc xbox360.c main.c gamepad/gamepad.c ../i2c/i2c.c ../PCA9685/pca9685.c encoder.c TCS3472/tcs3472.c -ludev -lm -g $(MOTOR) $(SAMPLES)
