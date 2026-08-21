#ifndef ARGS_H
#define ARGS_H

typedef struct{
	char *device_id;
	char *device_secret;
	char *product_id;
	int daemon;
} arguments_t;

void parse_args(int argc, char **argv, arguments_t *args);

#endif
