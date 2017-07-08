ALL:
	gcc xbox360.c main.c gamepad/gamepad.c ../i2c/i2c.c ../PCA9685/pca9685.c -ludev -lm

