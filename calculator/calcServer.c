#include <sys/types.h>        
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 3000
#define IP "127.0.0.1"
#define BUFFER_SIZE 256

//buffer for recieving command
char buffer[BUFFER_SIZE];

//function prototypes
void sendResult(int socket);
void error(char *msg);

int main(int argc, char const *argv[])
{
    // Create socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        error("Failed to create socket");
    }

    // set port and IP of server
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
    if (listen(server_sock, 1) < 0)
    {
        error("Error while listening\n");
    }
    printf("Listening for incoming connections.....\n");
    
    // Accept an incoming connection:
    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    int res_socket = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (res_socket < 0)
    {
        error("Can't accept");
    }
    printf("Client connected @IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // send result back to client
    sendResult(res_socket);

    // Close sockets
    close(res_socket);
    close(server_sock);
    printf("ALl done!\n");

    return 0;
}

void sendResult(int socket)
{
    char cmd[4] = {'\0'};
    char *ptr = NULL;
    int num1 = 0, num2 = 0;
    while (1)
    {
        //recieving command
        if (recv(socket, buffer, BUFFER_SIZE, 0) < 0)
        {
            error("Couldn't recieve command");
        }    

        if (strncmp(buffer, "END", 3) == 0)
        {
            break;
        }   
        printf("Command recieved from client = %s", buffer);
        ptr = buffer;

        //extract command and numbers from buffer
        strncpy(cmd, buffer, 3);
        ptr += 4;

        //strtol converts initial part of string to long
        num1 = strtol(ptr, &ptr, 10);

        //skip blank space after first number
        ptr++;
        num2 = strtol(ptr, NULL, 10);
        memset(buffer, '\0', strlen(buffer));

        //process command
        if (strcasecmp(cmd, "ADD") == 0)
        {
            sprintf(buffer, "%d", num1 + num2);
        }    
        else if (strcasecmp(cmd, "SUB") == 0)
        {
            sprintf(buffer, "%d", num1 - num2);
        } 
        else if (strcasecmp(cmd, "MUL") == 0)
        {
            sprintf(buffer, "%d", num1 * num2);
        } 
        else if (strcasecmp(cmd, "DIV") == 0)
        {
            sprintf(buffer, "%.2f", num1 / (float)num2);
        } 
        else 
        {
            strcpy(buffer, "Invalid command");
        } 

        //sending result
        if (send(socket, buffer, strlen(buffer), 0) < 0)
        {
            error("Couldn't send result");
        } 
        memset(buffer, '\0', strlen(buffer));
    }    
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

