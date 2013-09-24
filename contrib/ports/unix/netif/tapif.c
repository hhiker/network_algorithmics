/*
 * Modified by: Shengjie Jiang <shengjie@mail.ustc.edu.cn>
 */

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "lwip/debug.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "netif/etharp.h"
#include "netif/tapif.h"

#if defined(LWIP_DEBUG) && defined(LWIP_TCPDUMP)
#include "netif/tcpdump.h"
#endif /* LWIP_DEBUG && LWIP_TCPDUMP */

#define IFCONFIG_BIN "/sbin/ifconfig "

#if defined(linux)
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#define DEVTAP "/dev/net/tun"
#define IFCONFIG_ARGS "tap0 inet %d.%d.%d.%d"
#elif defined(openbsd)
#define DEVTAP "/dev/tun0"
#define IFCONFIG_ARGS "tun0 inet %d.%d.%d.%d link0"
#else /* others */
#define DEVTAP "/dev/tap0"
#define IFCONFIG_ARGS "tap0 inet %d.%d.%d.%d"
#endif

#define FREQ (1024 * 1024) /* NOT Bigger than 1024 * 1024 * 2 */

#define IFNAME0 't'
#define IFNAME1 'p'

#ifndef TAPIF_DEBUG
#define TAPIF_DEBUG LWIP_DBG_OFF
#endif

struct tapif {
	struct eth_addr *ethaddr;
	/* Add whatever per-interface state that is needed here. */
	int fd;
};

static unsigned int rx_pkts;
static unsigned int rx_bytes;
static unsigned int tx_pkts;
static unsigned int tx_bytes;
static unsigned long long int rx_timer;
static unsigned long long int tx_timer;

/* Forward declarations. */
static void tapif_input(struct netif *netif);

static void tapif_thread(void *data);

/*-----------------------------------------------------------------------------------*/

static void low_level_init(struct netif *netif)
{
	struct tapif *tapif;
	char buf[sizeof(IFCONFIG_ARGS) + sizeof(IFCONFIG_BIN) + 50];

	tapif = (struct tapif *)netif->state;

	/* Obtain MAC address from network interface. */

	/* (We just fake an address...) */
	tapif->ethaddr->addr[0] = 0x1;
	tapif->ethaddr->addr[1] = 0x2;
	tapif->ethaddr->addr[2] = 0x3;
	tapif->ethaddr->addr[3] = 0x4;
	tapif->ethaddr->addr[4] = 0x5;
	tapif->ethaddr->addr[5] = 0x6;

	/* Do whatever else is needed to initialize interface. */

	tapif->fd = open(DEVTAP, O_RDWR);
	LWIP_DEBUGF(TAPIF_DEBUG, ("tapif_init: fd %d\n", tapif->fd));
	if (tapif->fd == -1) {
#ifdef linux
		perror("tapif_init: try running \"modprobe tun\" or rebuilding your kernel with CONFIG_TUN; cannot open "DEVTAP);
#else
		perror("tapif_init: cannot open "DEVTAP);
#endif
		exit(1);
	}

#ifdef linux
	{
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		ifr.ifr_flags = IFF_TAP|IFF_NO_PI;
		if (ioctl(tapif->fd, TUNSETIFF, (void *) &ifr) < 0) {
			perror("tapif_init: "DEVTAP" ioctl TUNSETIFF");
			exit(1);
		}
	}
#endif /* Linux */

	sprintf(buf, IFCONFIG_BIN IFCONFIG_ARGS, ip4_addr1(&(netif->gw)), ip4_addr2(&(netif->gw)),
	        ip4_addr3(&(netif->gw)), ip4_addr4(&(netif->gw)));

	LWIP_DEBUGF(TAPIF_DEBUG, ("tapif_init: system(\"%s\");\n", buf));
	system(buf);
	sys_thread_new("tapif_thread", tapif_thread, netif, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}

/*-----------------------------------------------------------------------------------*/
/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */
/*-----------------------------------------------------------------------------------*/

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
	char buf[1514];
	char *bufptr;
	struct tapif *tapif;
	unsigned short int len;

	tapif = (struct tapif *)netif->state;

	bufptr = &buf[0];

	for (q = p; q != NULL; q = q->next) {
		memcpy(bufptr, q->payload, q->len);
		bufptr += q->len;
	}

	len = p->tot_len;

	/* signal that packet should be sent(); */
	if (write(tapif->fd, buf, p->tot_len) != -1) {
		tx_pkts++;
		tx_bytes += (unsigned int)len;
		if ((tx_pkts & (FREQ - 1)) == (FREQ - 1)) {
			struct timeval current;
			unsigned long long int tmp, diff;

			gettimeofday(&current, NULL);
			tmp = current.tv_sec * 1000000 + current.tv_usec;
			diff = tmp - tx_timer;

			assert(diff > 0);

			printf("------   [ %10u Packtes ] [ %12u Bytes ] [ %13.6f Mpps ] [ %13.6f Mbps ] [ %12u Bytes ]\n",
				tx_pkts, tx_bytes, tx_pkts * 1.0 / diff, tx_bytes * 8.0 /diff, tx_bytes / tx_pkts);

			tx_pkts = 0;
			tx_bytes = 0;
			tx_timer = tmp;
		}
	} else { 
		printf("low_level_output: write() failed.\n");
	}

	return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
/*-----------------------------------------------------------------------------------*/

static struct pbuf *low_level_input(struct tapif *tapif)
{
	struct pbuf *p, *q;
	u16_t len;
	char buf[1514];
	char *bufptr;

	/* Obtain the size of the packet and put it into the "len" variable. */
	len = read(tapif->fd, buf, sizeof(buf));

	/* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

	if (p != NULL) {
		/* We iterate over the pbuf chain until we have read the entire packet into the pbuf. */
		bufptr = &buf[0];
		for (q = p; q != NULL; q = q->next) {
			memcpy(q->payload, bufptr, q->len);
			bufptr += q->len;
		}
	} else {
		/* drop packet(); */
		printf("low_level_input: pbuf_alloc() failed.\n");
	}

	return p;
}

/*-----------------------------------------------------------------------------------*/

static void tapif_thread(void *arg)
{
	int ret;
	struct netif *netif;
	struct tapif *tapif;
	fd_set fdset;

	netif = (struct netif *)arg;
	tapif = (struct tapif *)netif->state;

	while (1) {
		FD_ZERO(&fdset);
		FD_SET(tapif->fd, &fdset);

		/* Wait for a packet to arrive. */
		ret = select(tapif->fd + 1, &fdset, NULL, NULL, NULL);

		if (ret == 1) {
			/* Handle incoming packet. */
			tapif_input(netif);
		} else if (ret == -1) {
			perror("tapif_thread: select");
		}
	}
}

/*-----------------------------------------------------------------------------------*/
/*
 * tapif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */
/*-----------------------------------------------------------------------------------*/

static void tapif_input(struct netif *netif)
{
	unsigned short int len;
	static int disp;
	struct tapif *tapif;
	struct eth_hdr *ethhdr;
	struct pbuf *p;

	tapif = (struct tapif *)netif->state;

	p = low_level_input(tapif);

	if (p == NULL) {
		LWIP_DEBUGF(TAPIF_DEBUG, ("tapif_input: low_level_input returned NULL\n"));
		return;
	}

	len = p->tot_len;
	ethhdr = (struct eth_hdr *)p->payload;

	switch (htons(ethhdr->type)) {
		/* IP or ARP packet? */
		case ETHTYPE_IP:
		case ETHTYPE_ARP:
#if PPPOE_SUPPORT
		/* PPPoE packet? */
		case ETHTYPE_PPPOEDISC:
		case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
			/* full packet send to tcpip_thread to process */
			if (netif->input(p, netif) == ERR_OK) {
				rx_pkts++;
				rx_bytes += (unsigned int)len;
				if ((rx_pkts & (FREQ - 1)) == (FREQ - 1)) {
					struct timeval current;
					unsigned long long int tmp, diff;

					gettimeofday(&current, NULL);
					tmp = current.tv_sec * 1000000 + current.tv_usec;
					diff = tmp - rx_timer;

					assert(diff > 0);

					printf("++++++   [ %10u Packtes ] [ %12u Bytes ] [ %13.6f Mpps ] [ %13.6f Mbps ] [ %12u Bytes ]\n",
						rx_pkts, rx_bytes, rx_pkts * 1.0 / diff, rx_bytes * 8.0 /diff, rx_bytes / rx_pkts);

					rx_pkts = 0;
					rx_bytes = 0;
					rx_timer = tmp;
				}
			} else {
				if (disp == 0) {
					printf("tapif_input: netif->input() failed.\n");
					disp = 1;
				}
				pbuf_free(p);
				p = NULL;
			}
			break;
		default:
			pbuf_free(p);
			break;
	}
}

/*-----------------------------------------------------------------------------------*/
/*
 * tapif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */
/*-----------------------------------------------------------------------------------*/

err_t tapif_init(struct netif *netif)
{
	struct tapif *tapif;
	struct timeval current;

	tapif = (struct tapif *)mem_malloc(sizeof(struct tapif));
	if (tapif == NULL) {
		return ERR_MEM;
	}
	netif->state = tapif;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = etharp_output;
	netif->linkoutput = low_level_output;
	netif->mtu = 1500;
	/* hardware address length */
	netif->hwaddr_len = 6;

	tapif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

	rx_pkts = 0;
	rx_bytes = 0;
	tx_pkts = 0;
	tx_bytes = 0;

	gettimeofday(&current, NULL);
	tx_timer = rx_timer = current.tv_sec * 1000000 + current.tv_usec;

	low_level_init(netif);

	return ERR_OK;
}

