#ifndef ENCODER_H
#define ENCODER_H

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

static const char ENCODER_FILE_MAGIC[] = {0xdc, 0x19, 0x93, 0x8f};

struct control_params_t {
  int led_status;
  float motor_speed;
  float servo_angle;
};

struct auto_params_t {
	char magic[4];
	double period;
	size_t n;
	struct control_params_t *params;
};

struct auto_params_t *read_from_file(const char *file);
int write_to_file(struct control_params_t *params, size_t size, double period, const char *file);

#endif
