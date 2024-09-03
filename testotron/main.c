#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char** argv) {
	if (argc == 1) {
		printf("nuh uh\n");
		return 1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in address = {
		AF_INET,
		htons(6750),
		0
	};

	int _ = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
	
	char buffer[256] = { 0 };

	if (!strcmp(argv[1], "0")) {
		strcpy(buffer, "IDENT illegitimate-egg MEGATRON/1.0");
	}

	send(sockfd, buffer, 255, 0);
	return 0;
}
