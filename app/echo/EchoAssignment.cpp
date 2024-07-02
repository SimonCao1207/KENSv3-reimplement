#include "EchoAssignment.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <arpa/inet.h>

// !IMPORTANT: allowed system calls.
// !DO NOT USE OTHER NETWORK SYSCALLS (send, recv, select, poll, epoll, fork
// etc.)
//  * socket
//  * bind
//  * listen
//  * accept
//  * read
//  * write
//  * close
//  * getsockname
//  * getpeername
// See below for their usage.
// https://github.com/ANLAB-KAIST/KENSv3/wiki/Misc:-External-Resources#linux-manuals

int EchoAssignment::serverMain(const char *bind_ip, int port,
                               const char *server_hello) {
  // Your server code
  // !IMPORTANT: do not use global variables and do not define/use functions
  // !IMPORTANT: for all system calls, when an error happens, your program must
  // return. e.g., if an read() call return -1, return -1 for serverMain.
  int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket == -1) {
    close(server_socket);
    return -1;
  }

  struct sockaddr_in address;
  socklen_t address_len = sizeof(address);
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  if (inet_pton(AF_INET, bind_ip, &address.sin_addr) <= 0){
    close(server_socket);
    return -1;
  }

  if (bind(server_socket, (struct sockaddr *) &address, address_len) < 0){
    close(server_socket);
    return -1;
  }

  if (listen(server_socket, 1024) < 0) {
    close(server_socket);
    return -1;
  }

  int BUFFER_SIZE = 1024;
  char client_request[BUFFER_SIZE] = {0};
  char client_ip[INET_ADDRSTRLEN];
  char* response;

  // Server accepts a new connection
  while (1){
    memset(client_request, 0, BUFFER_SIZE);
    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket < 0 ) {
      close(client_socket);
      return -1;
    }
    struct sockaddr_in client_address;
    if (getpeername(client_socket, (struct sockaddr *) &client_address, &address_len) < 0) {
      close(client_socket);
      return -1;
    }
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);

    // Server receives request terminated by a new line character
    read(client_socket, client_request, BUFFER_SIZE);

    if (!strcmp(client_request, "hello")) response = (char*) server_hello;
    else if (!strcmp(client_request, "whoami")) response = client_ip;
    else if (!strcmp(client_request, "whoru")) response = "!address";
    else response = client_request;

    write(client_socket, response, strlen(response));

    // Server logs requests via submit answer method
    submitAnswer(client_ip, client_request);

    close(client_socket);
  }
  return 0;
}

int EchoAssignment::clientMain(const char *server_ip, int port,
                               const char *command) {
  // Your client code
  // !IMPORTANT: do not use global variables and do not define/use functions
  // !IMPORTANT: for all system calls, when an error happens, your program must
  // return. e.g., if an read() call return -1, return -1 for clientMain.
  int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  if (inet_pton(AF_INET, server_ip, &address.sin_addr) <= 0){
    close(client_socket);
    return -1;
  }

  // Client connect to an echo server
  if (connect(client_socket, (struct sockaddr*) &address, sizeof(address)) < 0){
    close(client_socket);
    return -1;
  }
  
  // Client sends a request to the server
  write(client_socket, command, strlen(command));

  // Client receives response from the server
  int BUFFER_SIZE = 1024;
  char response[BUFFER_SIZE] = {0};
  int byte_reads = read(client_socket, response, BUFFER_SIZE);
  if (byte_reads < 0){
    close(client_socket);
    return -1;
  }

  if (!strcmp(response, "!address")) submitAnswer(server_ip, server_ip);
  else submitAnswer(server_ip, response);

  close(client_socket);

  return 0;
}

static void print_usage(const char *program) {
  printf("Usage: %s <mode> <ip-address> <port-number> <command/server-hello>\n"
         "Modes:\n  c: client\n  s: server\n"
         "Client commands:\n"
         "  hello : server returns <server-hello>\n"
         "  whoami: server returns <client-ip>\n"
         "  whoru : server returns <server-ip>\n"
         "  others: server echos\n"
         "Note: each command is terminated by newline character (\\n)\n"
         "Examples:\n"
         "  server: %s s 0.0.0.0 9000 hello-client\n"
         "  client: %s c 127.0.0.1 9000 whoami\n",
         program, program, program);
}

int EchoAssignment::Main(int argc, char *argv[]) {

  if (argc == 0)
    return 1;

  if (argc != 5) {
    print_usage(argv[0]);
    return 1;
  }

  int port = atoi(argv[3]);
  if (port == 0) {
    printf("Wrong port number\n");
    print_usage(argv[0]);
  }

  switch (*argv[1]) {
  case 'c':
    return clientMain(argv[2], port, argv[4]);
  case 's':
    return serverMain(argv[2], port, argv[4]);
  default:
    print_usage(argv[0]);
    return 1;
  }
}
