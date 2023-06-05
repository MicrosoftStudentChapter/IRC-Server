#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define MAX_CLIENTS 20
#define BUFFER_SIZE 2048
#define NAME_LENGTH 32
#define CLIENTS_AT_A_TIME 10

//gloabl variables
int clients_fd[MAX_CLIENTS];
fd_set fdSet;

//functions
void reset_fd_set(int serverfd)
{
    FD_ZERO(&fdSet);
    FD_SET(serverfd, &fdSet);
    
    for(int i = 0; i<MAX_CLIENTS; i++)
    {
        if(clients_fd[i] == 0)
        {
            continue;
        }
        FD_SET(clients_fd[i], &fdSet);
    }
}

void trim_buffer(char *msg, int length)
{
    for(int i = 0; i<length; i++)
    {
        if(msg[i] == '\n' || msg[i] == '\r')
        {
            msg[i] = '\0';
            break;
        }
    }
}

void send_message(int clientfd, char* msg)
{
    sprintf(msg, "%s\n", msg);
    for(int i = 0; i<MAX_CLIENTS; i++)
    {
        if(clients_fd[i] == clientfd || clients_fd[i] == 0)
        {
            continue;
        }
        else
        {
            write(clients_fd[i], msg, strlen(msg));
        }
    }
}

void check(int check, char *msg)
{
    if(check < 0)
    {
        perror(msg);
    }
}

int main(int argc, char* argv[argc])
{
    //setting up variables
    int server_fd, max_fd, current_client, activity, portno, opt;
    opt = 1;
    char buffer[BUFFER_SIZE];
    portno = atoi(argv[1]);

    //setting up server properties
    struct sockaddr_in address;
    address.sin_port = htons(portno);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    socklen_t addrlen = sizeof(address);

    //initialising client file descriptors to 0
    for(int i = 0; i<MAX_CLIENTS; i++)
    {
        clients_fd[i] = 0;
    }

    //creating server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0)
    {
        perror("Socket Creation Failed\n");
    }
    
    //enabling server socket to resuse address
    check(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)), "setsocket Failed\n");

    //binding server
    check( bind(server_fd, (struct sockaddr*)&address, addrlen) , "Bind failed\n");

    //listening for new connections
    check( listen(server_fd, CLIENTS_AT_A_TIME), "listen failed\n");

    printf("Server listening on Port %d\n", portno);

    //clearing the socket file descriptors
    FD_ZERO(&fdSet);
    FD_SET(server_fd, &fdSet);

    //accepting clients
    while(1)
    {
        bzero(buffer, BUFFER_SIZE);
        int sd;
        max_fd = server_fd;

        //setting up client fds in read fds set
        for(int i = 0; i<MAX_CLIENTS; i++)
        {
            sd = clients_fd[i];
            if(sd > 0)
            {
                FD_SET(sd, &fdSet);
            }
            //finding maximum file descriptor
            if(sd > max_fd)
            {
                max_fd = sd;
            }
        }
        //printf("Waiting for activity\n");
        reset_fd_set(server_fd);
        activity = select(max_fd + 1, &fdSet, NULL, NULL, NULL);

        check(activity, "Select Failed\n");

        if(activity == 0)
        {
            continue;
        }
        else if(FD_ISSET(server_fd, &fdSet))
        {
            sprintf(buffer, "Welcome to the Server\n");
            int new_client = accept(server_fd, (struct sockaddr*)&address, &addrlen);
            if(new_client == 0)
            {
                printf("Accept Failed\n");
                exit(1);
            }
            printf("Client %d added\n", new_client);
            write(new_client, buffer, strlen(buffer));
            //adding new client to array
            for(int i = 0; i<MAX_CLIENTS; i++)
            {
                if(clients_fd[i] == 0)
                {
                    clients_fd[i] = new_client;
                    break;
                }
            }
        }
        else
        {
            for(int i = 0; i<MAX_CLIENTS; i++)
            {
                if(clients_fd[i] == 0)
                {
                    continue;
                }
                else if(FD_ISSET(clients_fd[i], &fdSet))
                {
                    if(read(clients_fd[i], buffer, BUFFER_SIZE) == 0)
                    {
                        printf("client %d closing connection\n", clients_fd[i]);
                        FD_CLR(clients_fd[i], &fdSet);
                        close(clients_fd[i]);
                        clients_fd[i] = 0;
                        break;
                    }
                    trim_buffer(buffer, strlen(buffer));
                    printf("%s\n", buffer);
                    if(strcmp(buffer, "/leave") == 0)
                    {
                        printf("client %d closing connection\n", clients_fd[i]);
                        FD_CLR(clients_fd[i], &fdSet);
                        close(clients_fd[i]);
                        clients_fd[i] = 0;
                        break;
                    }
                    else
                    {
                        send_message(clients_fd[i], buffer);
                        bzero(buffer, BUFFER_SIZE);
                        break;
                    }
                }
            }
        }
    }
}