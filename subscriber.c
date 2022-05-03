#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_LEN 1551
#define ID_LEN 10

void subscribe(char topic[], int sf, int len);
void unsubscribe(char topic[], int len);

char idClient[50];
int sockfd;
struct sockaddr_in serv_addr;
char buffer[BUFFER_LEN];
fd_set read_fds, tmp_fds;

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int n;

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    strcpy(idClient, argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    inet_aton(argv[2], &serv_addr.sin_addr);

    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    memset(buffer, 0, sizeof(BUFFER_LEN));
    memcpy(buffer, idClient, sizeof(idClient));
    buffer[ID_LEN] = '\0';
    send(sockfd, buffer, ID_LEN + 1, 0);

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    char command[100];
    char *tok;
    while (1)
    {
        tmp_fds = read_fds;

        select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);

        if(FD_ISSET(STDIN_FILENO, &tmp_fds)){
            fgets(command, 100, stdin);
            tok = strtok(command, " ");
            if(strcmp(tok, "subscribe") == 0){
                tok = strtok(NULL, " ");
                char* topic = strtok(NULL, " ");
                tok = strtok(NULL, " ");
                subscribe(topic, atoi(tok), strlen(topic));
            }
            else if(strcmp(tok, "unsubscribe") == 0){
                tok = strtok(NULL, " ");
                unsubscribe(tok, strlen(tok));
            }
            else if(strcmp(tok, "exit") == 0){
                close(sockfd);
                return 0;
            }
        }
        if(FD_ISSET(sockfd, &tmp_fds)){
            memset(buffer, 0, BUFFER_LEN);
            n = recv(sockfd, buffer, BUFFER_LEN, 0);

            unsigned int type;
            memcpy(&type, buffer, 1);
            if (type == 0)
            {
                uint8_t sign;
                memcpy(&sign, buffer + 1, 1);
                uint32_t number;
                memcpy(&number, buffer + 2, sizeof(uint32_t));
                int p = number;
                if(sign == 1){
                    p *= -1;
                }
                printf("%d\n", p);
            }
            else if (type == 1)
            {
                uint16_t number;
                memcpy(&number, buffer + 1, sizeof(uint16_t));
                float p = number / 100;
                printf("%f\n", p);
            }
            else if (type == 2)
            {
                uint8_t sign;
                memcpy(&sign, buffer + 1, 1);
                uint32_t number;
                memcpy(&number, buffer + 2, sizeof(uint32_t));
                uint8_t point;
                memcpy(&point, buffer + 2 + sizeof(uint32_t), sizeof(uint8_t));
                float p = number;
                for (int k = 0; k < point; k++){
                    p /= 10;
                }
                if(sign == 1){
                    p *= -1;
                }
                printf("%f\n", p);
            }
            else if (type == 3)
            {
                char string[1500];
                memcpy(string, buffer + 1, sizeof(string));
                puts(string);
            }
        }
    }
}

void subscribe(char topic[], int sf, int len){
    memset(buffer, 0, BUFFER_LEN);
    memcpy(buffer, topic, len);
    memcpy(buffer + 50, &sf, sizeof(sf));
    memcpy(buffer + 50 + sizeof(sf), "\0", 1);
    send(sockfd, buffer, 55, 0);
    printf("Subscribed to topic.\n");
}

void unsubscribe(char topic[], int len){
    memset(buffer, 0, BUFFER_LEN);
    memcpy(buffer, topic, len);
    memcpy(buffer + 50, "\0", 1);
    send(sockfd, buffer, 51, 0);
    printf("Unsubscribed from topic.\n");
}