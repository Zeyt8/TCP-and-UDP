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
#define MAX_CLIENTS 100
#define FD_START 4

typedef struct client{
    int sock;
    char ID[10];
    ue_vector *topics;
    queue messagesToReceive;
} client;

int sockfd, newsockfd, portno, dest;
char buffer[BUFFER_LEN];
struct sockaddr_in serv_addr, cli_addr;
socklen_t clilen;

fd_set read_fds;
fd_set tmp_fds;
int fdmax;

ue_vector* clients;

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int n;
    clients = ue_vector_start(sizeof(client), sizeof(client));

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    portno = atoi(argv[1]);

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));

    listen(sockfd, MAX_CLIENTS);

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;

    while(1){
        tmp_fds = read_fds;

        select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);

        for (int i = 0; i < fdmax;i++){
            if(FD_ISSET(i, &tmp_fds)){
                if(i==sockfd){
                    clilen = sizeof(cli_addr);
                    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                    FD_SET(newsockfd, &read_fds);
                    n = recv(sockfd, buffer, BUFFER_LEN, 0);
                    if (newsockfd > fdmax)
                    {
                        fdmax = newsockfd;
                    }
                    client c;
                    c.sock = newsockfd;
                    c.topics = ue_vector_start(51, 51);
                    c.messagesToReceive = queue_create();
                    memcpy(c.ID, buffer, 10);
                    ue_vector_add_back(clients, &c);
                    printf("New client %s connected from %s:%d", buffer, inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port);
                }
                else{
                    memset(buffer, 0, BUFFER_LEN);
                    struct sockaddr addr;
                    socklen_t addrLen;
                    n = recvfrom(sockfd, buffer, BUFFER_LEN, 0, &addr, &addrLen);
                    if(n==0){
                        for (int j = 0; j < clients->length; j++)
                        {
                            if(((client*)ue_vector_get_in(clients, j))->sock == i){
                                ue_vector_delete_in(clients, j);
                            }
                        }
                    }
                    else{
                        char topic[51];
                        memcpy(topic, buffer, 50);
                        topic[50] = '\0';
                        for (int k = 0; k < clients->length; k++)
                        {
                            for (int j = 0; j < ((client *)ue_vector_get_in(clients, i))->topics->length; j++)
                            {
                                if (strcmp(ue_vector_get_in(((client *)ue_vector_get_in(clients, k))->topics, j), topic) == 0)
                                {
                                    send(((client *)ue_vector_get_in(clients, k))->sock, buffer + 50, sizeof(1501), 0);
                                }
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
            }
        }
    }
}