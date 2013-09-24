#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef LWIP
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/select.h>
	#define ADDRESS "192.168.1.2"
#else
	#include "sys/socket.h"
	#define ADDRESS "192.168.1.2"
#endif

#define ANSI_BOLD  "\033[1m"
#define ANSI_BLUE  "\033[34m"
#define ANSI_GREEN "\033[32m"
#define ANSI_RESET "\033[0m"

#define SERVER_PORT    20102
#define READ_BUF_SIZE  65536
#define MAX_EPOLL_SIZE 1024

#define FREQ (1024 * 128) /* NOT Bigger than 1024 * 1024 * 2 */

static unsigned int rx_pkts;
static unsigned int rx_bytes;
static unsigned long long int rx_timer;

/*----------------------------------------------------------------------------*/

void deal_signal(int sig)
{
	exit(0);
}

int main(void)
{
	int i, ret, server_sock, client_sock, nready, fd_max;
	struct sockaddr_in server_addr;
	char buffer[READ_BUF_SIZE];
	fd_set bfds, rfds;

	struct timeval current;

	(void)signal(SIGINT, deal_signal);

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ADDRESS);
	server_addr.sin_port = htons(SERVER_PORT);

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0) {
		printf("socket() for select server failed.\n");
		return 1;
	}

	ret = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (ret < 0) {
		printf("bind() for select server failed.\n");
		close(server_sock);
		return 1;
	}

	ret = listen(server_sock, 32);
	if (ret != 0) {
		printf("listen() for select server failed.\n");
		close(server_sock);
		return 1;
	}

	FD_ZERO(&bfds);
	FD_SET(server_sock, &bfds);
	fd_max = server_sock;

	rx_pkts = 0;
	rx_bytes = 0;
	gettimeofday(&current, NULL);
	rx_timer = current.tv_sec * 1000000 + current.tv_usec;

	printf("select server starting ...\n\n");

	while (1) {
		memcpy(&rfds, &bfds, sizeof(fd_set));
		nready = select(fd_max + 1, &rfds, NULL, NULL, NULL);

		if (nready <= 0) {
			continue;
		}

		for (i = 0; i < fd_max + 1 && nready; ++i) {
			if (FD_ISSET(i, &rfds)) {
				nready--;
				if (i == server_sock) {
					client_sock = accept(server_sock, NULL, NULL);
					if (client_sock < 0) {
						printf("accept() for select server failed.\n");
						continue;
					}

					FD_SET(client_sock, &bfds);
					if (fd_max < client_sock) {
						fd_max = client_sock;
					}
				} else {
					int nread = read(i, buffer, READ_BUF_SIZE);

					if (nread > 0) {
						rx_pkts++;
						rx_bytes += (unsigned int)nread;

						if ((rx_pkts & (FREQ - 1)) == (FREQ - 1)) {
							unsigned long long int tmp, diff;

							gettimeofday(&current, NULL);
							tmp = current.tv_sec * 1000000 + current.tv_usec;
							diff = tmp - rx_timer;

							assert(diff > 0);

							printf("%s%s         [ %10u Packtes ] [ %12u Bytes ] [ %13.6f Mpps ] [ %13.6f Mbps ] [ %12u Bytes ]%s\n",
								ANSI_BOLD, ANSI_GREEN, rx_pkts, rx_bytes, rx_pkts * 1.0 / diff, rx_bytes * 8.0 /diff, rx_bytes / rx_pkts, ANSI_RESET);

							rx_pkts = 0;
							rx_bytes = 0;
							rx_timer = tmp;
						}
					} else if (nread < 0) {
						close(i);

						FD_CLR(i, &bfds);
						if (fd_max == i) {
							fd_max--;
						}
					}
				}
			}
		}
	}

	return 0;
}
