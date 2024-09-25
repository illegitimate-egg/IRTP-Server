#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("nuh uh\n");
    return 1;
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in address = {AF_INET, htons(6750), 0};

  int _ = connect(sockfd, (struct sockaddr *)&address, sizeof(address));

  char buffer[5120] = {0};

  if (!strcmp(argv[1], "0")) {
    strcpy(buffer, "IDENT scooby\\ doo IRTP/1.0\n");
  } else if (!strcmp(argv[1], "1")) {
    strcpy(buffer, "IDENT scooby\\ doo IRTP/1.0\n");
    send(sockfd, buffer, strlen(buffer), 0);
    strcpy(buffer, "SEND genertals IRTP/1.0\n\nHouse MD rizzle dizzle mega "
                   "balls (steamy creamy)\n");
  }

  send(sockfd, buffer, strlen(buffer), 0);
  return 0;
}
