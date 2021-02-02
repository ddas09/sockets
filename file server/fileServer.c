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
#define BUFFER_SIZE 65535 //65KB buffer

//buffer for sending file data
char buffer[BUFFER_SIZE];

/*structures to perform semop*/
struct sembuf wait = {0, -1, 0};
struct sembuf signal = {0, 1, 0};

//semaphores for synchronising send and recieve of file data
int fullBuf, emptyBuf;

//function prototypes
void fileSend(const char *filename, FILE *fp, int socket);
void error(char *msg);

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./fileServer 'filename'\n");
        exit(-1);
    }    

    //open file in binary mode
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        error("Couldn't open file");
    } 

    //Create socket
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

    //Bind to the set port and IP:
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Couldn't bind to specified port and IP");
    }

    //Listen for clients
    if(listen(server_sock, 1) < 0)
    {
        error("Error while listening\n");
    }
    printf("Listening for incoming connections.....\n");
    
    //Accept an incoming connection:
    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    int res_socket = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
    if (res_socket < 0)
    {
        error("Can't accept");
    }
    printf("Client connected @IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    //create key for semaphores
    key_t key_full = ftok("./keyFull.txt", 9);
    key_t key_empty = ftok("./keyEmpty.txt", 9);
    if (key_full == -1 || key_empty == -1)
    {
        error("Couldn't generate key for semaphore");
    }    

    //creating the semaphores 
    fullBuf = semget(key_full, 1, IPC_CREAT | 0666);
    emptyBuf = semget(key_empty, 1, IPC_CREAT | 0666);
    if (emptyBuf == -1 || fullBuf == -1)
    {
        error("Semaphore creation failed");        
    }

    //initializing semaphores
    union semun
    {
        int val;    
    } arg;
    arg.val = 0;
    if (semctl(fullBuf, 0, SETVAL, arg) == -1 || semctl(emptyBuf, 0, SETVAL, arg) == -1)
    {
        error("Semaphore initialization failed");
    }

    //Send file data
    fileSend(argv[1], fp, res_socket);

    //delete semaphores
    if (semctl(fullBuf, 0, IPC_RMID, 0) == -1 || semctl(emptyBuf, 0, IPC_RMID, 0) == -1)
    {
        error("Couldn't remove semaphore");
    }

    //Close sockets and file
    close(res_socket);
    close(server_sock);
    fclose(fp);
    printf("ALl done!\n");
    
    return 0;
}

void fileSend(const char *filename, FILE *fp, int socket)
{
    //send file name
    if (send(socket, filename, strlen(filename), 0) < 0)
    {
        error("Couldn't send filename");
    }

    //send file data
    int items = 0;
    long file_size = 0;
    while (1)
    {
        if (semop(emptyBuf, &wait, 1) == -1)
        {
            error("Failed to lock");
        }

        //read from file
        items = fread(buffer, 1, BUFFER_SIZE, fp);
        //end of file 
        if (items == 0)
        {
            //inform client and release lock
            if (send(socket, "END", 3, 0) < 0)
            {
                error("Couldn't send data");
            }
            if (semop(fullBuf, &signal, 1) == -1)
            {
                error("Failed to signal");
            }
            break;
        }    

        //send data    
        if (send(socket, buffer, items, 0) < 0)
        {
            error("Couldn't send data");
        }
        printf("Packet sent, size = %d bytes.\n", items);
        file_size += items;
        memset(buffer, '\0', items);

        if (semop(fullBuf, &signal, 1) == -1)
        {
            error("Failed to signal");
        }
    }

    printf("All packets sent, waiting for client to complete.\n");
    //wait for client to complete
    if (recv(socket, buffer, 3, 0) < 0)
    {
        error("Couldn't recieve data");
    }
    if (strncmp(buffer, "END", 3) != 0)
    {
        error("Sync Error");
    }    
    printf("File sent successfully, file size = %ld bytes.\n", file_size);
}

void error(char *msg)
{
    perror(msg);
    exit(-1);
}
