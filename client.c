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
short int flag;
char name[100];
char name2[100];
pid_t pid;

void printBoard(){
    printf("     ");
    for(int i=1; i<=sizey; i++){
        if(i<9)printf("%d  ", i);
        else printf("%d ", i);
    }
    printf("\n");
    for(int i=1; i<=sizex; i++){
        if(i<10) printf("%d    ", i);
        else printf("%d   ", i);
        for(int j=1; j<=sizey; j++){
            if(win[i][j]==0) printf("%c  ", board[i][j]);
            else printf(KRED "%c  " RESET, board[i][j]);
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
        system("clear");
        printf("%s's turn\n", name);
        printBoard();
        char ch;
        while(true){
            printf("Give y x: ");
            if(scanf("%d %d", &(msg.x), &(msg.y))==2) break;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        system("clear");
        strcpy(msg.name,name);

        while( msg.x<1 || msg.x>sizex || msg.y<1 || msg.y>sizey || board[msg.x][msg.y]!='_'){
            system("clear");
            printf("Choose another place\n");
            printBoard();
            while(true){
                printf("Give y x: ");
                if(scanf("%d %d", &(msg.x), &(msg.y))==2) break;
                while ((ch = getchar()) != '\n' && ch != EOF);
            }
        }
        system("clear");
        board[msg.x][msg.y]=sign;
        msg.sign=sign;
        printf("%s's turn\n", name);
        printBoard();
        char prompt;
        printf("Do you confirm your choice? [y/n]\n");
        scanf(" %c", &prompt);
        system("clear");
        if(prompt=='y'){
            printf("Enemy's turn\n");
            printBoard();
            break;
        }
        else{
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
    printf("Waiting for opponent, to end the game press ctrl+c\n");

}

void cleanUp(){
    (void) shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    unlink(ADDR);
}

void finish(int sig){
    msg.x=-1;
    msg.state=LEFT;
    msg.sign=sign;
    write(SocketFD, (char*)&msg, sizeof(msg));
    read(SocketFD, (char*)&(msg), sizeof(msg));
    exit(0);
}

void endGame(){
    system("clear");
    if(flag==1) kill(SIGINT, pid);
    if(msg.state==LEFT){
        printf("Your opponent left\n");
        exit(0);
    }
    if(msg.state==-10){
        system("clear");
        printf("There is a draw\n");
        printBoard();
        printf("To return to main menu, press any key and enter\n");
        scanf(" %c", &sign);
        cleanUp();
        execl("./tictactoe", "tictactoe", NULL);
    }
    if(msg.state==WIN){
        system("clear");

        FILE* fd=fopen("results.txt", "a");
        fprintf(fd, "Player: %s\tWinner: %s\tLoser: %s\n", name, name, name2);
        fclose(fd);

        printf("!!!!!!!!%s WON!!!!!!!!!!\n", msg.name);
        checkBoard();
        printBoard();
        printf("To return to main menu, press any key and enter\n");
        scanf(" %c", &sign);
        cleanUp();
        execl("./tictactoe", "tictactoe", NULL);

    }
    if(msg.state==LOSE){
        system("clear");

        FILE* fd=fopen("results.txt", "a");
        fprintf(fd, "Player: %s\tWinner: %s\tLoser: %s\n", name, name2, name);
        fclose(fd);

        printf("!!!!!!!!%s WON!!!!!!!!!!\n", msg.name);
        board[msg.x][msg.y]=msg.sign;
        checkBoard();
        printBoard();
        printf("To return to main menu, press any key and enter\n");
        scanf(" %c", &sign);
        cleanUp();
        execl("./tictactoe", "tictactoe", NULL);
    }
    exit(0);
}

int main(int argc, char** argv){
    atexit(cleanUp);
    signal(SIGINT, finish);
    FD_ZERO(&readset);

    if(strcmp(argv[1], "remote")==0 ){
        strcpy(name, argv[4]);
        msg.sign=argv[5][0];
        connectRemote(atoi(argv[2]), argv[3]);
    }
    else{
        strcpy(name, argv[3]);
        msg.sign=argv[4][0];
        connectLocal(argv[2]);
        if(argc==6){
            flag=1;
            pid=atoi(argv[5]);
        }
    }
    sign=msg.sign;
    //Set the board size
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    msg.x=w.ws_row;
    msg.y=w.ws_col;
    write(SocketFD, (char*)&msg, sizeof(msg));
    read(SocketFD, (char*)&msg, sizeof(msg));

    sizex=msg.x;
    sizey=msg.y;

    for(int i=1; i<=sizex; i++)
        for(int j=1; j<=sizey; j++) board[i][j]='_';

    system("clear");
    printf("Waiting for opponent, to end the game press ctrl+c\n");
    printBoard();
    FD_SET(SocketFD, &readset);
    while(true){
        if(select(SocketFD+2, &readset, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        read(SocketFD, (char*)&msg, sizeof(msg));

        if(msg.x>0){
            board[msg.x][msg.y]=msg.sign;
        }
        system("clear");
        if(msg.state!=0) endGame();
        strcpy(name2, msg.name);
        if(msg.x==-1 || msg.x>0){
            printf("Your turn\n");
            yourTurn();
        }

        FD_SET(SocketFD, &readset);
    }
    exit(EXIT_SUCCESS);
}
