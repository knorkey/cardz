#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    // the game requires an argument to indicate client or server role. abort with errorcode 1 if none is provided.
    printf("cardz needs information on whether to run as client or server. aborting.\n");
    return 1;
  } else if (strcmp(argv[1], "client") == 0) {
    char ip[15];
    short port;
    // the client asks for ipv4 address and port of hosting player.
    printf("please set ip and port: ");
    scanf("%s%d", &ip, &port);
    printf("addressing %s on port %d...\n", ip, port);

    // to establish network connection with server, connect() function requires struct sockaddr, which contains
    // -family --> type of connection. AF_INET close(sock);is ipv4.
    // -port --> port on which server listens to requests. needs to be converted with htons for network compatibility.
    // -addr --> ipv4 address of server. inet_pton() converts conventional notation (127.0.0.1) to network compatible format and saves to addr.
    // struct sockaddr_in is a better readable version of sockaddr, therefore needs to be cast to sockaddr for connect() to work.
    struct in_addr addr;
    inet_pton(AF_INET, ip, &addr);
    struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = addr
    };

    // socket provides information on which network protocols to use. AF_INET is ipv4, SOCK_STREAM is tcp.
    // 0 doesn't provide any special flags to socket() function.
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // establishes network connection to server with previously created struct (needs to be cast to struct sockaddr*).
    connect(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    char hello[] = "hello ma boi\n";
    send(sock, hello, sizeof(hello), 0);
    close(sock);
    printf("shutting down. bye bye!\n");
    // terminate with no error code.
    return 0;
  } else if (strcmp(argv[1], "server") == 0) {
    // server listens on all of its network addresses, so only port needs to be set by user.
    short port;
    printf("please set port to listen to: ");
    scanf("%d", &port);
    printf("listening on port %d...\n", port);

    // same as client sockaddr_in, but addr is set to all addresses via (INADDR_ANY), which is converted by htons().
    struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = htons(INADDR_ANY)
    };

    // erstablish socket parameters.
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // server binds to provided port to receive requests.
    bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    // server listens and can take up to 1024 requests in its queue.
    listen(sock, 1024);

    // needs to save size of sock_addr as int, so its address can be provided to accept();
    int addr_length = sizeof(sock_addr);
    // server accepts connection request. socket is reused for connection.
    sock = accept(sock, (struct sockaddr*)&sock_addr, &addr_length);

    char buff[1024];
    // receives message form client and saves it to buff. 0 provides no special flags.
    recv(sock, &buff, sizeof(buff), 0);
    close(sock);
    // prints message from clients.
    printf("%s", buff);
    printf("shutting down. bye bye!\n");
    // exits with no errorcode.
    return 0;
  } else {
    // when not provided with either client or server argument, quit with errorcode 2.
    printf("argument not recognized. must be client or server. aborting.\n");
    return 2;
  }
}
