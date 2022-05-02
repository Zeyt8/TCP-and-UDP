#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void subscribe(char topic[], int sf);
void unsubscribe(char topic[]);

char idClient[50];
int sockfd, n;
struct sockaddr_in serv_addr;
char buffer[BUFFER_LEN];
fd_set read_fds, tmp_fds;

const int BUFFER_LEN = 100;

void main(int argc, char *argv[])
{
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    strcpy(idClient, argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    inet_aton(argv[2], &serv_addr.sin_addr);

    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    char command[100];
    scanf("%[^\n]s", command);
    char *tok;
    while(1){
        tmp_fds = read_fds;

        select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);

        if(FD_ISSET(STDIN_FILENO, &tmp_fds)){
            tok = strtok(command, " ");
            if(strcmp(tok, "subscribe") == 0){
                tok = strtok(NULL, " ");
                char* topic = strtok(NULL, " ");
                tok = strtok(NULL, " ");
                subscribe(topic, sf);
            }
            else if(strcmp(tok, "unsubscribe") == 0){
                tok = strtok(NULL, " ");
                unsubscribe(tok);
            }
            else if(strcmp(tok, "exit") == 0){
                close(sockfd);
                return 0;
            }
        }
        if(FD_ISSET(sockfd, &tmp_fds)){
            memset(buffer, 0, BUFFER_LEN);
            n = recv(sockfd, buffer, BUFFER_LEN, 0);
        }
    }
}

void subscribe(char topic[], int sf){

}

void unsubscribe(char topic[]){

}