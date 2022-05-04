#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

#define BUFFER_LEN 1551
#define ID_LEN 10

void subscribe(char topic[], int sf, int len);
void unsubscribe(char topic[], int len);

char idClient[ID_LEN];
int sockfd;
struct sockaddr_in serv_addr;
char buffer[BUFFER_LEN];
fd_set read_fds, tmp_fds;
int ret;

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int n;

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    strcpy(idClient, argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        fprintf(stderr, "No socket available.\n");
    }
    int flag = 1;
    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
    if(ret < 0){
        fprintf(stderr, "Error disabling Nagle.");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    inet_aton(argv[2], &serv_addr.sin_addr);

    ret = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0){
        fprintf(stderr, "Cannot connect to server.\n");
    }
    memset(buffer, 0, sizeof(BUFFER_LEN));
    memcpy(buffer, idClient, sizeof(idClient));
    buffer[ID_LEN] = '\0';
    ret = send(sockfd, buffer, ID_LEN + 1, 0);
    if(ret < 0){
        fprintf(stderr, "Cannot send message to server.\n");
    }

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    char command[100];
    char *tok;
    while (1)
    {
        tmp_fds = read_fds;

        ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        if(ret < 0){
            fprintf(stderr, "Cannot select fd.\n");
        }

        if(FD_ISSET(STDIN_FILENO, &tmp_fds)){
            fgets(command, 100, stdin);
            tok = strtok(command, " \n");
            if(strcmp(tok, "subscribe") == 0){
                char* topic = strtok(NULL, " ");
                tok = strtok(NULL, " ");
                subscribe(topic, atoi(tok), strlen(topic));
            }
            else if(strcmp(tok, "unsubscribe") == 0){
                tok = strtok(NULL, " ");
                unsubscribe(tok, strlen(tok));
            }
            else if(strcmp(tok, "exit") == 0){
                shutdown(sockfd, SHUT_RDWR);
                close(sockfd);
                exit(0);
            }
            else{
                fprintf(stderr, "Invalid command.\n");
            }
        }
        if(FD_ISSET(sockfd, &tmp_fds)){
            memset(buffer, 0, BUFFER_LEN);
            n = recv(sockfd, buffer, BUFFER_LEN, 0);
            if(n < 0)
            {
                fprintf(stderr, "Cannot receive message.\n");
            }
            else if(n == 0){
                close(sockfd);
                return 1;
            }

            char topic[51];
            memcpy(topic, buffer, 50);
            topic[50] = '\0';

            unsigned int type = 0;
            memcpy(&type, buffer + 50, 1);
            if (type == 0)
            {
                uint8_t sign = 0;
                memcpy(&sign, buffer + 51, 1);
                uint32_t number = 0;
                memcpy(&number, buffer + 52, sizeof(uint32_t));
                int p = ntohl(number);
                if(sign == 1){
                    p *= -1;
                }
                printf("%s - %s - %d\n", topic, "INT", p);
            }
            else if (type == 1)
            {
                uint16_t number = 0;
                memcpy(&number, buffer + 51, sizeof(uint16_t));
                unsigned short p = ntohs(number);
                float res = (float)p / 100;
                printf("%s - %s - %.2f\n", topic, "SHORT_REAL", res);
            }
            else if (type == 2)
            {
                uint8_t sign = 0;
                memcpy(&sign, buffer + 51, 1);
                uint32_t number = 0;
                memcpy(&number, buffer + 52, sizeof(uint32_t));
                uint8_t point = 0;
                memcpy(&point, buffer + 52 + sizeof(uint32_t), sizeof(uint8_t));
                float p = ntohl(number);
                for (int k = 0; k < point; k++){
                    p /= 10;
                }
                if(sign == 1){
                    p *= -1;
                }
                printf("%s - %s - %f\n", topic, "FLOAT", p);
            }
            else if (type == 3)
            {
                char string[1501];
                memcpy(string, buffer + 51, sizeof(string) - 1);
                string[1500] = '\0';
                printf("%s - %s - %s\n", topic, "STRING", string);
            }
            else
            {
                fprintf(stderr, "Invalid message.\n");
            }
        }
    }
}

void subscribe(char topic[], int sf, int len){
    memset(buffer, 0, BUFFER_LEN);
    memcpy(buffer, "s", 1);
    memcpy(buffer + 1, topic, len);
    memcpy(buffer + 52, &sf, sizeof(sf));
    memcpy(buffer + 52 + sizeof(sf), "\0", 1);
    ret = send(sockfd, buffer, 55, 0);
    if(ret < 0){
        fprintf(stderr, "Cannot send message to server.\n");
    }
    else{
        printf("Subscribed to topic.\n");
    }
}

void unsubscribe(char topic[], int len){
    memset(buffer, 0, BUFFER_LEN);
    memcpy(buffer, "u", 1);
    memcpy(buffer + 1, topic, len);
    memcpy(buffer + 52, "\0", 1);
    ret = send(sockfd, buffer, 51, 0);
    if(ret < 0){
        fprintf(stderr, "Cannot send message to server.\n");
    }
    else{
        printf("Unsubscribed to topic.\n");
    }
}