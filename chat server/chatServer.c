#include <sys/types.h>        
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*server ip and port number*/
#define PORT 3000
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024

//buffers for sending and recieving message
char sendBuf[BUFFER_SIZE + 1], recvBuf[BUFFER_SIZE + 1];

//socket id to be shared accross send and recieve thread
int res_socket;

//function prototypes
void *sendMessage(void *arg);
void *recvMessage(void *arg);
void error(char *msg);

int main(int argc, char const *argv[])
{
    // Create socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        error("Failed to create socket");
    }

    //set port and IP of server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr);

    // Bind to the set port and IP:
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Couldn't bind to specified port and IP");
    }

    // Listen for clients
    if(listen(server_sock, 1) < 0)
    {
        error("Error while listening\n");
    }
    printf("Listening for incoming connections.....\n");
    
    // Accept an incoming connection:
    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    res_socket = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (res_socket < 0)
    {
        error("Can't accept");
    }
    printf("Client connected @IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    printf("Enter 'END' to stop sending.\n");

    // Create thread for sending and recieving messages
    pthread_t recvThread, sendThread;
    if (pthread_create(&recvThread, NULL, recvMessage, NULL) != 0)
    {
        error("Thread creation failed");
    }
    if (pthread_create(&sendThread, NULL, sendMessage, NULL) != 0)
    {
        error("Thread creation failed");
    }

    // wait for all the threads
    pthread_join(recvThread, NULL);  
    pthread_join(sendThread, NULL);

    // Close sockets
    close(res_socket);
    close(server_sock);
    printf("ALl done!\n");
    return 0;
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void *sendMessage(void *arg)
{
    while (strncmp(sendBuf, "END", 3))
    {
        fgets(sendBuf, BUFFER_SIZE, stdin);
        if (send(res_socket, sendBuf, strlen(sendBuf), 0) < 0)
        {
            error("Couldn't send message");
        }
    }
    printf("Closed sending end.\n");
    pthread_exit(0);
}

void *recvMessage(void *arg)
{
    int msglen = 0;
    while (strncmp(sendBuf, "END", 3))
    {
        msglen = recv(res_socket, recvBuf, BUFFER_SIZE, 0);
        if (msglen < 0)
        {
            error("Couldn't receive message");
        } 
        if (strncmp(recvBuf, "END", 3) == 0)
        {
            printf("Closed recieving end.\n");
            break;
        }    
        if (msglen > 1)
        {
            printf("Recieved: %s", recvBuf);
            memset(recvBuf, '\0', msglen);
        }
    }
    pthread_exit(0);
}
