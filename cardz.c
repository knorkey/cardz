#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

// struct player contains player information. unsigned char score allows a maximum score of 255. winmes is message displayed when player wins the game.
struct player {
  char name[128];
  char winmes[1024];
  unsigned char score;
};

// fgets() saves the newline character from pressing enter into the string. strip_string() overrides this newline with \0 to end string correctly.
void strip_string (char* string) {
  string[strlen(string)-1] = '\0';
}

// ask_player() saves player input of their name and winmes into the player struct and returns pointer to it.
struct player* ask_player() {
  struct player* player = malloc(sizeof(struct player));

  // for some reason scanf("%s", player->name); caused the fgets() for winmes to misbehave. workaround for now.
  printf("please enter your player name: ");
  fgets(player->name, sizeof(player->name), stdin);
  strip_string(player->name);

  // need to use fgets() because scanf() interprets whitespace as end of input.
  printf("please enter your winning message (please stay respectful (if you want to)): ");
  fgets(player->winmes, sizeof(player->winmes), stdin);
  strip_string(player->winmes);

  return player;
}

// asks ip and port of host upon starting client. address to sockaddr is returned for use in connect().
struct sockaddr* ask_sockaddr_client() {
  char ip[15];
  short port;

  // the client asks for ipv4 address and port of hosting player.
  printf("please set ip and port: ");
  scanf("%s%d", &ip, &port);
  printf("addressing %s on port %d...", ip, port);

  /*
  to establish network connection with server, connect() function requires pointer to struct sockaddr, which contains
    -family --> type of connection. AF_INET is ipv4.
    -port --> port on which server listens to requests. needs to be converted with htons for network compatibility.
    -addr --> ipv4 address of server.
              inet_pton() converts conventional notation (for example 127.0.0.1) to network compatible format and saves to sin_addr of sock_addr.
    struct sockaddr_in is a better readable version of sockaddr, therefore needs to be cast to sockaddr for use in connect().
  */
  struct sockaddr_in* sock_addr = malloc(sizeof(struct sockaddr_in));
  sock_addr->sin_family = AF_INET;
  sock_addr->sin_port = htons(port);
  inet_pton(AF_INET, ip, &sock_addr->sin_addr);

  return (struct sockaddr*)sock_addr;
}

// analogous to ask_sockaddr_client(), but only requires port to listen to, as server listens on all of its ip addresses for connection request.
struct sockaddr* ask_sockaddr_server() {
  short port;
  printf("please set port to listen to: ");
  scanf("%d", &port);
  printf("listening on port %d...\n", port);

  // same as client sockaddr_in, but addr is set to all addresses via INADDR_ANY, which is converted by htons().
  struct sockaddr_in* sock_addr = malloc(sizeof(struct sockaddr_in));
  sock_addr->sin_family = AF_INET;
  sock_addr->sin_port = htons(port);
  // sin_addr is of struct type in_addr, which only contains s_addr which htons can write to. trying to address sin_addr directly fails due to different data types.
  sock_addr->sin_addr.s_addr = htons(INADDR_ANY);

  return (struct sockaddr*)sock_addr;
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    // the game requires an argument to indicate client or server role. abort with errorcode 1 if none is provided.
    printf("cardz needs information on whether to run as client or server. aborting.\n");
    return 1;
  } else if (strcmp(argv[1], "client") == 0) {
    // create player and sock_addr via respective functions.
    struct player* player = ask_player();
    struct sockaddr* sock_addr = ask_sockaddr_client();

    // socket provides information on which network protocols to use. AF_INET is ipv4, SOCK_STREAM is tcp.
    // 0 doesn't provide any special flags to socket() function.
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // establishes network connection to server with struct created by ask_sockaddr_client().
    // success of connect() returns 0, therefore skipping if. failure causes printf and program to exit.

    // !should implement way to try and reconnect for x times and only quit if this fails as well.
    if (connect(sock, sock_addr, sizeof(*sock_addr))) {
      printf("couldn't connect to host. aborting.\n");
      return 1;
    };

    send(sock, player, sizeof(*player), 0);
    close(sock);
    printf("shutting down. bye bye!\n");
    // terminate with no error code.
    return 0;
  } else if (strcmp(argv[1], "server") == 0) {
    struct sockaddr* sock_addr = ask_sockaddr_server();

    // establish socket parameters.
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // server binds to provided port to receive requests.
    bind(sock, sock_addr, sizeof(*sock_addr));
    // server listens and can take up to 1024 requests in its queue.
    listen(sock, 1024);

    // needs to save size of sock_addr as int, so its address can be provided to accept();
    int addr_length = sizeof(*sock_addr);
    // server accepts connection request. socket is reused for connection.
    sock = accept(sock, sock_addr, &addr_length);

    // create an array for a maximum of 8 players to play in one session.
    struct player players[8];

    // receives message from client and saves it to first slot of player array. 0 provides no special flags.
    // !need to implement loop so multiple players can connect at the same time.
    recv(sock, &players[0], sizeof(players[0]), 0);
    close(sock);

    // prints message from clients.
    printf("\nplayer %s connected with %d points.\n", players[0].name, players[0].score);
    printf("if it was possible to win this game yet, %s could have taunted their fellow players by saying \"%s\". what a pitty!\n", players[0].name, players[0].winmes);
    printf("shutting down. bye bye!\n");
    // exits with no errorcode.
    return 0;
  } else {
    // when not provided with either client or server argument, quit with errorcode 2.
    printf("argument not recognized. must be client or server. aborting.\n");
    return 2;
  }
}
