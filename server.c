#include "socket.h"

int INETSocketFD;
int UNSocketFD;
struct player playerOne;
struct player playerTwo;
fd_set readset;
struct message msg;
int PORT;
char board[BORAD_SIZE][BORAD_SIZE];
int sizex=BORAD_SIZE;
int sizey=BORAD_SIZE;
char ADDR[100];

int checkOneSide(int changex, int changey, int count, int newx, int newy){
    while(board[newx][newy]==board[msg.x][msg.y]){
        newx+=changex;
        newy+=changey;
        count++;
    }
    return count;
}

short int checkBoard(){
    int count=1;

    count=checkOneSide(-1, 0, 1, msg.x-1, msg.y);
    count=checkOneSide(1, 0, count, msg.x+1, msg.y);
    if(count>=5) return true;

    count=checkOneSide(0, -1, 1, msg.x, msg.y-1);
    count=checkOneSide(0, 1, count, msg.x, msg.y+1);
    if(count>=5) return true;

    count=checkOneSide(-1, -1, 1, msg.x-1, msg.y-1);
    count=checkOneSide(1, 1, count, msg.x+1, msg.y+1);
    if(count>=5) return true;

    count=checkOneSide(+1, -1, 1, msg.x+1, msg.y-1);
    count=checkOneSide(-1, +1, count, msg.x-1, msg.y+1);
    if(count>=5) return true;

    return false;
}

void setUpRemote(){
    struct sockaddr_in stSockAddr;
    INETSocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == INETSocketFD){
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    memset(&stSockAddr, 0, sizeof(stSockAddr));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(PORT);
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(INETSocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr))){
        perror("error bind failed");
        close(INETSocketFD);
        exit(EXIT_FAILURE);
    }
    if(-1 == listen(INETSocketFD, 10)){
        perror("error listen failed");
        close(INETSocketFD);
        exit(EXIT_FAILURE);
    }
}

void setUpLocal(){
    struct sockaddr_un stSockAddr;
    UNSocketFD = socket(AF_UNIX, SOCK_STREAM, 0);
    if(-1 == UNSocketFD){
        perror("cannot create unix socket");
        exit(EXIT_FAILURE);
    }
    memset(&stSockAddr, 0, sizeof(stSockAddr));
    stSockAddr.sun_family = AF_UNIX;
    strcpy(stSockAddr.sun_path, ADDR);
    if(-1 == bind(UNSocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr))){
        perror("error bind failed");
        close(UNSocketFD);
        exit(EXIT_FAILURE);
    }
    if(-1 == listen(UNSocketFD, 10)){
        perror("error listen failed");
        close(UNSocketFD);
        exit(EXIT_FAILURE);
    }
}

void addNewClient(int SocketFD, int num){
    if(num==1){
        playerOne.socket = accept(SocketFD, NULL, NULL);
        playerOne.sign=msg.sign;
        if(0 > playerOne.socket){
            perror("error accept failed");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }
        //Set the size of playerOne's terminal
        read(playerOne.socket, (char*)&msg, sizeof(msg));
        strcpy(playerOne.name,msg.name);
        sizex=msg.x;
        sizey=msg.y/2;
        FD_SET(playerOne.socket, &readset);
    }
    else{
        playerTwo.socket = accept(SocketFD, NULL, NULL);
        if(0 > playerTwo.socket){
            perror("error accept failed");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }
        //Set the size of playerTwo's terminal
        read(playerTwo.socket, (char*)&msg, sizeof(msg));
        strcpy(playerTwo.name,msg.name);
        if(msg.x<sizex) sizex=msg.x;
        if(msg.y/2<sizey) sizey=msg.y/2;
        sizex-=3;
        msg.x=sizex;
        msg.y=sizey;
        write(playerOne.socket, (char*)&msg, sizeof(msg) );
        write(playerTwo.socket, (char*)&msg, sizeof(msg) );
        FD_SET(playerTwo.socket, &readset);
    }
}

void addClients(){
    if (select(INETSocketFD+5, &readset, NULL, NULL, NULL) < 0) {
        perror("select");
        exit(1);
    }
    if(FD_ISSET(INETSocketFD, &readset)) addNewClient(INETSocketFD, 1);
    if(FD_ISSET(UNSocketFD, &readset)) addNewClient(UNSocketFD, 1);

    FD_SET(INETSocketFD, &readset);
    FD_SET(UNSocketFD, &readset);
    if (select(INETSocketFD+5, &readset, NULL, NULL, NULL) < 0) {
        perror("select");
        exit(1);
    }
    if(FD_ISSET(INETSocketFD, &readset)) addNewClient(INETSocketFD, 2);
    if(FD_ISSET(UNSocketFD, &readset)) addNewClient(UNSocketFD, 2);

    FD_SET(INETSocketFD, &readset);
    FD_SET(UNSocketFD, &readset);
}

void sendMessage(int socket){
    if(write(socket, (void*)&msg, sizeof(msg))<0){
        perror("writing");
        exit(1);
    }
}

void handleMessage(int i){
    if(read(i, (char*)&msg, sizeof(msg))<=0) return;
    if(msg.x<=0 || msg.state==LEFT){
        msg.state=LEFT;
        if(playerOne.sign!=msg.sign) {
            sendMessage(playerOne.socket);
        }
        else {
            sendMessage(playerTwo.socket);
        }
        exit(0);
    }
    board[msg.x][msg.y]=msg.sign;
    if(checkBoard()==true){
        if(msg.sign==playerOne.sign) {
            FILE* fd=fopen("results.txt", "a");
            fprintf(fd, "Winner: %s\tLooser: %s\n", playerOne.name, playerTwo.name);
            fclose(fd);
            msg.state=WIN;
            sendMessage(playerOne.socket);
            msg.state=LOOSE;
            sendMessage(playerTwo.socket);
        }
        else {
            FILE* fd=fopen("results.txt", "a");
            fprintf(fd, "Winner: %s\tLooser: %s\n", playerTwo.name, playerOne.name);
            fclose(fd);

            msg.state=WIN;
            sendMessage(playerTwo.socket);
            msg.state=LOOSE;
            sendMessage(playerOne.socket);
        }
    }
    else{
        if(i==playerOne.socket) {
            sendMessage(playerTwo.socket);
        }
        else{
            sendMessage(playerOne.socket);
        }
    }
}

void cleanUp(){
    printf("\nShutting server down...\n");
    unlink(ADDR);
    shutdown(playerOne.socket, SHUT_RDWR);
    shutdown(playerTwo.socket, SHUT_RDWR);

    close(playerTwo.socket);
    close(playerOne.socket);
    close(INETSocketFD);
    close(UNSocketFD);
    exit(1);
}

void finish(int sig){
    exit(0);
}

int main(int argc, char** argv){
    atexit(cleanUp);
    signal(SIGINT, finish);

    if(argc<3){
        printf("Wrong number of arguments\n");
        exit(1);
    }
    PORT=atoi(argv[1]);
    strcpy(ADDR,argv[2]);

    setUpLocal();
    setUpRemote();

    FD_ZERO(&readset);
    FD_SET(INETSocketFD, &readset);
    FD_SET(UNSocketFD, &readset);

    for(int i=1; i<=sizex; i++)
        for(int j=1; j<=sizey; j++) board[i][j]='_';

    addClients();

    srand(time(NULL));
    int start=rand()%2;
    msg.x=-1;

    if(start==1){
        sendMessage(playerOne.socket);
    }
    else sendMessage(playerTwo.socket);

    msg.x=0;
    msg.y=0;

    FD_SET(playerOne.socket, &readset);
    FD_SET(playerTwo.socket, &readset);

    while(true){
        if(select(INETSocketFD+5, &readset, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        if(FD_ISSET(playerOne.socket, &readset)) handleMessage(playerOne.socket);
        else if(FD_ISSET(playerTwo.socket, &readset)) handleMessage(playerTwo.socket);
        FD_SET(playerOne.socket, &readset);
        FD_SET(playerTwo.socket, &readset);
    }
    exit(0);
}
