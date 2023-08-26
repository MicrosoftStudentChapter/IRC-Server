#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <vector>

using namespace std;

#define MAX_CLIENTS 20
#define BUFFER_SIZE 2048
#define NAME_LENGTH 32
#define CLIENTS_AT_A_TIME 10

class Client;
//gloabl variables
int clients_fd[MAX_CLIENTS];
fd_set fdSet;
vector<Client> Client_pointers;

class Client
{
    public : 
        string name; int fd, uid; 

    public : 

        Client(int Fd, int Uid, char* Name);
        Client(int Fd, int Uid);

        void setName(char* s)
        {
            name = s;
        }
        void send_message(char* msg);
        void handle_commands(char* cmd, int fd);
        void leave_client();
};

int search_by_fd(int fd)
{
    int i = 0;

    for( ; i < Client_pointers.size(); i++)
    {
        if(fd == Client_pointers[i].fd)
        {
            return i;
        }
    }
    return i;
}       

Client::Client(int Fd, int Uid)
{
    fd = Fd;
    uid = Uid;
}

Client::Client(int Fd, int Uid, char* Name)
{
    fd = Fd;
    uid = Uid;
    name = Name;
}


//functions

void checkClients()
{
    for(int i = 0; i<Client_pointers.size(); i++)
    {
        cout<<Client_pointers[i].name<<endl;
    }
}

void Client::leave_client()
{
    cout<<"client "<<name<<" closing connection\n";
    FD_CLR(fd, &fdSet);
    close(fd);
    fd = 0;
    name = "";
    uid = 0;
}

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

void Client::send_message(char* msg)
{
    sprintf(msg, "%s\n", msg);
    for(int i = 0; i<MAX_CLIENTS; i++)
    {
        if(clients_fd[i] == fd || clients_fd[i] == 0)
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

int search_by_name(string name)
{
    for(int i = 0; i<Client_pointers.size(); i++)
    {
        if(Client_pointers[i].fd == 0)
        {
            continue;
        }
        else
        {
            if(name.compare(Client_pointers[i].name) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

void extract(char* msg, char* cmd)
{
    int i = 0;
    for(; i<strlen(msg); i++)
    {
        if(msg[i] == ' ')
        {
            break;
        }
        cmd[i] = msg[i];
    }
    cmd[i] = '\0';
    cout<<cmd<<endl;
}

void send_message_private(char* msg, int fd)
{
    char name[NAME_LENGTH];
    extract(msg, name);
    int i = search_by_name(name);
    if(i!=-1)
    {
        cout<<"Message : "<<msg<<endl;
        write(Client_pointers[i].fd, &msg[strlen(name)+1], strlen(msg));
    }
    else
    {
        msg = "Name not Found\n";
        write(fd, msg, strlen(msg));
    }
}

void Client::handle_commands(char* msg, int fd)
{
    cout<<msg<<endl;
    char cmd[20];
    extract(msg, cmd);
    if(strcmp(cmd,"pm") == 0)
    {
        send_message_private(&msg[3], fd);
    }
}

//Main
int main(int argc, char* argv[])
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
    sprintf(buffer, "setsocket Failed\n");
    check(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)), buffer);
    bzero(buffer, BUFFER_SIZE);
    //binding server
    sprintf(buffer, "Bind failed\n");
    check( bind(server_fd, (struct sockaddr*)&address, addrlen) , buffer);
    bzero(buffer, BUFFER_SIZE);
    //listening for new connections
    sprintf(buffer, "listen failed\n");
    check( listen(server_fd, CLIENTS_AT_A_TIME), buffer);
    bzero(buffer, BUFFER_SIZE);
    cout<<"Server listening on Port "<<portno<<"\n";

    //clearing the socket file descriptors
    FD_ZERO(&fdSet);
    FD_SET(server_fd, &fdSet);

    int uid = 0;
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

        sprintf(buffer, "Select Failed\n");
        check(activity, buffer);
        bzero(buffer, BUFFER_SIZE);

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
                cout<<"Accept Failed\n";
                exit(1);
            }
            cout<<"Client "<<new_client<<" added\n" ;
            write(new_client, buffer, strlen(buffer));

            //-----------------
            //ADDING NEW CLIENT
            //-----------------

            for(int i = 0; i<MAX_CLIENTS; i++)
            {
                if(clients_fd[i] == 0)
                {
                    clients_fd[i] = new_client;
                    break;
                }
            }

            int i = 0;
            name:
            sprintf(buffer, "Enter a Name : ");
            write(new_client, buffer, strlen(buffer));
            bzero(buffer, BUFFER_SIZE);
            if(read(new_client, buffer, BUFFER_SIZE) == 0)
            {
            cout<<"error\n";
            }

            cout<<buffer<<endl;
            for( ; i<Client_pointers.size(); i++)
            {
                if(&Client_pointers[i] == NULL)
                {
                    continue;
                }   
                else
                {
                    string s = Client_pointers[i].name;
                    cout<<s<<endl;
                    cout<<buffer<<endl;
                    if(s.compare(buffer) == 0)
                    {
                        sprintf(buffer, "Name Already Taken\n");
                        write(new_client, buffer, strlen(buffer));
                        goto name;
                    }
                }
            }

            //Forming new Client Object
            i = search_by_fd(0);     
            if(i == Client_pointers.size())
            {
                cout<<"pushed new client\n";
                Client_pointers.push_back(Client(new_client, ++uid, buffer));
                cout<<Client_pointers[i].name<<endl;
                cout<<Client_pointers[i].uid<<endl;
                cout<<uid<<endl;
            }
            else
            {
                cout<<"used client renamed\n";
                Client_pointers[i] = Client(new_client, ++uid, buffer);
                cout<<Client_pointers[i].name<<endl;
                cout<<Client_pointers[i].uid;
            }
            bzero(buffer, BUFFER_SIZE);
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
                    int iter = search_by_fd(clients_fd[i]);
                    Client* cli;
                    cli = &Client_pointers[iter];
                    
                    if(iter == Client_pointers.size())
                    {
                        cout<<"error fd not found\n";
                        continue;
                    }
                    
                    if(read(clients_fd[i], buffer, BUFFER_SIZE) == 0)
                    {
                        cli->leave_client();
                        clients_fd[i] = 0;
                        break;
                    }
                    trim_buffer(buffer, strlen(buffer));
                    cout<<buffer<<endl;
                    if(strcmp(buffer, "/leave") == 0)
                    {
                        cli->leave_client();
                        clients_fd[i] = 0;
                        break;
                    }
                    else if(buffer[0] == '/')
                    {
                        cli->handle_commands(&buffer[1], cli->fd);
                    }
                    else
                    {
                        cli->send_message(buffer);
                        bzero(buffer, BUFFER_SIZE);
                        break;
                    }
                }
            }
        }
    }
}