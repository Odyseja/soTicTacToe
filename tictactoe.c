#include "socket.h"

char* ADDR="./serv";
char name[100];

void connectToGame(int isLocal){
    //client <clients_name> sign remote <port> <ip>
    //client <clients_name> local <addr>

    char PORT[100];
    char ip[100];

    system("clear");

    if(isLocal==true){
        printf("What's your name?: ");
        scanf("%s", name);
        execl("./client", "client", "local", ADDR, name, "o", NULL);
    }
    else{
        printf("What's your name?: ");
        scanf("%s", name);
        printf("Give me port: ");
        scanf("%s", PORT);
        printf("Give me ip: ");
        scanf("%s", ip);

        execl("./client", "client", "remote", PORT, ip, name, "x",NULL);
    }
}

void newGame(){
    pid_t serverPid;
    unlink(ADDR);
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
        connectToGame(true);
    }

}

void showResults(){
    system("clear");
    FILE* fd=fopen("results.txt", "r");
    if(fd==NULL){
        perror("opening file");
        return;
    }
    printf("Results:\n");
    char ch;
    while((ch=fgetc(fd))!= EOF) printf("%c",ch);
    fclose(fd);
    printf("\n\n\nTo return to main menu, press any key and enter\n");
    scanf(" %c", &ch);
}

void newLocalGame(){
    char name1[100], name2[100];
    char PORT[100];

    printf("Give me port: ");
    scanf("%s", PORT);

    pid_t serverPid;
    serverPid=fork();
    if(serverPid<0){
        perror("fork");
        exit(1);
    }
    else if(serverPid==0){
        execl("./server", "server", PORT, ADDR, NULL);
    }
    printf("Please wait, server is initializing\n");
    sleep(1);

    printf("Player one, what's your name?: ");
    scanf("%s", name1);

    printf("Player two, what's your name?: ");
    scanf("%s", name2);

    pid_t pid=fork();
    char buff[100];

    if(pid<0){
        perror("fork");
        exit(1);
    }
    else if(pid==0){
        sprintf(buff, "%ud", getpid());
        execl("./client", "client", "local", ADDR, name1, "o", buff, NULL);
    }
    else{
        sprintf(buff, "%ud", pid);
        execl("./client", "client", "local", ADDR, name2, "x", buff, NULL);
    }
}

int main(){
    int get;
    while(true){
        system("clear");
        printf(KGRN "Welcome to the soTicTacToe game!\n" RESET);
        printf("Choose\n");
        printf("1 - to start new online game\n");
        printf("2 - to connect to the online game\n");
        printf("3 - to start new local game\n");
        printf("4 - to show the results\n");
        printf("5 - to exit\n\n");
        scanf("%d", &get);
        switch(get){
            case 1:
                newGame();
                break;
            case 2:
                connectToGame(false);
                break;
            case 3:
                newLocalGame();
                break;
            case 4:
                showResults();
                break;
            case 5:
                exit(0);
                break;
            default:
                printf("Wrong command try again\n");
                char ch;
                while ((ch = getchar()) != '\n' && ch != EOF);

        }
    }

    exit(0);
}
