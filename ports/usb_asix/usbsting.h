/*
 * usbsting.h: header for STiNG USB Ethernet driver (w/ASIX chipset)
 *
 * Copyright Roger Burrows (June 2018), based on unpublished SCSILINK code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _USBSTING_H
#define _USBSTING_H

#ifndef NULL
# define NULL   ((void *)0)
#endif

/*
 *  GCC compatibility stuff
 */
#ifdef __PUREC__
#else
#include "mint/basepage.h"
#define BASPAG    BASEPAGE
#endif

#include "transprt.h"
#include "port.h"

typedef unsigned char uchar;

/*
 *  Ethernet architectural stuff
 */
#define ETH_ALEN        6           /* HW addr length */
#define ETH_HLEN        14          /* frame header length */
#define ETH_MIN_DLEN    46          /* minimum data length */
#define ETH_MAX_DLEN    1500        /* maximum data length */

typedef struct {                /* packet header */
     char destination[ETH_ALEN];    /* Destination hardware address */
     char source[ETH_ALEN];         /* Source hardware address */
     uint16 type;                   /* Ethernet protocol type */
} ENET_HDR;
#define ENET_TYPE_IP    0x0800
#define ENET_TYPE_ARP   0x0806
#define ENET_TYPE_RARP  0x8035      /* not used by us afaik */

typedef struct {                /* ARP packet contents */
     uint16 hardware_space;         /* Hardware address space identifier */
     uint16 protocol_space;         /* Protocol address space identifier */
     char   hardware_len;           /* Length of hardware address */
     char   protocol_len;           /* Length of protocol address */
     uint16 op_code;                /* Operation Code */
     char   src_ether[ETH_ALEN];    /* Sender's hardware address */
     uint32 src_ip;                 /* Sender's protocol address */
     char   dest_ether[ETH_ALEN];   /* Target's hardware address */
     uint32 dest_ip;                /* Target's protocol address */
} ARP;
#define ARP_HARD_ETHER  1
#define ARP_OP_REQ      1
#define ARP_OP_ANS      2

typedef struct {                /* generic IP ethernet packet */
    ENET_HDR eh;
    char ed[ETH_MAX_DLEN];
} ENET_PACKET;
#define ETH_MIN_LEN     (ETH_HLEN+ETH_MIN_DLEN)
#define ETH_MAX_LEN     sizeof(ENET_PACKET)

typedef struct {                /* ARP ethernet packet */
    ENET_HDR eh;
    ARP      arp;
    char     padbytes[ETH_MIN_LEN-sizeof(ENET_HDR)-sizeof(ARP)];
} ARP_PACKET;


/*
 *  manifest constants
 */
#define hz_200          (*(unsigned long *) 0x4baL)
#define _p_cookie       0x5a0L

#define FRB_COOKIE      0x5f465242L     /* '_FRB' */
#define STING_COOKIE    0x5354694bL     /* 'STiK' */
#define USB_COOKIE      0x5f555342L     /* '_USB' */

#ifndef ETH_ALEN
# define ETH_ALEN       6
#endif


/*
 *    driver-specific stuff
 */
#define BASE_PORTNAME    "USBether"

typedef struct
{
    unsigned char hwaddr[ETH_ALEN]; /* default MAC address */
    unsigned char macaddr[ETH_ALEN];/* current MAC address */
    long arp_entries;               /* number of active entries in ARP cache */
    long trace_entries;             /* number of entries in trace table */
    struct
    {
        long total_packets;
        long failed;
    } read;
    struct
    {
        long total_packets;
        long good_packets;
        long bad_packets;
    } receive;
    struct
    {
        long broadcast_ip_packets;
        long normal_ip_packets;
        long arp_packets;
        long bad_ip_packets;
        long bad_arp_packets;
    } process;
    struct
    {
        long total_packets;
        long failed;                /* write_device() returned error */
    } write;
    struct
    {
        long dequeued;
        long bad_length;
        long bad_host;
        long bad_network;
        long ip_packets;
        long arp_packets;
        long arp_packets_err;
    } send;
    struct
    {
        long input_errors;          /* ARP dgram counts */
        long opcode_errors;
        long requests_received;
        long answers_received;
        long wait_queued;           /* normal dgrams queued waiting for ARP */
        long wait_dequeued;         /* dequeued */
        long wait_requeued;         /* requeued */
    } arp;
} ASIX_STATS;

#define ASIX_TRACE_LEN   52
typedef struct
{
    unsigned long time;
    long rc;
    char type;
#define TRACE_MAC_GET       'G'
#define TRACE_READ          'R'
#define TRACE_WRITE         'W'
    char reserved;
    short length;
    uchar data[ASIX_TRACE_LEN];
} ASIX_TRACE;

typedef struct                  /* data returned by CTL_ETHER_GET_ARPTABLE */
{
    uint32 ip_addr;                 /* IP address */
    unsigned char ether[ETH_ALEN];  /* EtherNet station address */
} ARP_INFO;

/*
 *    additional function codes for cntrl_port()
 */
#define CTL_ETHER_GET_STAT      (('E' << 8) | 'S')  /* returns a copy of the struct driver_statistics */
#define CTL_ETHER_CLR_STAT      (('E' << 8) | 'C')  /* sets all entries in struct driver_statistics to 0 */
#define CTL_ETHER_GET_ARP       (('E' << 8) | 'A')  /* gets ARP table */
#define CTL_ETHER_CLR_ARP       (('E' << 8) | 'B')  /* clears ARP table */
#define CTL_ETHER_GET_TRACE     (('E' << 8) | 'X')  /* gets trace table */
#define CTL_ETHER_CLR_TRACE     (('E' << 8) | 'Y')  /* clears trace table */

#endif
