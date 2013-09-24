#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ADDRESS "192.168.1.2"
#define PORT 20102

char usage[] = "\
Options:                                              \n\
    -d    Delay.                                    0 \n\
    -l    Length of Data Buffer.                 4096 \n\
    -n    Loop Times.                           40960 \n\
    -p    Number of Threads.                        1 \n\
    -s    Number of Sockets in Each Thread.         1 \n\
    -t    Type:                                     1 \n\
          1   TCP                                     \n\
          9   UDP                                     \n\
";

int d = 0;                        /* Delay. */
int l = 4096;                     /* Length of Data Buffer. */
int n = 40960;                    /* Loop Times. */
int p = 1;                        /* Number of Threads. */
int s = 1;                        /* Number of Sockets in Each Thread. */
int t = 1;                        /* Type */

int connected[9999] = {0};        /* Connected Connections. */

/*----------------------------------------------------------------------------*/

void *tcp_client_thread(void *arg)
{
	int i, id, ret, left;
	int c_sockfd[s];
	long long int num[s], total[s];
	struct sockaddr_in s_addr;
	char buffer[l];

	id = (int)(long int)(arg);
	left = s;

	for (i = 0; i < s; ++i) {
		num[i] = 0;
		total[i] = ((long long int)l) * n;
	}

	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = inet_addr(ADDRESS);
	s_addr.sin_port = htons(PORT);

	for (i = 0; i < s; i++) {
		c_sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (c_sockfd[i] < 0) {
			printf("socket() for connection [ %04d.%03d ] failed.\n", id, i);
			num[i] = total[i];
			left--;
			continue;
		}

		ret = connect(c_sockfd[i], (struct sockaddr *)&s_addr, sizeof(s_addr));
		if (ret < 0) {
			printf ("connect() for connection [ %04d.%03d ] failed.\n", id, i);
			close(c_sockfd[i]);
			num[i] = total[i];
			left--;
			continue;
		}

		printf("++++++   [ %04d.%03d ]\n", id, i);
	}

	connected[id] = left;

	do {
		for (i = 0; i < s; ++i) {
			if (num[i] < total[i]) {
				ret = write(c_sockfd[i], buffer, l);
				if (ret > 0)
					num[i] += ret;
				if (num[i] >= total[i]) {
					close(c_sockfd[i]);
					left--;
					printf("------   [ %04d.%03d ]\n", id, i);
				}
			}
		}
	} while (left);

	pthread_exit(NULL);
}

/*----------------------------------------------------------------------------*/

inline uint64_t read_tsc()
{
	uint64_t time;
	uint32_t msw, lsw;

	__asm__  __volatile__(       \
		"rdtsc               \n\t"
		"movl %%edx, %0      \n\t"
		"movl %%eax, %1      \n\t"
		:"=r"(msw), "=r"(lsw)
		:
		:"%edx", "%eax");

	time = ((uint64_t) msw << 32) | lsw;
	return time;
}

inline void wait_ticks(uint64_t ticks)
{
	uint64_t current_time;
	uint64_t time = read_tsc();

	time += ticks;
	do {
		current_time = read_tsc();
	} while (current_time < time);
}

void *udp_client_thread(void *arg)
{
	int i, id, ret, left;
	int c_sockfd[s];
	long long int num[s], total[s];
	struct sockaddr_in s_addr;
	char buffer[l];

	id = (int)(long int)(arg);
	left = s;

	for (i = 0; i < s; ++i) {
		num[i] = 0;
		total[i] = ((long long int)l) * n;
	}

	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = inet_addr(ADDRESS);
	s_addr.sin_port = htons(PORT);

	for (i = 0; i < s; i++) {
		c_sockfd[i] = socket(AF_INET, SOCK_DGRAM, 0);
		if (c_sockfd[i] < 0) {
			printf("socket() for connection [ %04d.%03d ] failed.\n", id, i);
			num[i] = total[i];
			left--;
			continue;
		}

		printf("++++++   [ %04d.%03d ]\n", id, i);
	}

	connected[id] = left;

	do {
		for (i = 0; i < s; ++i) {
			if (num[i] < total[i]) {
				wait_ticks((uint64_t )d); /* Delay */
				ret = sendto(c_sockfd[i], buffer, l, 0, (struct sockaddr *)&s_addr, sizeof(s_addr));
				if (ret == l)
					num[i] += ret;
				if (num[i] >= total[i]) {
					close(c_sockfd[i]);
					left--;
					printf("------   [ %04d.%03d ]\n", id, i);
				}
			}
		}
	} while (left);

	pthread_exit(NULL);
}

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	int c, i, total_conns;
	struct timeval starttime, finishtime, diff_time;

	while ((c = getopt(argc, argv, ":d:l:n:p:s:t:")) != -1) {
		switch (c) {
			case 'd':
				d = atoi(optarg);
				if (d <= 0) {
					printf("Invalid Delay: %d.\n", d);
					return 1;
				}
				break;
			case 'l':
				l = atoi(optarg);
				if (l <= 0) {
					printf("Invalid Length of Data Buffer: %d.\n", l);
					return 1;
				}
				break;
			case 'n':
				n = atoi(optarg);
				if (n <= 0) {
					printf("Invalid Loop Times: %d.\n", n);
					return 1;
				}
				break;
			case 'p':
				p = atoi(optarg);
				if (p <= 0 || p > 9999) {
					printf("Invalid Number of Threads: %d.\n", p);
					if (p > 9999) {
						printf("p > 9999.\n");
					}
					return 1;
				}
				break;
			case 's':
				s = atoi(optarg);
				if (s <= 0 || s > 999) {
					printf("Invalid Number of Sockets in Each Thread: %d.\n", s);
					if (s > 999) {
						printf("s > 999.\n");
					}
					return 1;
				}
				break;
			case 't':
				t = atoi(optarg);
				if (t != 1 && t != 9) {
					printf("Invalid Type: %d.\n", t);
					return 1;
				}
				break;
			default:
				printf("Invalid Option!\n");
				goto USAGE;
				break;
		}
	}

	if (p * s >= 65536) {
		printf("No Enough Ports for %d Connections.\n", p * s);
		return 1;
	}

	gettimeofday(&starttime, NULL);
	if (t == 1) {
		int ret;
		pthread_t threads[p];

		printf("--- l = %d, n = %d, p = %d, s = %d, t = %d ---\n\n", l, n, p, s, t);

		for (i = 0; i < p; ++i) {
			ret = pthread_create(&threads[i], NULL, tcp_client_thread, (void *)(long int)(i));
			if (ret != 0) {
				printf("pthread_create() for tcp_client_thread [ %d ] failed.\n", i);
				threads[i] = -1;
				connected[i] = 0;
			}
		}

		for (i = 0; i < p; ++i) {
			if (threads[i] == -1) {
				continue;
			}

			ret = pthread_join(threads[i], NULL);
			if (ret != 0) {
				printf("pthread_join() for tcp_client_thread [ %d ] failed.\n", i);
				return 1;
			}
		}
	} else { /* t == 9 */
		int ret;
		pthread_t threads[p];

		printf("--- d = %d, l = %d, n = %d, p = %d, s = %d, t = %d ---\n\n", d, l, n, p, s, t);

		for (i = 0; i < p; ++i) {
			ret = pthread_create(&threads[i], NULL, udp_client_thread, (void *)(long int)(i));
			if (ret != 0) {
				printf("pthread_create() for udp_client_thread [ %d ] failed.\n", i);
				threads[i] = -1;
				connected[i] = 0;
			}
		}

		for (i = 0; i < p; ++i) {
			if (threads[i] == -1) {
				continue;
			}

			ret = pthread_join(threads[i], NULL);
			if (ret != 0) {
				printf("pthread_join() for udp_client_thread [ %d ] failed.\n", i);
				return 1;
			}
		}
	}
	gettimeofday(&finishtime, NULL);

	total_conns = 0;
	for (i = 0; i < p; ++i) {
		total_conns += connected[i];
	}

	timersub(&finishtime, &starttime, &diff_time);
	printf("\n");
	printf("Time Cost     : %5ld.%06ld   Seconds\n", diff_time.tv_sec, diff_time.tv_usec);
	printf("Total Conn.   : %12d   Connections.\n", total_conns);
	printf("Total Bytes   : %12lld   MBytes.\n", (((long long int)l) * n * total_conns) >> 20);
	printf("B.W.          : %12.3f   Mbps.\n", ((long long int)l) * n * total_conns * 8.0 / (diff_time.tv_sec * 1000000 + diff_time.tv_usec));

	return 0;

USAGE:
	printf("%s", usage);
	return 1;
}
