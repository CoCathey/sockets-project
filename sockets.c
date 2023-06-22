/* 
Bonus Lab - Program 7 
Creighton Cathey
ccathe4
cs3304ao
4-25-23
Peer programming with Sockets
*/

/*
Please Reference README.txt for explaination 
*/ 

// Provides socket, bind, and listen
#include <sys/socket.h>
 // Used for IPv4 addresses 
#include <netinet/in.h>
 // Provides inet_pton and inet_ntop
#include <arpa/inet.h>
 // Provides POSIX functions 
#include <unistd.h>
 // Provides input and output 
#include <stdio.h>
 // Provides exit and srand
#include <stdlib.h>
 // Provides string manipulation
#include <string.h>
 // Provides signal function
#include <signal.h>
 // Used for reporting errors 
#include <errno.h>

// Had to add function prototypes to get rid of errors 
void handle_sigpipe(int sig);
void handle_sigint(int sig);

// Defined the random ephemeral port given in spec 
#define PORT_RANGE_LOW 20000
#define PORT_RANGE_HIGH 50000
#define BUFFER_SIZE 1024

// Signal handler for SIGPIPE
// This handles the situation when the peer disconnects, 
// and the program tries to send a message on the closed socket.
void handle_sigpipe(int sig) {
  (void) sig; // Had to add this line to get rid of warnings
  printf("peer disconnected: exiting\n");
  exit(0);
}

// Signal handler for SIGINT
// Allows the user to close the program by typing y or n
void handle_sigint(int sig) {
  char input;
  (void) sig; // Had to add this line to get rid of warnings

  printf("\nDo you want to exit the program? (y/n): ");
  scanf(" %c", & input);

  if (input == 'y' || input == 'Y') {
    printf("Exiting...\n");
    exit(0);
  }
}

// Main function
// Had to make main function a void to get rid of warnings 
int main(void) {
  int sockfd, peerfd, server_port;
  struct sockaddr_in addr;
  char buffer[BUFFER_SIZE];

  // Signal handlers for SIGPIPE and SIGINT
  // For disconnection and interrupts.
  signal(SIGPIPE, handle_sigpipe);
  signal(SIGINT, handle_sigint);

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Define the sockaddr_in 
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  // This tries binding to a port in the given range spec
  // Uses a random port number for communication 
  while (1) {
    server_port = PORT_RANGE_LOW + rand() % (PORT_RANGE_HIGH - PORT_RANGE_LOW);
    addr.sin_port = htons(server_port);

    if (bind(sockfd, (struct sockaddr * ) & addr, sizeof(addr)) == 0) {
      break;
    }
  }

  printf("Server role. Listening on port %d.\n", server_port);

  // If the program acts as a server, it will listen for incoming connections.
  if (listen(sockfd, 1) < 0) {
    perror("listen");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // If the program successfully accepts a connection, it is a server.
  peerfd = accept(sockfd, NULL, NULL);
  if (peerfd < 0) {
    // If accept() fails, connect to an existing instance
    // If the program cannot accept connections, it will try to connect as a client.
    perror("accept");
    printf("Trying to connect to an existing connection...\n");
    addr.sin_port = htons(server_port);

    if (connect(sockfd, (struct sockaddr * ) & addr, sizeof(addr)) < 0) {
      perror("connect");
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    printf("Connected to the server at port %d\n", server_port);
    snprintf(buffer, BUFFER_SIZE, "Hello World!");
  } else {
    // If accept() is successful, send message to the connected peer
    printf("Accepted connection! \n");
    snprintf(buffer, BUFFER_SIZE, "This is the server!");
    // Close the listening socket bc we dont need it 
    close(sockfd);
  }

  // Send the hello message
  // Once connected, both server and client send their hello messages.
  if (send(peerfd, buffer, strlen(buffer), 0) < 0) {
    perror("send");
    close(peerfd);
    exit(EXIT_FAILURE);
  }

  // This loop is used to send and receive messages 
  while (1) {
    memset(buffer, 0, BUFFER_SIZE);

    // Listens for incoming messages from the peer
    if (recv(peerfd, buffer, BUFFER_SIZE, 0) <= 0) {
      perror("recv");
      close(peerfd);
      exit(EXIT_FAILURE);
    }

    // Prints the received message to the console
    printf("Received message: %s\n", buffer);

    // Prompts  user to enter a message to send 
    printf("Enter a message to send: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    // This removes the trailing newline character
    buffer[strlen(buffer) - 1] = '\0';

    // Send the message to the peer
    if (send(peerfd, buffer, strlen(buffer), 0) < 0) {
      perror("send");
      close(peerfd);
      exit(EXIT_FAILURE);
    }
  }

  // Close the connection
  close(peerfd);
  return 0;
}
