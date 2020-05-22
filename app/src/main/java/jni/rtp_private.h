/*
 * rtp_private.h
 *
 * Data structures used by RTP.
 *
 * Kedar B. Patil
 *
 * April, 2001
 *
 * Copyright (c) 2001-2003 HCL Technologies Ltd.
 *
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of HCL Technologies Ltd.
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code
 *
 */
/*
 * $Log: rtp_private.h,v $
 * Revision 1.2  2009/10/17 05:32:09  baojianyi
 * no message
 *
 * Revision 1.1.1.1  2007/04/06 03:47:12  baojianyi
 * no message
 *
 * Revision 1.1.1.1  2006/06/22 06:31:03  root
 * init version
 *
 * Revision 1.1  2006/03/24 07:53:53  chenzaiyong
 * no message
 *
 * Revision 1.8.8.1  2003/08/05 15:05:05  vcuddapa
 * Modified to support Agere platform
 *
 * Revision 1.8  2003/05/15 07:28:36  vcuddapa
 * Integrated with OSAPI, Added Windows support
 *
 * Revision 1.7  2003/03/03 11:37:35  vcuddapa
 * modified
 *
 * Revision 1.6  2003/02/18 12:49:35  vcuddapa
 * rtp support for mixer application is provided. Multicast support is added.
 *
 * $EndLog$
 */

#ifndef __RTP_PRIVATE_H__
#define __RTP_PRIVATE_H__

//#include "voip_oa_types.h"
#define DEBUG_RTP if (debug_rtp)

#ifndef ZERO
#define ZERO 0
#endif

#define RTP_MAX_SESS_NO 128  

#define RTP_CSRC_SIZE 4 /* bytes */
#define RTP_DOUBLE_WORD 4 /* bytes */
#define RTP_VERSION    2
#define RTP_SEQ_MOD (1<<16)
#define RTP_MAX_TRIES 5 /* Config Parameter */



typedef enum {
    RTCP_SDES_END   = 0,
    RTCP_SDES_CNAME = 1,
    RTCP_SDES_NAME  = 2,
    RTCP_SDES_EMAIL = 3,
    RTCP_SDES_PHONE = 4,
    RTCP_SDES_LOC   = 5,
    RTCP_SDES_TOOL  = 6,
    RTCP_SDES_NOTE  = 7,
    RTCP_SDES_PRIV  = 8
} e_rtcp_sdes_type_t;


#ifdef BIG_ENDIAN_ARCH

#if defined (UNDER_CE)
#pragma pack(push,1)
#endif

/* RTP data header */
typedef struct {
    UINT8_t version:2;   /* protocol version */
    UINT8_t p:1;         /* padding flag */
    UINT8_t x:1;         /* header extension flag */
    UINT8_t cc:4;        /* CSRC count */
    UINT8_t m:1;         /* marker bit */
    UINT8_t pt:7;        /* payload type */
    UINT16_t seq;             /* sequence number */
    UINT32_t ts;              /* timestamp */
    UINT32_t ssrc;            /* synchronization source */
    UINT32_t csrc[1];         /* optional CSRC list */
} rtp_hdr_t;

#if defined (UNDER_CE)
#pragma pack(pop)
#endif

#if defined (UNDER_CE)
#pragma pack(push,1)
#endif

/* RTCP common header word */
typedef struct {
    UINT8_t version:2;   /* protocol version */
    UINT8_t p:1;         /* padding flag */
    UINT8_t count:5;     /* varies by packet type */
    UINT8_t pt:8;        /* RTCP packet type */
    UINT16_t length;          /* pkt len in words, w/o this word */
} rtcp_common_t;

#if defined (UNDER_CE)
#pragma pack(pop)
#endif

/* Big-endian mask for version, padding bit and packet type pair */
#define RTCP_VALID_MASK (0xc000 | 0x2000 | 0xfe)
#define RTCP_VALID_VALUE ((RTP_VERSION << 14) | RTCP_SR)

/* Reception report block */
#if defined (UNDER_CE)
#pragma pack(push,1)
#endif

typedef struct {
    UINT32_t ssrc;            /* data source being reported */
    UINT32_t fraction:8;  /* fraction lost since last SR/RR */
    UINT32_t lost:24;              /* cumul. no. pkts lost (signed!) */
    UINT32_t last_seq;        /* extended last seq. no. received */
    UINT32_t jitter;          /* interarrival jitter */
    UINT32_t lsr;             /* last SR packet from this source */
    UINT32_t dlsr;            /* delay since last SR packet */
} rtcp_rr_t;

#if defined (UNDER_CE)
#pragma pack(pop)
#endif

#endif

#define LITTLE_ENDIAN_ARCH

#ifdef LITTLE_ENDIAN_ARCH

#if defined (UNDER_CE)
#pragma pack(push,1)
#endif

//BAOJIANYI 2009.09.28
#pragma pack(1)

/* Venkat modifing the structure */

typedef struct {
    UINT8_t cc:4;
    UINT8_t x:1;
    UINT8_t p:1;
    UINT8_t version:2;
    UINT8_t pt:7;
    UINT8_t m:1;
    UINT16_t seq;
    UINT32_t ts;
    UINT32_t ssrc;
    UINT32_t csrc[1];
} rtp_hdr_t;

#if defined (UNDER_CE)
#pragma pack(pop)
#endif

#if defined (UNDER_CE)
#pragma pack(push,1)
#endif

/* RTCP common header word */
typedef struct {
    UINT8_t pt:8;        /* RTCP packet type */
    UINT8_t count:5;     /* varies by packet type */
    UINT8_t p:1;         /* padding flag */
    UINT8_t version:2;   /* protocol version */
    UINT16_t length;          /* pkt len in words, w/o this word */
} rtcp_common_t;

#if defined (UNDER_CE)
#pragma pack(pop)
#endif

/* little-endian mask for version, padding bit and packet type pair */
#define RTCP_VALID_MASK (0x0003 | 0x04 | 0x7f00)
#define RTCP_VALID_VALUE (RTP_VERSION | RTCP_SR)

/* Reception report block */
#if defined (UNDER_CE)
#pragma pack(push,1)
#endif

typedef struct {
    UINT32_t ssrc;            /* data source being reported */
    int lost:24;              /* cumul. no. pkts lost (signed!) */
    unsigned int fraction:8;  /* fraction lost since last SR/RR */
    UINT32_t last_seq;        /* extended last seq. no. received */
    UINT32_t jitter;          /* interarrival jitter */
    UINT32_t lsr;             /* last SR packet from this source */
    UINT32_t dlsr;            /* delay since last SR packet */
} rtcp_rr_t;

#if defined (UNDER_CE)
#pragma pack(pop)
#endif

#endif

/* SDES item */
#if defined (UNDER_CE)
#pragma pack(push,1)
#endif

typedef struct {
    UINT8_t type;             /* type of item (e_rtcp_sdes_type_t) */
    UINT8_t length;           /* length of item (in octets) */
    char data[1];             /* text, not null-terminated */
} rtcp_sdes_item_t;

#if defined (UNDER_CE)
#pragma pack(pop)
#endif

/* One RTCP packet */
typedef struct {
    rtcp_common_t common;     /* common header */
    union {
            /* sender report (SR) */
        struct {
            UINT32_t ssrc;    /* sender generating this report */
            UINT32_t ntp_sec; /* NTP timestamp */
            UINT32_t ntp_frac;
            UINT32_t rtp_ts;  /* RTP timestamp */
            UINT32_t psent;   /* packets sent */
            UINT32_t osent;   /* octets sent */
            rtcp_rr_t repblk[1];  /* variable-length list */
        } sr;

            /* reception report (RR) */
        struct {
            UINT32_t ssrc;    /* receiver generating this report */
            rtcp_rr_t repblk[1];  /* variable-length list */
        } rr;

            /* source description (SDES) */
        struct rtcp_sdes {
            UINT32_t src;     /* first SSRC/CSRC */
            rtcp_sdes_item_t item[1]; /* list of SDES items */
        } sdes;

            /* BYE */
        struct {
            UINT32_t ssrc[1];  /* list of sources */
        } bye;
    } r;
} rtcp_t;

typedef struct rtcp_sdes rtcp_sdes_t;

/* Per-source state information */
typedef struct {
    UINT16_t max_seq;         /* highest seq. number seen */
    UINT32_t cycles;          /* shifted count of seq. number cycles */
    UINT32_t base_seq;        /* base seq number */
    UINT32_t bad_seq;         /* last 'bad' seq number + 1 */
    UINT32_t probation;       /* sequ. packets till source is valid */
    UINT32_t received;        /* packets received */
    UINT32_t expected_prior;  /* packet expected at last interval */
    UINT32_t received_prior;  /* packet received at last interval */
    UINT32_t transit;         /* relative trans time for prev pkt */
    UINT32_t jitter;          /* estimated jitter */
} source;


#define RTP_INVALID_SESSID 0

#pragma pack()

#endif /* __RTP_PRIVATE_H__ */





















