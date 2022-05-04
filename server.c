#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "queue.h"
#include "ue_vector.h"

#define BUFFER_LEN 1551
#define MAX_CLIENTS 10000
#define FD_START 4

typedef struct client{
    int sock;
    char ID[10];
    ue_vector *topics;
    ue_vector *sfs;
    queue messagesToReceive;
} client;

int sockfd, newsockfd, portno, dest, udpfd;
char buffer[BUFFER_LEN];
struct sockaddr_in serv_addr, cli_addr;
socklen_t clilen;

fd_set read_fds;
fd_set tmp_fds;
int fdmax;

ue_vector* clients;

int ret;

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int n;
    clients = ue_vector_start(sizeof(client), sizeof(client));

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        fprintf(stderr, "No socket available.\n");
    }

    portno = atoi(argv[1]);

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    if(ret < 0){
        fprintf(stderr, "Cannot bind socket.\n");
    }

    ret = listen(sockfd, MAX_CLIENTS);
    if(ret < 0){
        fprintf(stderr, "Cannot listen to socket.\n");
    }

    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpfd < 0){
        fprintf(stderr, "No scoket available.\n");
    }
    ret = bind(udpfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0){
        fprintf(stderr, "Cannot bind socket.\n");
    }

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(udpfd, &read_fds);
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;
    if(udpfd > fdmax){
        fdmax = udpfd;
    }

    int incomingId = 0;

    while(1){
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        if(ret < 0){
            fprintf(stderr, "Cannot listen to socket.\n");
        }

        for (int i = 0; i <= fdmax; i++){
            if(FD_ISSET(i, &tmp_fds)){
                if(i==sockfd){
                    clilen = sizeof(cli_addr);
                    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                    if(newsockfd < 0){
                        fprintf(stderr, "Cannot accept connection.\n");
                    }
                    FD_SET(newsockfd, &read_fds);
                    if (newsockfd > fdmax)
                    {
                        fdmax = newsockfd;
                    }
                    incomingId = 1;
                }
                else if(i == udpfd){
                    memset(buffer, 0, BUFFER_LEN);
                    struct sockaddr addr;
                    socklen_t addrLen;
                    n = recvfrom(i, buffer, BUFFER_LEN, 0, &addr, &addrLen);
                    if(n < 0){
                        fprintf(stderr, "Cannot receive from socket.\n");
                    }
                    char topic[51];
                    memcpy(topic, buffer, 50);
                    topic[50] = '\0';
                    for (int k = 0; k < clients->length; k++)
                    {
                        client *c = ((client *)ue_vector_get_in(clients, k));
                        for (int j = 0; j < c->topics->length; j++)
                        {
                            printf("Server: %s %s\n", *(char **)ue_vector_get_in(c->topics, j), topic);
                            if (strcmp(*(char **)ue_vector_get_in(c->topics, j), topic) == 0)
                            {
                                send(c->sock, buffer + 50, sizeof(1501), 0);
                            }
                        }
                    }
                }
                else{
                    memset(buffer, 0, BUFFER_LEN);
                    struct sockaddr addr;
                    socklen_t addrLen;
                    n = recvfrom(i, buffer, BUFFER_LEN, 0, &addr, &addrLen);
                    if(n < 0){
                        fprintf(stderr, "Cannot receive from socket.\n");
                    }
                    else if(n==0){
                        for (int j = 0; j < clients->length; j++)
                        {
                            if(((client*)ue_vector_get_in(clients, j))->sock == i){
                                ue_vector_delete_in(clients, j);
                            }
                        }
                    }
                    else{
                        if(incomingId){
                            int delet = 0;
                            for (int j = 0; j < clients->length; j++)
                            {
                                if (strcmp(((client *)ue_vector_get_in(clients, j))->ID, buffer) == 0)
                                {
                                    close(i);
                                    delet = 1;
                                }
                            }
                            if(!delet){
                                client c;
                                c.sock = i;
                                c.topics = ue_vector_start(sizeof(char*), sizeof(char*));
                                c.sfs = ue_vector_start(sizeof(int), sizeof(int));
                                c.messagesToReceive = queue_create();
                                memcpy(c.ID, buffer, 10);
                                ue_vector_add_back(clients, &c);
                                printf("New client %s connected from %s:%d\n", buffer, inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port);
                            }
                            incomingId = 0;
                        }
                        else{
                            char action;
                            memcpy(&action, buffer, 1);
                            if(action == 's'){
                                char *topic = (char *)malloc(51);
                                memcpy(topic, buffer + 1, 51);
                                int sf;
                                memcpy(&sf, buffer + 52, sizeof(sf));
                                for (int j = 0; j < clients->length; j++){
                                    if(((client*)ue_vector_get_in(clients, j))->sock == i){
                                        ue_vector_add_front(((client *)ue_vector_get_in(clients, j))->topics, &topic);
                                        ue_vector_add_front(((client *)ue_vector_get_in(clients, j))->sfs, &sf);
                                    }
                                }
                            }
                            else if(action == 'u'){
                                char topic[51];
                                memcpy(topic, buffer + 1, 51);
                                for (int j = 0; j < clients->length; j++)
                                {
                                    client *c = ((client *)ue_vector_get_in(clients, j));
                                    if (c->sock == i)
                                    {
                                        for (int k = 0; k < c->topics->length; k++)
                                        {
                                            if (strcmp(*(char**)ue_vector_get_in(c->topics, k), topic) == 0){
                                                ue_vector_delete_in(c->topics, k);
                                                ue_vector_delete_in(c->sfs, k);
                                            }
                                        }
                                    }
                                }
                            }
                            else{
                                fprintf(stderr, "Invalid message.");
                            }
                        }
                    }
                }
            }
            else if (FD_ISSET(STDIN_FILENO, &tmp_fds))
            {
                char command[5];
                fgets(command, 5, stdin);
                if(strcmp(command, "exit") == 0){
                    close(sockfd);
                    return 1;
                }
                else{
                    fprintf(stderr, "Invalid command.\n");
                }
            }
        }
    }
}