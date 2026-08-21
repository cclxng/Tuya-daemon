#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "fork_daemon.h"
#include <syslog.h>
#include <errno.h>
#include <string.h>

int fork_daemon(){
	pid_t pid = fork();
	if(pid<0) return -1;
	if(pid>0) exit(0);
	if(setsid() == -1) return -1;

	if(freopen("/dev/null", "r", stdin) == NULL) syslog(LOG_ERR, "freopen stdin failed %s", strerror(errno));
	if(freopen("/dev/null", "w", stdout)  == NULL) syslog(LOG_ERR, "freopen stdoud failed %s", strerror(errno));
	if(freopen("/dev/null", "w", stderr) == NULL) syslog(LOG_ERR, "freopen stderr failed %s", strerror(errno));


	return 0;
}
