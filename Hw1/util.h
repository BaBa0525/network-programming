#ifndef UTIL_H
#define UTIL_H

#include "global.h"

int listen_port(int serv_port);
int handle_connection(int listenfd);
void handle_disconnection(int i, fd_set& allset);

#endif
