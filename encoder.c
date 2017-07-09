#include "encoder.h"

struct auto_params_t *read_from_file(const char *file)
{
	int fd;
	struct auto_params_t *apt;
	char buf[sizeof(ENCODER_FILE_MAGIC)];
	size_t size;

		fd = open(file, O_RDWR);

	if(fd < 0) {
		perror("read_from_file");
		return NULL;
	}

	memset(buf, '\0', sizeof(ENCODER_FILE_MAGIC));

	if(pread(fd, buf, sizeof(ENCODER_FILE_MAGIC), offsetof(struct auto_params_t, magic)) < 0) {
		perror("read_from_file:pread:magic");
		close(fd);
		return NULL;
	}

	if(strncmp(buf, ENCODER_FILE_MAGIC, sizeof(ENCODER_FILE_MAGIC))) {
		fprintf(stderr, "read_from_file: Not a params file\n");
		close(fd);
		return NULL;
	}

	if(pread(fd, &size, sizeof(size_t), offsetof(struct auto_params_t, n)) < 0) {
		perror("read_from_file:pread:n");
		close(fd);
		return NULL;
	}

	apt = (struct auto_params_t *)mmap(NULL,
			sizeof(struct auto_params_t) + sizeof(struct control_params_t)*size,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(apt == MAP_FAILED) {
		perror("read_from_file:mmap");
		close(fd);
		return NULL;
	}

	apt->params = (struct control_params_t *)&apt[1] ;

	return apt;
}

void auto_params_free(struct auto_params_t *apt) {
	munmap((void *)apt, sizeof(struct auto_params_t) + sizeof(struct control_params_t)*apt->n);
}

int write_to_file(struct control_params_t *params, size_t size, double period, const char *file)
{
	int fd;
	struct auto_params_t *apt;

	fd = open(file, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	if(fd < 0) {
		perror("write_to_file");
		return -1;
	}

	if(ftruncate(fd, sizeof(struct auto_params_t) + sizeof(struct control_params_t)*size)) {
		perror("write_to_file: ftruncate");
		close(fd);
		return -1;
	}

	apt = (struct auto_params_t *)mmap(NULL,
			sizeof(struct auto_params_t) + sizeof(struct control_params_t)*size,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(apt == MAP_FAILED) {
		perror("write_to_file:mmap");
		close(fd);
		return -1;
	}

	memcpy(&apt->magic, ENCODER_FILE_MAGIC, sizeof(ENCODER_FILE_MAGIC));

	apt->period = period;
	apt->n = size;
	apt->params = NULL;

	memcpy(&apt[1], params, sizeof(struct control_params_t)*size);

	munmap((void *)apt, sizeof(struct auto_params_t) + sizeof(struct control_params_t)*size);

	close(fd);

	return 0;
}
