#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

struct client {
  std::string username;
  std::string processingMessage;
  int fd;
  bool active = true;
};

bool isRunning = true;

void communication(std::vector<client *> *clients, bool *blockConnection) {
  while (isRunning) {
    uint numberOfClients = clients->size();
    for (uint i = 0; i < numberOfClients; i++) {
      if (!clients->at(i)->active) {
        close(clients->at(i)->fd);
        delete clients->at(i);
        clients->erase(clients->begin() + i);
        numberOfClients--;
        i--;
      }
    }

    struct pollfd fds[numberOfClients + 1];
    for (uint i = 0; i < numberOfClients; i++) {
      fds[i].fd = clients->at(i)->fd;
      fds[i].events = POLLIN;
      fds[i].revents = 0;
    }

    if (poll(fds, numberOfClients, 500)) {
      *blockConnection = true;
      for (uint i = 0; i < numberOfClients; i++) {
        if (fds[i].revents & POLLIN) {
          char buffer[5121];
          if (recv(clients->at(i)->fd, buffer, 5120, 0) < 0) {
            clients->at(i)->active = false;
          } else {
            clients->at(i)->processingMessage.append(buffer);
            for (uint j = 0; j < numberOfClients; j++) {
              if (send(clients->at(j)->fd,
                       clients->at(i)->processingMessage.c_str(),
                       clients->at(i)->processingMessage.length(),
                       MSG_NOSIGNAL) < 0) {
                clients->at(j)->active = false;
              }
            }
            clients->at(i)->processingMessage = "";
          }
          memset(buffer, 0, sizeof buffer);
        }
      }
      *blockConnection = false;
    }
  }
}

void handleSigint(int) {
  isRunning = false;
  std::cout << "EXITING" << std::endl;
}

int main() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == 0) {
    fprintf(stderr, "Failed to create the socket!\n");
    return 1;
  }

  struct sockaddr_in address = {AF_INET, htons(6750), 0};

  if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    fprintf(stderr, "Failed to bind the port: %s\n", strerror(errno));
    return 1;
  }

  std::vector<client *> clients;
  bool blockConnection = false;

  std::thread ct(communication, &clients, &blockConnection);
  ct.detach();

  struct sigaction cleanExit = {
    &handleSigint,
    0,
    0,
    0,
    0
  };
  sigaction(SIGINT, &cleanExit, NULL);

  while (isRunning) {
    listen(sockfd, 256);
    int clientfd = accept(sockfd, 0, 0);
    if (clientfd != 0) {
      while (blockConnection)
        ;
      client *currClient = new client;
      currClient->active = true;
      currClient->fd = clientfd;

      clients.push_back(currClient);
    }
  }

  for (uint i = 0; i > clients.size(); i++) {
    // clients[i]->active = false; // Important for multithreading
    close(clients[i]->fd);
    delete clients[i];
  }
  return 0;
}
