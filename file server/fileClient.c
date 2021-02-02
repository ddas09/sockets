#include <sys/types.h> 
#include <sys/ipc.h>
#include <sys/sem.h>        
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 3000
#define IP "127.0.0.1"
#define BUFFER_SIZE 65535 //65kb buffer

//buffer for recieving file data
char buffer[BUFFER_SIZE];

/*structures to perform semop*/
struct sembuf wait = {0, -1, 0};
struct sembuf signal = {0, 1, 0};

//semaphores for synchronising send and recieve of file data
int fullBuf, emptyBuf;

//function prototypes
void fileRecieve(int socket);
void error(char *msg);

int main(int argc, char const *argv[])
{
    //Create socket
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

    //#Send connection request to server:
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Unable to connect to server");
    }
    printf("Connected with server @IP: %s and port: %i\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    //create key for semaphores
    key_t key_full = ftok("./keyFull.txt", 9);
    key_t key_empty = ftok("./keyEmpty.txt", 9);
    if (key_full == -1 || key_empty == -1)
    {
        error("Couldn't generate key for semaphore");
    }    

    //getting the semaphores 
    fullBuf = semget(key_full, 1, 0666);
    emptyBuf = semget(key_empty, 1, 0666);
    if (emptyBuf == -1 || fullBuf == -1)
    {
        error("Failed to accquire semaphores");        
    }

    //recieve file data
    fileRecieve(client_socket);

    //Close socket
    close(client_socket);
    printf("All done!\n");
    
    return 0;
}

void fileRecieve(int socket)
{
    //recieve source file name 
    if (recv(socket, buffer, BUFFER_SIZE, 0) < 0)
    {
        error("Failed to recieve filename");
    }    

    //prefix dest file name
    char filename[100] = "dest_";
    strcat(filename, buffer);

    //create dest file
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        error("Failed to create destination file");
    }    
    memset(buffer, '\0', strlen(buffer));

    //recieve and write data to dest file
    int items = 0;
    long file_size = 0;
    while (1)
    {
        if (semop(emptyBuf, &signal, 1) == -1)
        {
            error("Failed to signal");
        }
        if (semop(fullBuf, &wait, 1) == -1)
        {
            error("Failed to lock");
        }

        //recieving data
        items = recv(socket, buffer, BUFFER_SIZE, 0);
        if (items < 0)
        {
            error("Couldn't recieve data");
        }
        if (strncmp(buffer, "END", 3) == 0)
        {
            break;
        }
        printf("Packet recieved, size = %d bytes.\n", items);
        file_size += items;

        //write to file
        fwrite(buffer, 1, items, fp);
        memset(buffer, '\0', items);
    } 

    printf("All packets recieved, informing server.\n");
    //inform server
    if (send(socket, "END", 3, 0) < 0)
    {
        error("Couldn't send data");
    }
    fclose(fp);
    printf("File recieved successfully, file size = %ld bytes.\n", file_size);   
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}