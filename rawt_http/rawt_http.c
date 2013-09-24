#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/api_msg.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static char indexdata[] =
"HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: 1024\r\n\
\r\n\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\
";

static struct tcp_pcb *rawt_http_pcb;

void rawt_http_err(void *arg, err_t err)
{
	printf("Error occured: err = %d.\n", err);
}

err_t rawt_http_poll(void *arg, struct tcp_pcb *tpcb)
{
	return ERR_OK;
}

void rawt_http_close(struct tcp_pcb *tpcb)
{
	tcp_arg(tpcb, NULL);
	tcp_accept(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);
	tcp_recv(tpcb, NULL);
	tcp_sent(tpcb, NULL);

	tcp_close(tpcb);
}

err_t rawt_http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	if (p == NULL) {
		rawt_http_close(tpcb);
	} else if (err != ERR_OK) {
		if (p != NULL) {
			pbuf_free(p);
		}
	} else {
		char *rq;

		rq = p->payload;

		if (rq[0] == 'G' && rq[1] == 'E'  &&
		    rq[2] == 'T' && rq[3] == ' '  &&
		    rq[4] == '/') {
			tcp_write(tpcb, indexdata, sizeof(indexdata), 0);
		}

		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p); /* free() after tcp_recved() */
	}

	rawt_http_close(tpcb);

	return ERR_OK;
}

err_t rawt_http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	return ERR_OK;
}

err_t rawt_http_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	/* Should NOT call tcp_accepted()! */
	tcp_arg(newpcb, NULL);
	tcp_err(newpcb, rawt_http_err);
	tcp_poll(newpcb, rawt_http_poll, 0);
	tcp_recv(newpcb, rawt_http_recv);
	tcp_sent(newpcb, rawt_http_sent);

	return ERR_OK;
}

void rawt_http_init(struct api_msg_msg *msg)
{
	err_t err;
	ip_addr_t ipaddr;

	rawt_http_pcb = tcp_new();
	if (rawt_http_pcb == NULL) {
		printf("rawt_http_init: tcp_new() failed.\n");
		exit(1);
	}

	IP4_ADDR(&ipaddr, 192, 168, 1, 2);
	err = tcp_bind(rawt_http_pcb, &ipaddr, 20102);
	if (err != ERR_OK) {
		printf("rawt_http_init: tcp_bind() failed.\n");
		exit(1);
	}

	rawt_http_pcb = tcp_listen(rawt_http_pcb);
	if (rawt_http_pcb == NULL) {
		printf("rawt_http_init: tcp_listen() failed.\n");
		exit(1);
	}

	tcp_accept(rawt_http_pcb, rawt_http_accept); /* only accept callback is valid! */

	TCPIP_APIMSG_ACK(msg);
}

void deal_signal(int sig)
{
	exit(0);
}

int main(void)
{
	struct netconn *conn;
	struct api_msg msg;

	conn = netconn_alloc(NETCONN_TCP, NULL);
	if (conn != NULL) {
		msg.function = rawt_http_init;
		msg.msg.conn = conn;

		if (TCPIP_APIMSG(&msg) != ERR_OK) {
			sys_sem_free(&conn->op_completed);
			sys_mbox_free(&conn->recvmbox);
			memp_free(MEMP_NETCONN, conn);

			printf("TCPIP_APIMSG() failed.\n");
			exit(1);
		}
	}

	(void)signal(SIGINT, deal_signal);

	sleep(10000);

	return 0;
}
