#include "socket.h"
char* ADDR="./serv";

void connectToGame(int isLocal){
    //client <clients_name> sign remote <port> <ip>
    //client <clients_name> local <addr>

    char PORT[100];
    char ip[100];

    system("clear");

    if(isLocal==true){
        execl("./client", "client","local", ADDR, NULL);
    }
    else{
        printf("Give me port: ");
        scanf("%s", PORT);
        printf("Give me ip: ");
        scanf("%s", ip);

        execl("./client", "client", "remote", PORT, ip, NULL);
    }

}

void newGame(){
    pid_t serverPid;
    char PORT[100];
    printf("Give me port: ");
    scanf("%s", PORT);
    serverPid=fork();
    if(serverPid<0){
        perror("fork");
        exit(1);
    }
    else if(serverPid==0){
        execl("./server", "server", PORT, ADDR, NULL);
    }
    else{
        printf("...\n");
        connectToGame(true);
    }

}



int main(){
    int get;
    while(true){
        system("clear");
        printf("Welcome to the Tic-tac-toe game!\n");
        printf("Choose\n");
        printf("1 - to start new game\n");
        printf("2 - to connect to the game\n");
        printf("3 - to exit\n\n");
        scanf("%d", &get);
        switch(get){
            case 1:
                newGame();
                break;
            case 2:
                connectToGame(false);
                break;
            case 3:
                exit(0);
                break;
            default:
                printf("Wrong command try again\n");

        }
    }

    exit(0);
}
