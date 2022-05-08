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

#define BUFFER_LEN 1570
#define ID_LEN 10

void subscribe(char topic[], int sf, int len);
void unsubscribe(char topic[], int len);

char idClient[ID_LEN];
int sockfd;
struct sockaddr_in serv_addr;
char buffer[BUFFER_LEN];
char recvBuffer[BUFFER_LEN * 2];
fd_set read_fds, tmp_fds;
int ret;
char* pointer;

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

    //Server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    inet_aton(argv[2], &serv_addr.sin_addr);

    //Connect to server
    ret = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0){
        fprintf(stderr, "Cannot connect to server.\n");
    }
    memset(buffer, 0, sizeof(BUFFER_LEN));
    memcpy(buffer, idClient, sizeof(idClient));
    buffer[ID_LEN] = '\0';
    //Send ID to server
    ret = send(sockfd, buffer, ID_LEN + 1, 0);
    if(ret < 0){
        fprintf(stderr, "Cannot send message to server.\n");
    }

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    char command[100];
    char *tok;

    pointer = recvBuffer;

    while (1)
    {
        tmp_fds = read_fds;

        ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        if(ret < 0){
            fprintf(stderr, "Cannot select fd.\n");
        }
        //Console input
        if(FD_ISSET(STDIN_FILENO, &tmp_fds)){
            fgets(command, 100, stdin);
            tok = strtok(command, " \n");
            if(strcmp(tok, "subscribe") == 0){  //Subscribe
                char* topic = strtok(NULL, " ");
                tok = strtok(NULL, " ");
                subscribe(topic, atoi(tok), strlen(topic));
            }
            else if(strcmp(tok, "unsubscribe") == 0){   //Unsubscribe
                tok = strtok(NULL, " ");
                unsubscribe(tok, strlen(tok));
            }
            else if(strcmp(tok, "exit") == 0){  //Exit
                shutdown(sockfd, SHUT_RDWR);
                close(sockfd);
                exit(0);
            }
            else{
                fprintf(stderr, "Invalid command.\n");
            }
        }
        if(FD_ISSET(sockfd, &tmp_fds)){ //Network input
            memset(buffer, 0, BUFFER_LEN);
            n = 0;
            char topic[51];
            //Receive until aan entire message is received
            while(1){
                n = recv(sockfd, pointer, BUFFER_LEN, 0);
                if(n < 0)
                {
                    fprintf(stderr, "Cannot receive message.\n");
                }
                else if(n == 0){
                    close(sockfd);
                    return 1;
                }
                //Move current pointer n bytes to the right
                pointer = recvBuffer + n;
                for(int i=0;i<n;i++){
                    //End of message is marked with \r
                    if(recvBuffer[i] == '\r'){
                        //Move message to buffer
                        memcpy(buffer, recvBuffer, BUFFER_LEN);
                        //Move rest of recvBuffer to the beggining
                        memcpy(recvBuffer, recvBuffer + BUFFER_LEN, BUFFER_LEN);
                        //Move pointer back
                        pointer -= BUFFER_LEN;
                        goto processMessage;
                    }
                }
            }
processMessage:
            memcpy(topic, buffer, 50);  //Get topic
            topic[50] = '\0';

            unsigned int type = 0;
            //Get message type
            memcpy(&type, buffer + 50, 1);

            char ip[16] = "127.0.0.1\0";
            //Get ip address from message
            memcpy(ip, buffer + 1551, 16);
            uint16_t port = 0;
            //Get port from message
            memcpy(&port, buffer + 1567, 2);
            if (type == 0)
            {
                uint8_t sign = 0;
                memcpy(&sign, buffer + 51, 1);
                uint32_t number = 0;
                memcpy(&number, buffer + 52, sizeof(uint32_t));
                int p = ntohl(number);  //Convert to correct byte order
                if(sign == 1){
                    p *= -1;    //Apply sign
                }
                printf("%s:%u - %s - INT - %d\n", ip, port, topic, p);
            }
            else if (type == 1)
            {
                uint16_t number = 0;
                memcpy(&number, buffer + 51, sizeof(uint16_t));
                unsigned short p = ntohs(number);   //Convert to correct byte order
                float res = (float)p / 100; //Turn into 2 digit float
                printf("%s:%u - %s - SHORT_REAL - %.2f\n", ip, port, topic, res);
            }
            else if (type == 2)
            {
                uint8_t sign = 0;
                memcpy(&sign, buffer + 51, 1);
                uint32_t number = 0;
                memcpy(&number, buffer + 52, sizeof(uint32_t));
                uint8_t point = 0;
                memcpy(&point, buffer + 52 + sizeof(uint32_t), sizeof(uint8_t));
                float p = ntohl(number);    //Convert to correct byte order
                for (int k = 0; k < point; k++){
                    p /= 10;    //Set point position
                }
                if(sign == 1){
                    p *= -1;    //Apply sign
                }
                printf("%s:%u - %s - FLOAT - %f\n", ip, port, topic, p);
            }
            else if (type == 3)
            {
                char string[1501];
                memcpy(string, buffer + 51, sizeof(string) - 1);
                string[1500] = '\0';
                printf("%s:%u - %s - STRING - %s\n", ip, port, topic, string);
            }
            else
            {
                fprintf(stderr, "Invalid message.\n");
            }
        }
    }
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    return 0;
}

//Send subscribe message to server
//First byte is s, marking a subscribe command
//50 bytes with the topic
//1 byte with sf
//1 byte null termination
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

//Send unsubscribe message to server
//First byte is s, marking a subscribe command
//50 bytes with the topic
//1 byte null termination
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