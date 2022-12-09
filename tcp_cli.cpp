#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>



#define SERVER_PORT 5555
#define SERVER_NAME "127.0.0.1"
#define BUFLEN 512


void * writeToServer (void *arg);
void * readFromServer (void *arg);

volatile sig_atomic_t flag = 0;


void catch_ctrl_c_and_exit(int sig){
    flag = 1;
}

int sock; 
char * cli_id;


int main(){
    signal(SIGINT, catch_ctrl_c_and_exit);


    int err;
    struct sockaddr_in server_addr;
    struct hostent *hostinfo;
    hostinfo = gethostbyname(SERVER_NAME);
    if ( NULL == hostinfo ) {
        fprintf (stderr, "Unknown host %s. \n", SERVER_NAME);
        exit (EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr = *(struct in_addr*) hostinfo->h_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if ( sock < 0 ) {
        perror ("Client: socket was not created ");
        exit (EXIT_FAILURE);
    }


    sock = socket(PF_INET, SOCK_STREAM, 0);
    if ( sock<0 ){
        perror("Client: socket was not created");
        exit (EXIT_FAILURE);
    }


    err = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if ( err < 0 ) {
        perror("Client: connect failure");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Connection is ready\n");


    pthread_t write_to_server;

    if (pthread_create(&write_to_server, NULL, &writeToServer, NULL) != 0) {
        perror("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t read_from_server;

    if (pthread_create(&read_from_server, NULL, &readFromServer, NULL) != 0) {
        perror("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }




    while(1){
        if (flag) break;
    }
    fprintf(stdout, "The end\n");

    close(sock);
    exit (EXIT_SUCCESS);


    return 0;
}





void * writeToServer(void *arg){

    int nbytes;
    char buf[BUFLEN];

    while(1){
        fprintf(stdout, "");
        while (fgets(buf, BUFLEN, stdin)==nullptr){
            printf("Empty string. Try again!\n\t > ");
        }
        buf[strlen(buf)-1] = 0;

        nbytes = write(sock, buf, strlen(buf)+1);
        if (nbytes < 0)  break;
        if (strstr(buf, "stop")) break;
    }
    catch_ctrl_c_and_exit(2);

}


void * readFromServer (void *arg) {

    int nbytes;
    char buf[BUFLEN];

    while(1){
        nbytes = read(sock, buf, BUFLEN);
        if ( nbytes < 0 ) {
            perror("Read error\n");
            break;
        }else if (nbytes == 0) {
            fprintf(stderr, "Server no message\n");   
            break;
        }
        else {
            printf("\n[server] %s\n", buf);
        }
    }
}