#include <stdio.h>
#include <argp.h>
#include "args.h"

static struct argp_option options[] = {
	{"device-id", 'd', "ID", 0, "Tuya device ID"},
	{"device-secret", 's', "SECRET", 0, "Tuya device secret"},
	{"product-id", 'i', "PRODUCT", 0, "Tuya device product ID"},
	{"daemon", 'D', 0, 0, "Background daemon"},
	{0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state){

	arguments_t *args = state->input;

	switch(key){
		case 'd':
			args->device_id = arg;
			break;
		case 's':
			args->device_secret = arg;
			break;
		case 'i':
			args->product_id = arg;
			break;
		case 'D':
			args->daemon = 1;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}


static struct argp argp = {options, parse_opt, 0, 0};

void parse_args(int argc, char **argv, arguments_t *args){
	argp_parse(&argp, argc, argv, 0, 0, args);
}

