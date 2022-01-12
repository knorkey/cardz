#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("cardz needs information on whether to run as client or server. aborting.\n");
    return 1;
  } else if (strcmp(argv[1], "client") == 0) {
    char ip[15];
    short port;
    printf("please set ip and port: ");
    scanf("%s%d", &ip, &port);
    printf("addressing %s on port %d...\n", ip, port);

    struct in_addr addr;
    inet_pton(AF_INET, ip, &addr);
    struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = addr
    };

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    connect(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    char hello[] = "hello ma boi\n";
    send(sock, hello, sizeof(hello), 0);
    close(sock);
    printf("shutting down. bye bye!\n");
    return 0;
  } else if (strcmp(argv[1], "server") == 0) {
    short port;
    printf("please set port to listen to: ");
    scanf("%d", &port);
    printf("listening on port %d...\n", port);

    struct sockaddr_in sock_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = htons(INADDR_ANY)
    };

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    listen(sock, 1024);
    int addr_length = sizeof(sock_addr);

    sock = accept(sock, (struct sockaddr*)&sock_addr, &addr_length);

    char buff[1024];
    recv(sock, &buff, sizeof(buff), 0);
    printf("%s", buff);
    close(sock);
    printf("shutting down. bye bye!\n");
    return 0;
  } else {
    printf("argument not recognized. must be client or server. aborting.\n");
    return 2;
  }
}
