/* Client code in C */

#include "socket.h"
int SocketFD;
struct message msg;
fd_set readset;

void yourTurn(){
    printf("Give x, y: ");
    scanf("%d %d", &(msg.x), &(msg.y));

    if(write(SocketFD, (char*) &msg, sizeof(msg))<0){
        perror("sending");
        exit(1);
    }
}

void connectRemote(int port, char* ip){
    struct sockaddr_in stSockAddr;
    int Res;
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == SocketFD){
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(stSockAddr));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    Res = inet_pton(AF_INET, ip, &stSockAddr.sin_addr);
    if (0 > Res){
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    else if (0 == Res){
        perror("char string (second parameter does not contain valid ipaddress)");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))){
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

}

void connectLocal(char* addr){
    struct sockaddr_un stSockAddr;
    SocketFD = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == SocketFD){
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(stSockAddr));
    stSockAddr.sun_family = AF_UNIX;
    strcpy(stSockAddr.sun_path,"./serv");


    if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))){
        perror("connection failed, no access to the server");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    printf("logged in to the server\n");

}

void cleanUp(){
    msg.x=-1;
    if(write(SocketFD, (char*) &msg, sizeof(msg))<0){
        perror("sending");
        exit(1);
    }
    (void) shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}

void finish(int sig){
    exit(0);
}

int main(int argc, char** argv){
    atexit(cleanUp);
    signal(SIGINT, finish);
    FD_ZERO(&readset);

    if(strcmp(argv[2], "remote")==0 ){
        connectRemote(atoi(argv[3]), argv[4]);
    }
    else{
        connectLocal(argv[3]);
    }

    printf("Give me your sign\n");
    scanf("%c", &(msg.sign));

    char buff[100];
    struct message ms;
    printf("Waiting for another player to log in\n");
    FD_SET(SocketFD, &readset);
    while(true){
        if(select(SocketFD+1, &readset, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        printf("im here\n");
        read(SocketFD, buff, 100);
        ms=*((struct message*)&buff);
        if(ms.x!=-1){
            printf("%d %d: %c\n", ms.x, ms.y, ms.sign);
        }
        printf("Your turn\n");
        yourTurn();

        FD_SET(SocketFD, &readset);
    }
    exit(EXIT_SUCCESS);
}
