#include "socket.h"
int SocketFD;
struct message msg;
char sign;
fd_set readset;
int sizex=30;
int sizey=30;
char board[BORAD_SIZE][BORAD_SIZE];
char win[BORAD_SIZE][BORAD_SIZE];
char* ADDR="./serv";

void printBoard(){
    for(int i=1; i<=sizex; i++){
        for(int j=1; j<sizey; j++){
            if(win[i][j]==0) printf("%c ", board[i][j]);
            else printf(KRED "%c " RESET, board[i][j]);
        }
        printf("\n");
    }
}

int checkOneSide(int changex, int changey, int count, int newx, int newy, int flag){
    while(board[newx][newy]==board[msg.x][msg.y]){
        if(flag==1){
            win[newx][newy]=1;
        }
        newx+=changex;
        newy+=changey;
        count++;
    }
    return count;
}

void checkBoard(){
    int count=1;

    win[msg.x][msg.y]=1;
    count=checkOneSide(-1, 0, 1, msg.x-1, msg.y, 0);
    count=checkOneSide(1, 0, count, msg.x+1, msg.y, 0);
    if(count>=5) {
        checkOneSide(-1, 0, 1, msg.x-1, msg.y, 1);
        checkOneSide(1, 0, count, msg.x+1, msg.y, 1);
        return;
    }

    count=checkOneSide(0, -1, 1, msg.x, msg.y-1, 0);
    count=checkOneSide(0, 1, count, msg.x, msg.y+1, 0);
    if(count>=5){
        checkOneSide(0, -1, 1, msg.x, msg.y-1, 1);
        checkOneSide(0, 1, count, msg.x, msg.y+1, 1);
        return;
    }

    count=checkOneSide(-1, -1, 1, msg.x-1, msg.y-1, 0);
    count=checkOneSide(1, 1, count, msg.x+1, msg.y+1, 0);
    if(count>=5){
        checkOneSide(-1, -1, 1, msg.x-1, msg.y-1, 1);
        checkOneSide(1, 1, count, msg.x+1, msg.y+1, 1);
        return;
    }

    count=checkOneSide(+1, -1, 1, msg.x+1, msg.y-1, 0);
    count=checkOneSide(-1, +1, count, msg.x-1, msg.y+1, 0);
    if(count>=5){
        checkOneSide(+1, -1, 1, msg.x+1, msg.y-1, 1);
        checkOneSide(-1, +1, count, msg.x-1, msg.y+1, 1);
        return;
    }

}

void yourTurn(){
    while(true){
        printBoard();
        printf("Give x y: ");
        scanf("%d %d", &(msg.x), &(msg.y));

        while(board[msg.x][msg.y]!='_' || msg.x<1 || msg.x>sizex || msg.y<1 || msg.y>sizey){
            printf("You cannot put it there\n");
            scanf("%d", &(msg.x));
            scanf("%d", &(msg.y));
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
            board[msg.x][msg.y]='_';
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
    write(SocketFD, (char*) &msg, sizeof(msg));
    (void) shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    unlink(ADDR);
}

void finish(int sig){
    exit(0);
}

void endGame(struct message ms){
    msg=ms;
    if(ms.state==WIN){
        system("clear");
        printf("!!!!!!!!YOU WON!!!!!!!!!!\n");
        checkBoard();
        printBoard();
        printf("To return to main menu, press any key and enter\n");
        scanf(" %c", &sign);
        cleanUp();
        execl("./tictactoe", "tictactoe", NULL);

    }
    if(ms.state==LOOSE){
        system("clear");
        printf("!!!!!!!!YOU LOST!!!!!!!!!!\n");
        board[ms.x][ms.y]=ms.sign;
        checkBoard();
        printBoard();
        printf("To return to main menu, press any key and enter\n");
        scanf(" %c", &sign);
        cleanUp();
        execl("./tictactoe", "tictactoe", NULL);
    }
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

    for(int i=1; i<=sizex; i++)
        for(int j=1; j<=sizey; j++) board[i][j]='_';

    printf("Give me your sign\n");
    scanf("%c", &(msg.sign));
    sign=msg.sign;

    char buff[100];
    struct message ms;
    system("clear");

    FD_SET(SocketFD, &readset);
    while(true){
        if(select(SocketFD+2, &readset, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        read(SocketFD, buff, 100);
        ms=*((struct message*)&buff);
        if(ms.state!=0) endGame(ms);
        if(ms.x!=-1){
            board[ms.x][ms.y]=ms.sign;
            system("clear");
            printf("%d %d: %c\n", ms.x, ms.y, ms.sign);
        }
        if(ms.x==-1 || ms.x>0){
            printf("Your turn\n");
            yourTurn();
        }

        FD_SET(SocketFD, &readset);
    }
    exit(EXIT_SUCCESS);
}
