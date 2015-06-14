#include "socket.h"
int SocketFD;
struct message msg;
char sign;
fd_set readset;
int size=30;
char board[BORAD_SIZE][BORAD_SIZE];

void printBoard(){
    for(int i=1; i<=size; i++){
        for(int j=1; j<size; j++){
            printf("%c", board[i][j]);
        }
        printf("\n");
    }
}

void yourTurn(){
    while(true){
        printBoard();
        printf("Give x, y: ");
        scanf("%d %d", &(msg.x), &(msg.y));

        while(board[msg.x][msg.y]!='#'){
            printf("Wrong coords, try again\n");
            scanf("%d %d", &(msg.x), &(msg.y));
        }

        board[msg.x][msg.y]=sign;
        msg.sign=sign;

        printBoard();
        char prompt;
        printf("Do you confirm your choice? [y/n]\n");
        scanf(" %c", &prompt);
        if(prompt=='y'){
            system("clear");
            printf("Enemy's turn\n");
            printBoard();
            break;
        }
        else{
            system("clear");
            board[msg.x][msg.y]='#';
        }
    }

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

    for(int i=1; i<=size; i++)
        for(int j=1; j<=size; j++) board[i][j]='#';

    printf("Give me your sign\n");
    scanf("%c", &(msg.sign));
    sign=msg.sign;

    char buff[100];
    struct message ms;
    printf("Waiting for another player to log in\n");
    system("clear");

    FD_SET(SocketFD, &readset);
    while(true){
        if(select(SocketFD+1, &readset, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        read(SocketFD, buff, 100);
        ms=*((struct message*)&buff);
        if(ms.x!=-1){
            board[ms.x][ms.y]=ms.sign;
            system("clear");
        }
        printf("Your turn\n");
        printf("%d %d: %c", ms.x, ms.y, ms.sign);
        yourTurn();

        FD_SET(SocketFD, &readset);
    }
    exit(EXIT_SUCCESS);
}
