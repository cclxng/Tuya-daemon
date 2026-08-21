#include "signals.h"

volatile sig_atomic_t keep_running = 1;

static void handle_signal(int sig){
	keep_running = 0;
}

void setup_signal_handler(){
	signal(SIGTERM, handle_signal);
	signal(SIGINT, handle_signal);
}
