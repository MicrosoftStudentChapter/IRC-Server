#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9999
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;

    // Create the socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_address.sin_zero), '\0', 8);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1) {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    int activity;
    char buffer[BUFFER_SIZE];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client_socket, &readfds);

        // Wait for activity on any socket
        activity = select(client_socket + 1, &readfds, NULL, NULL, NULL);

        if (activity == -1) {
            perror("Select failed");
            exit(EXIT_FAILURE);
        }

        // Handle user input
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = '\0';

            // Send the message to the server
            if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
                perror("Send failed");
                exit(EXIT_FAILURE);
            }
        }

        // Handle incoming messages from the server
        if (FD_ISSET(client_socket, &readfds)) {
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received == -1) {
                perror("Receive failed");
                exit(EXIT_FAILURE);
            } else if (bytes_received == 0) {
                // Server disconnected
                cout<<"Server disconnected\n";
                break;
            } else {
                buffer[bytes_received] = '\0';
                cout<<buffer<<endl;
                // cout<<"\n";
            }
        }
    }

    close(client_socket);
    return 0;
}