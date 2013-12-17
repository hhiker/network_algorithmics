/*
 * Modified by: Shengjie Jiang <shengjie@mail.ustc.edu.cn>
 */


#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


/*
   -----------------------------------------------
   ---------- Platform specific locking ----------
   -----------------------------------------------
*/

#define SYS_LIGHTWEIGHT_PROT            1

#define NO_SYS                          0


/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/

/* Approch 1 : Using LibC All. */
#define MEM_LIBC_MALLOC                 1 /* default 0 */
#define MEMP_MEM_MALLOC                 1 /* default 0 */

/* Approch 2 : Using Pools Instead of Heap. */
#define MEM_USE_POOLS                   0 /* default 0 */
#define MEMP_USE_CUSTOM_POOLS           0 /* default 0 */
#define MEM_USE_POOLS_TRY_BIGGER_POOL   0 /* default 0 */

/* Approch 3 : Using Heap. (Default) */

#define MEM_ALIGNMENT                   4

#define MEM_SIZE                        (8 * 1024 * 1024)


/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/

#define MEMP_NUM_PBUF                   8192

#define MEMP_NUM_RAW_PCB                0

#define MEMP_NUM_UDP_PCB                128

#define MEMP_NUM_TCP_PCB                4194304

#define MEMP_NUM_TCP_PCB_LISTEN         16

#define MEMP_NUM_TCP_SEG                8192

#define MEMP_NUM_REASSDATA              32

#define MEMP_NUM_ARP_QUEUE              2

#define MEMP_NUM_SYS_TIMEOUT            256

#define MEMP_NUM_NETBUF                 8192

#define MEMP_NUM_NETCONN                8192

#define MEMP_NUM_TCPIP_MSG_API          2048

#define MEMP_NUM_TCPIP_MSG_INPKT        2048

#define MEMP_NUM_TCP_TIMER              (MEMP_NUM_TCP_PCB)

#define PBUF_POOL_SIZE                  8192


/*
   ---------------------------------
   ---------- ARP options ----------
   ---------------------------------
*/

#define LWIP_ARP                        1


/*
   --------------------------------
   ---------- IP options ----------
   --------------------------------
*/

#define IP_FORWARD                      0

#define IP_OPTIONS_ALLOWED              1

#define IP_REASSEMBLY                   1

#define IP_FRAG                         1

#define IP_REASS_MAXAGE                 3

#define IP_REASS_MAX_PBUFS              128

#define IP_FRAG_USES_STATIC_BUF         0

#define IP_DEFAULT_TTL                  255


/*
   ----------------------------------
   ---------- ICMP options ----------
   ----------------------------------
*/

#define LWIP_ICMP                       1


/*
   ---------------------------------
   ---------- RAW options ----------
   ---------------------------------
*/

#define LWIP_RAW                        0


/*
   ----------------------------------
   ---------- DHCP options ----------
   ----------------------------------
*/

#define LWIP_DHCP                       0


/*
   ------------------------------------
   ---------- AUTOIP options ----------
   ------------------------------------
*/

#define LWIP_AUTOIP                     0


/*
   ----------------------------------
   ---------- SNMP options ----------
   ----------------------------------
*/

#define LWIP_SNMP                       0


/*
   ----------------------------------
   ---------- IGMP options ----------
   ----------------------------------
*/

#define LWIP_IGMP                       0


/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/

#define LWIP_DNS                        0


/*
   ---------------------------------
   ---------- UDP options ----------
   ---------------------------------
*/

#define LWIP_UDP                        1


/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/

#define LWIP_TCP                        1

#define TCP_MSS                         1460 /* default 536 */

#define TCP_WND                         0xEFFF /* default (4 * TCP_MSS) */

#define TCP_SND_BUF                     TCP_WND /* default (2 * TCP_MSS) */

#define TCP_SND_QUEUELEN                512 /* default ((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS)) */

#define TCP_SNDLOWAT                    LWIP_MIN(LWIP_MAX(((TCP_SND_BUF)/2), (2*TCP_MSS)+1), (TCP_SND_BUF)-1)

#define TCP_SNDQUEUELOWAT               LWIP_MAX(((TCP_SND_QUEUELEN)/2), 5)

#define TCP_WND_UPDATE_THRESHOLD        (TCP_WND / 8) /* default (TCP_WND / 4) */

#define TCP_MAXRTX                      12

#define TCP_SYNMAXRTX                   6

#define LWIP_LISTEN_BACKLOG             0


/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/

#define PBUF_LINK_HLEN                  16

#define PBUF_POOL_BUFSIZE               LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_HLEN)


/*
   ------------------------------------
   ---------- LOOPIF options ----------
   ------------------------------------
*/

#define LWIP_HAVE_LOOPIF                0


/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/

#define LWIP_NETCONN                    1

#define LWIP_TCPIP_TIMEOUT              0


/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/

#define LWIP_SOCKET                     1


/*
   ----------------------------------------
   ---------- Statistics options ----------
   ----------------------------------------
*/

#define LWIP_STATS                      0


/*
   ---------------------------------
   ---------- PPP options ----------
   ---------------------------------
*/

#define PPP_SUPPORT                     0


/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

#define LWIP_CHECKSUM_ON_COPY           0

/* 
 * PCB hash-table search option.
 * define 0: apply pcb hash table
 * define 1: debug pcb hash table
 * no define: don't use pcb hash table
 */
#define TCP_PCB_HASH			0

#endif /* __LWIPOPTS_H__ */
