#ifndef SRV_H
#define SRV_H

#define _DEFAULT_SOURCE
#include <poll.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>



#define SERVER_PORT 5555
#define BUFLEN 512
#define N_CLIENTS 200
#define TEMPFILENAME ".tmp.txt"


void writeToClient (int fd, char *buf);
int readFromClient (int fd, char *buf);
void resend(int fd);

#endif
