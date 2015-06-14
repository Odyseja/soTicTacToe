#include "socket.h"

int INETSocketFD;
int UNSocketFD;
struct player playerOne;
struct player playerTwo;
fd_set readset;
struct message msg;
int PORT;
char ADDR[100];
char board[BORAD_SIZE][BORAD_SIZE];
int size=BORAD_SIZE;

short int checkBoard(){
    int count=1;
    int newx=msg.x-1;
    int newy=msg.y;
    while(board[newx][newy]==board[msg.x][msg.y]){
        newx--;
        count++;
    }
    newx=msg.x+1;
    while(board[newx][newy]==board[msg.x][msg.y]){
        newx++;
        count++;
    }
    if(count>=5) return true;

    newx=msg.x;
    newy=msg.y-1;
    count=1;
    while(board[newx][newy]==board[msg.x][msg.y]){
        newy--;
        count++;
    }
    newy=msg.y+1;
    while(board[newx][newy]==board[msg.x][msg.y]){
        newy++;
        count++;
    }
    if(count>=5) return true;

    newx=msg.x-1;
    newy=msg.y-1;
    count=1;
    while(board[newx][newy]==board[msg.x][msg.y]){
        newy--;
        newx--;
        count++;
    }
    newy=msg.y+1;
    newx=msg.x+1;
    while(board[newx][newy]==board[msg.x][msg.y]){
        newy++;
        newx++;
        count++;
    }
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
        FD_SET(playerOne.socket, &readset);
    }
    else{
        playerTwo.socket = accept(SocketFD, NULL, NULL);
        if(0 > playerTwo.socket){
            perror("error accept failed");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }
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
    if(msg.x==-1){
        printf("Client left\n");
        exit(0);
    }
    board[msg.x][msg.y]=msg.sign;
    if(i==playerOne.socket)sendMessage(playerTwo.socket);
    else sendMessage(playerOne.socket);
    /*if(checkBoard()==true){
        if(msg.sign==playerOne.sign) {
            msg.x=WIN;
            sendMessage(playerOne.sign);
            sendMessage(playerOne.sign);
            msg.x=LOOSE;
            sendMessage(playerTwo.sign);
            sendMessage(playerTwo.sign);
        }
        else {
            msg.x=WIN;
            sendMessage(playerTwo.sign);
            sendMessage(playerTwo.sign);
            msg.x=LOOSE;
            sendMessage(playerOne.sign);
            sendMessage(playerOne.sign);
        }
        sleep(3);
        exit(0);
    }*/
}

void cleanUp(){
    printf("Shutting down...\n");
    unlink(ADDR);
    shutdown(playerOne.socket, SHUT_RDWR);
    shutdown(playerTwo.socket, SHUT_RDWR);

    close(playerTwo.socket);
    close(playerOne.socket);
    close(INETSocketFD);
    close(UNSocketFD);
}
void finish(int sig){
    printf("I've got signal\n");
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

    setUpRemote();
    setUpLocal();

    FD_ZERO(&readset);
    FD_SET(INETSocketFD, &readset);
    FD_SET(UNSocketFD, &readset);
    for(int i=1; i<=size; i++)
        for(int j=1; j<=size; j++) board[i][j]='_';

    addClients();

    srand(time(NULL));
    int start=rand()%2;
    msg.x=-1;
    start=0;
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
