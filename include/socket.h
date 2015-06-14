#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/ioctl.h>

#define NAME_LEN 100
#define BORAD_SIZE 200
#define WIN -2
#define LOOSE -3

#define true 1
#define false 0

struct message{
    int x;
    int y;
    char sign;
};

struct player{
    int socket;
    char name[NAME_LEN];
    char sign;
};


