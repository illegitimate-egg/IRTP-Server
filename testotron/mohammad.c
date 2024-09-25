#include <arpa/inet.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address = {AF_INET, htons(80), 0};
  inet_aton("162.159.138.232", &address.sin_addr);

  int _ = connect(sockfd, (struct sockaddr *)&address, sizeof(address));

  char buffer[] = "POST /api/webhooks/1286062066783752313/T0wEuNBkVZnVxJyeNco0LRwo31JB6cVzgtK2YRNM3bX_kN1ZK25pLTkzYbpJdsQ384_7 HTTP/1.1\nHost: discord.com\nContent-Type: application/json\nContent-Length: 55\n\n{content: \"<@1114831458486464572> WAKE UP WE NEED YOU\"}\n";

  send(sockfd, buffer, strlen(buffer), 0);
  return 0;
}
