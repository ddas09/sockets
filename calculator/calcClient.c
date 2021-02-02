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

//buffer for sending command
char buffer[BUFFER_SIZE];

//function prototypes
void sendCommand(int socket);
void error(char *msg);

int main(int argc, char const *argv[])
{
    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        error("Failed to create socket");
    }

    //set port and IP of server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr);

    // Send connection request to server:
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Unable to connect to server");
    }
    printf("Connected with server @IP: %s and port: %i\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    printf("Enter 'END' to stop sending.\n");

    //send command to server
    sendCommand(client_socket);

    // Close socket
    close(client_socket);
    printf("All done!\n");

    return 0;
}

void sendCommand(int socket)
{
    while (1)
    {
        printf("Enter command: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        if (strncmp(buffer, "END", 3) == 0)
        {
            //inform server
            if (send(socket, "END", 3, 0) < 0)
            {
                error("Couldn't send command");
            }  
            break;
        }

        //sending command
        if (send(socket, buffer, strlen(buffer), 0) < 0)
        {
            error("Couldn't send command");
        }  
        memset(buffer, '\0', strlen(buffer));

        //recieve result from server
        if (recv(socket, buffer, BUFFER_SIZE, 0) < 0)
        {
            error("Couldn't recieve result");
        }  
        printf("Result recieved from server: %s\n", buffer);
        memset(buffer, '\0', strlen(buffer));
    }    
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}


