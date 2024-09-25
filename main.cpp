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

enum protocolRecMessages { _INCOMPLETE = -1, IDENT, SEND, IMAGE };

std::string escapedString(char **ptr, char *vicodin) {
  bool escape = true;
  std::string retval = "";
  while (escape) {
    vicodin = strtok_r(NULL, " ", ptr);

    if (vicodin[strlen(vicodin) - 1] != '\\') {
      retval.append(vicodin);
      escape = false;
    } else {
      vicodin[strlen(vicodin) - 1] =
          '\0'; // Convert the backslash to null smeller
      retval.append(vicodin);
      retval.append(" ");
    }
  }

  return retval;
}

std::string escape(std::string inputString) {
  char *ptr;
  std::string output = "";

  char c_string[inputString.length() + 1];
  strcpy(c_string, inputString.c_str());

  char *res;
  res = strtok_r(c_string, " ", &ptr);

  while (res != NULL) {
    if (output.length() != 0) {
      output.append("\\ ");
    }
    output.append(res);
  }

  return output;
}

int IRTPRec(std::vector<client *> *clients,
            int index) { // I am not qualified to write a parser lmfao
  client *client = clients->at(index);
  char *nlPtr;

  int recType = _INCOMPLETE;

  int word, row = 0;

  char c_message[client->processingMessage.length() + 1];
  strcpy(c_message, client->processingMessage.c_str());

  char *res;
  res = strtok_r(c_message, "\n", &nlPtr);

  std::string channelName = "";

  while (res != NULL) {
    word = 0;
    char *spPtr;
    char *vicodin;

    char completeString[strlen(res) + 1];
    strcpy(completeString, res);

    vicodin = strtok_r(res, " ", &spPtr);

    while (vicodin != NULL) {
      if (recType == SEND) {
        std::cout << "(" << channelName << ") " << client->username << ": "
                  << completeString << std::endl;
        std::string outGoing = "SEND " + escape(channelName) + " IRTP/1.0\n\n" + completeString + "\n";
        for (uint j = 0; j < clients->size(); j++) {
          if (send(clients->at(j)->fd,
                   outGoing.c_str(),
                   outGoing.length(),
                   MSG_NOSIGNAL) < 0) {
            clients->at(j)->active = false;
          }
        }
        client->processingMessage = "";
        row = 0;
        recType = _INCOMPLETE;
      }
      if (word == 0) {
        if (strcmp(vicodin, "IDENT") == 0) {
          recType = IDENT;

          std::string username = escapedString(&spPtr, vicodin);

          vicodin = strtok_r(NULL, " ", &spPtr);
          if (strcmp(vicodin, "IRTP/1.0") != 0) {
            recType = _INCOMPLETE;
          } else {
            client->username = username;
          }
          row = 0;
          client->processingMessage = "";
        } else if (strcmp(vicodin, "SEND") == 0) {
          recType = SEND; // And alexander wept, because he had no more worlds
                          // left to conquer
          channelName = escapedString(&spPtr, vicodin);

          vicodin = strtok_r(NULL, " ", &spPtr);
          if (strcmp(vicodin, "IRTP/1.0") != 0) {
            recType = _INCOMPLETE;
            client->processingMessage = "";
          }
        } else if (row == 0) {
          // Shit pant
          client->processingMessage = "";
        }
      }

      vicodin = strtok_r(NULL, " ", &spPtr);
      word++;
    }

    res = strtok_r(NULL, "\n", &nlPtr);
    row++;
  }
  return recType;
}

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

    if (poll(fds, numberOfClients, 20)) {
      *blockConnection = true;
      for (uint i = 0; i < numberOfClients; i++) {
        if (fds[i].revents & POLLIN) {
          char buffer[5121];
          if (recv(clients->at(i)->fd, buffer, 5120, 0) < 0) {
            clients->at(i)->active = false;
          } else {
            clients->at(i)->processingMessage.append(buffer);
            IRTPRec(clients, i);
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

  struct sigaction cleanExit = {&handleSigint, 0, 0, 0, 0};
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
