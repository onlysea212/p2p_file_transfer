#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "../common.h"
#include "../sockio.h"
#include "handle_request.h"


void *serveClient(void *arg);

int main(int argc, char **argv){
	if (argc > 1){
		if (!strcmp(argv[1], "--debug"))
			stream = stderr;
		else{
			printf("unkown option: %s\n", argv[1]);
			exit(1);
		}
	}else
		stream = fopen("/dev/null", "w");

	int servsock = socket(AF_INET, SOCK_STREAM, 0);
	if (servsock < 0){
		perror("socket");
		return 1;
	}

	int on = 1;
	setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	struct sockaddr_in servsin;
	bzero(&servsin, sizeof(servsin));
	servsin.sin_addr.s_addr = htonl(INADDR_ANY);
	servsin.sin_family = AF_INET;
	servsin.sin_port = htons(6969);

	if ((bind(servsock, (struct sockaddr*) &servsin, sizeof(servsin))) < 0){
		perror("bind");
		return 1;
	}

	if ((listen(servsock, 10)) < 0){
		perror("listen");
		return 1;
	}

	struct net_info *cli_info = malloc(sizeof(struct net_info));
	cli_info->data_port = 0;
	struct sockaddr_in clisin;
	unsigned int sin_len = sizeof(clisin);
	bzero(&clisin, sizeof(clisin));

	/* ignore EPIPE to avoid crashing when writing to a closed socket */
	signal (SIGPIPE, SIG_IGN);

	/* serve multiple clients simultaneously */
	while (1){
		cli_info->sockfd = accept(servsock, (struct sockaddr*) &clisin, &sin_len);
		if (cli_info->sockfd < 0){
			perror("accept connection");
			continue;
		}

		inet_ntop(AF_INET, &(clisin.sin_addr), cli_info->ip_add, sizeof(cli_info->ip_add));
		cli_info->port = ntohs(clisin.sin_port);

		/* create a new thread for each client */
		pthread_t tid;
		int thr = pthread_create(&tid, NULL, &serveClient, (void*)cli_info);
		if (thr != 0){
			char err_mess[255];
			strerror_r(errno, err_mess, sizeof(err_mess));
			fprintf(stream, "create thread to handle %s:%u: %s\n", 
					cli_info->ip_add, cli_info->port, err_mess);
			close(cli_info->sockfd);
			continue;
		}
		cli_info = malloc(sizeof(struct net_info));
		cli_info->data_port = 0;
	}
	close(servsock);
}

void *serveClient(void *arg){
	pthread_detach(pthread_self());
	struct net_info cli_info = *((struct net_info*) arg);
	free(arg);
	char cli_addr[256];
	sprintf(cli_addr, "%s:%u", cli_info.ip_add, cli_info.port);

	printf("connection from client: %s\n", cli_addr);
	uint8_t packet_type;

	/* receive request from clients,
	 * then response accordingly */
	//read packet type first
	while(readBytes(cli_info.sockfd, &packet_type, sizeof(packet_type)) > 0){
		if (packet_type == DATA_PORT_ANNOUNCEMENT){
			if (cli_info.data_port != 0){
				/* only accept DATA_PORT_ANNOUNCEMENT packet once */
				fprintf(stdout, "%s > only accept DATA_PORT_ANNOUNCEMENT packet once\n", 
						cli_addr);
				fprintf(stdout, "removing %s from the file list due to data port violation\n",
						cli_addr);
				struct DataHost host;
				host.ip_addr = inet_addr(cli_info.ip_add);
				host.port = cli_info.data_port;
				removeHost(host);
				close(cli_info.sockfd);
				int ret = 100;
				pthread_exit(&ret);
			}
			uint16_t data_port;
			int n_bytes = readBytes(cli_info.sockfd, &data_port, sizeof(data_port));
			if (n_bytes <= 0)
				handleSocketError(cli_info, "read from socket");
			fprintf(stdout, "%s > dataPort: %u\n", cli_addr, ntohs(data_port));
			cli_info.data_port = ntohs(data_port);
		} else if (packet_type == FILE_LIST_UPDATE){

		} else if (packet_type == LIST_FILES_REQUEST){
			//do something
		} else if (packet_type == LIST_HOSTS_REQUEST){
			//do something
		}
	}
	handleSocketError(cli_info, "read from socket");
	return NULL;
}
