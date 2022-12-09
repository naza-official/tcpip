
#define _DEFAULT_SOURCE
#include <poll.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>



#define SERVER_PORT 5555
#define BUFLEN 512
#define N_CLIENTS 200
#define TEMPFILENAME ".tmp.txt"


void writeToClient (int fd, char *buf);
int readFromClient (int fd, char *buf);
void resend(int fd);

int main(){

    int i, err, opt=1;
    int sock, new_sock;
    struct sockaddr_in addr;
    struct sockaddr_in client;
    char buf[BUFLEN];
    char inp[BUFLEN];
    socklen_t size;
    FILE * fout;
    fout = fopen(TEMPFILENAME, "w");


    sock = socket (PF_INET, SOCK_STREAM, 0);
    if ( sock < 0 ) {
        perror("Server: cannot create socket");
        exit (EXIT_FAILURE);
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    addr.sin_family = PF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if ( err<0 ) {
        perror("Server: cannot bind socket\n");
        exit(EXIT_FAILURE);
    }



    err = listen(sock, 3);
    if ( err<0 ){
        perror ("Server: listen queue failure");
        exit(EXIT_FAILURE);
    }


    pollfd act_set[N_CLIENTS];
    act_set[0].fd = sock;
    act_set[0].events = POLLIN;
    act_set[0].revents = 0;
    int num_set = 1;
    



    while(1) {
        int ret = poll(act_set, num_set, -1);
        if ( ret<0 ){
            perror("Server: poll failure");
            exit(EXIT_FAILURE);
        }
        
        if ( ret>0 ){
            for (i=0; i<num_set; i++){
                if (act_set[i].revents & POLLIN){
                    printf("Get POLLIN at fd %d\n", act_set[i].fd);
                    act_set[i].revents = 0;
                    if ( i==0 ){
                        size = sizeof(client);
                        new_sock = accept(act_set[i].fd, (struct sockaddr*)&client, &size);
                        printf("new client at port %u\n", ntohs(client.sin_port));
                        if ( num_set<N_CLIENTS){
                            act_set[num_set].fd = new_sock;
                            act_set[num_set].events = POLLIN;
                            act_set[num_set].revents = 0;
                            num_set++;
                            fclose(fout);
                            resend(new_sock);
                            FILE * fout;
                            fout = fopen(TEMPFILENAME, "a");
                            
                        }
                        else{
                            printf("No more sockets for clients");
                            close(new_sock);
                        }
                    } else{
                        err = readFromClient(act_set[i].fd, inp);
                        char num[50];
                        sprintf(num, "[%d]",act_set[i].fd );
                        strcpy(buf, num);
                        strcat(buf, inp);

                        fprintf(fout, "%s", buf);
                        fprintf(fout, "\n");
                        printf("[%s] \n", buf);
                        if ( err<0 || strstr(buf, "stop")){
                            printf("get stop\n");
                            close(act_set[i].fd);
                            if (i<num_set-1){
                                act_set[i] = act_set[num_set-1];
                                num_set --;
                                i--;
                            }
                        }
                        else {
                            for (int k=1; k<num_set; k++){
                                if (act_set[k].fd != act_set[i].fd) 
                                {
                                    writeToClient(act_set[k].fd, buf);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}


void resend(int fd){
    bool start; 
    FILE * inp;
    inp = fopen(TEMPFILENAME, "r");
    
    if (inp!=NULL)start = true;
    else start = false;
    char msg[BUFLEN];
    while (start){
        int t = fscanf(inp, "%s", msg);
        if (feof(inp)) break;
        writeToClient(fd, msg);
    }
    fclose(inp);
}



int readFromClient (int fd, char *buf) {
 
    int nbytes;

    nbytes = read(fd, buf, BUFLEN);
    if ( nbytes < 0 ) {
        perror("Read error\n");
        return -1;
    }else if (nbytes == 0) {
        printf("Client no message\n");   
        return -1;
    }
    else {
        printf("Server got a message %s\n", buf);
        return 0;
    }
}


void writeToClient (int fd, char *buf)
{
    int nbytes;
    // unsigned char *s;

    // for (s=(unsigned char*)buf; *s; s++) *s = toupper(*s);
    nbytes = write(fd, buf, strlen(buf) + 1);
    printf("Write back: %s\nnbytes=%d\n", buf, nbytes);

    if ( nbytes < 0 ){
        perror("Server: write failure");
    }
}