/*
 * rtp_interface.h
 *
 * RTP user application should include this header file in order
 * to access RTP interface functions.
 *
 * Kedar B. Patil
 * April, 2002
 *
 * Venkat. C
 * Nov, 2002
 *
 * Venkat. C
 * Aug, 2003
 *
 * Copyright (c) 2002-2003 HCL Technologies Ltd.
 *
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of HCL Technologies Ltd.
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code
 */
/*
 * $Log: rtp_interface.h,v $
 * Revision 1.10  2010/01/06 09:30:43  baojianyi
 * no message
 *
 * Revision 1.9  2010/01/05 09:04:03  baojianyi
 * no message
 *
 * Revision 1.8  2009/12/22 06:04:39  baojianyi
 * no message
 *
 * Revision 1.7  2009/12/17 08:27:51  baojianyi
 * no message
 *
 * Revision 1.6  2009/12/16 04:00:10  baojianyi
 * no message
 *
 * Revision 1.5  2009/10/17 05:32:09  baojianyi
 * no message
 *
 * Revision 1.4  2009/09/21 01:46:50  baojianyi
 * no message
 *
 * Revision 1.3  2009/06/01 08:42:07  baojianyi
 * no message
 *
 * Revision 1.2  2009/03/20 06:18:58  baojianyi
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
 * Revision 1.17.4.11  2004/01/08 10:16:23  vcuddapa
 * updated the payloadtype value for G729 as per RFC3551.
 *
 * Revision 1.17.4.10  2003/12/10 08:34:54  vcuddapa
 * Fix to Bug Id: 1743
 *
 * Revision 1.17.4.9  2003/11/21 10:27:43  vcuddapa
 * Added support to RFC2833 for sending DTMF digits (sec3.10 of RFC2833)
 *
 * Revision 1.17.4.8  2003/08/22 12:58:13  vcuddapa
 * Modified the PCMU sample size.
 *
 * Revision 1.17.4.7  2003/08/21 13:21:41  vcuddapa
 * modified define for G729.
 *
 * Revision 1.17.4.6  2003/08/17 13:13:27  arsundar
 * Added frame/sample defines for more codecs.
 *
 * Revision 1.17.4.5  2003/08/08 11:49:53  vcuddapa
 * MOdified to suppport RTP imestamp caluculations for different codecs.
 *
 * Revision 1.17.4.4  2003/08/05 15:24:40  vcuddapa
 * Removed cntrl M charecteres added because of copying the files from windows to linux.
 *
 * Revision 1.17.4.3  2003/08/05 15:05:05  vcuddapa
 * Modified to support Agere platform
 *
 * Revision 1.17.4.1  2003/07/21 08:59:56  vcuddapa
 * Fix for bug id 1329.
 *
 * Revision 1.17  2003/06/12 07:14:56  vcuddapa
 * bug fixes for bug-ids 1221 and 1222.
 *
 * Revision 1.16  2003/05/20 09:04:38  vcuddapa
 * Fix for bugid 1083
 *
 * Revision 1.15  2003/05/15 07:28:36  vcuddapa
 * Integrated with OSAPI, Added Windows support
 *
 * Revision 1.14  2003/04/18 14:20:11  msivakum
 * BugId: 973 : Code re-org for OS dependant modules
 *
 * Revision 1.13  2003/03/19 09:57:35  vcuddapa
 * Modified parse_rtpevents_pack() and send_rtpteleph_events_pack() to support
 * the sending of DTMF digits and Telephony signalling via RTP.
 *
 * Revision 1.12  2003/03/03 11:37:35  vcuddapa
 * modified
 *
 * Revision 1.11  2003/02/18 12:49:35  vcuddapa
 * rtp support for mixer application is provided. Multicast support is added.
 *
 * $EndLog$
 */

#ifndef __RTP_INTERFACE_H__
#define __RTP_INTERFACE_H__

//#include "rtp_udp.h"
#include "rtp_private.h"

/*
#if defined (UNDER_CE) || defined (_WINDOWS)
#ifdef EXE
#define RTP_EXPORT
#else
#ifdef RTP_DLL
#define RTP_EXPORT __declspec(dllexport)
#else
#define RTP_EXPORT __declspec(dllimport)
#endif
#endif
#else 
#define RTP_EXPORT extern
#endif
*/
#define RTP_EXPORT extern

#define RTP_MAX_SESSIONS 4 /* prime number prefered */
#define RTP_FIXED_HDR_SIZE 12 /* bytes */
#define RTP_MAX_PACKSIZE 1024
#define RTP_INVALID_SESSID 0
#define RTP_MAX_SDES (255) /* maximum text length for SDES */
#define MAX_CSRC_LIST (15) /* This cc field in the RTP header is 
							* 4 bits, the maximum number of ssrc's
							* in a csrc list is 16 (range:0-15) */

#define SESSION_OWNER_SIP 0
#define SESSION_OWNER_PTT 1

#define RTP_MORE_DATA_FLAG_VALUE (0x5a5a5a5a)
#define RTP_MORE_DATA_FLAG_SIZE (4)

#define RTP_G729_10MS_PAYLOAD_SIZE (10)
#define RTP_G729_20MS_PAYLOAD_SIZE (20)
#define RTP_G729_40MS_PAYLOAD_SIZE (40)
#define RTP_G711_10MS_PAYLOAD_SIZE (80)
#define RTP_G711_20MS_PAYLOAD_SIZE (160)
#define RTP_G711_40MS_PAYLOAD_SIZE (320)
/***************************************************************
 * enum defining the payload types as idefined in rfc1890 and
 * as per rfc2833 for dynamic payload types
 ***************************************************************/
typedef enum {
	RTP_AUDIO_PAYLOAD_PCMU    = 0,     /* G.711 (ulaw) (8000 Hz)       */
	RTP_AUDIO_PAYLOAD_G726_32 = 2,     /* G.726-32 (8000 Hz)           */
	RTP_AUDIO_PAYLOAD_G723    = 4,     /* G.723 (8000 Hz)              */
	RTP_AUDIO_PAYLOAD_PCMA    = 8,     /* G.711 (alaw) (8000 Hz)       */
	RTP_AUDIO_PAYLOAD_G722    = 9,     /* G.722 (16000 Hz)             */
	RTP_AUDIO_PAYLOAD_L16_2   = 10,    /* Linear 16 (2 ch) (44100 Hz)  */
    RTP_AUDIO_PAYLOAD_L16_1   = 11,    /* Linear 16 (1 ch) (44100 Hz)  */
    RTP_AUDIO_PAYLOAD_COMFORT_NOISE = 13, /* COMFORT NOISE */
	RTP_AUDIO_PAYLOAD_G728    = 15,    /* G.728 (8000 Hz)              */
	RTP_AUDIO_PAYLOAD_G729    = 18,   /* G.729 (8000 Hz)              */

	RTP_RFC2833_PAYLOAD = 101,         /* dynamic payload type for DTMF digits,
			  				            * Telephony signals and events */
	UNDEFINED_PAYLOAD = 255
} e_rtp_pl_t;


/* Venkat @@ From Table4 : Payload types (PT) for Audio encodings of RFC 3551 */
#define RTP_AUDIO_CLOCK_RATE              (8)    /* The clock rate, in KHz 
                                                     * for the audio codecs 
                                                     * defined below */

#define RTP_10MS_TIMESTAMP  			(10)

/* Venkat @@ framesizes for different payloads */
/* Venkat @@ refered sec4.5 Audio Encodings of RFC3551 */


/* Sample Based Codecs - default ms/packet */

#define RTP_PT_G722_SAMPLES	    (20)      /* G.722 (16000 Hz)             */
#define RTP_PT_G726_16_SAMPLES      (20)      /* G726_32 (8000 Hz)            */
#define RTP_PT_G726_24_SAMPLES      (20)      /* G726_32 (8000 Hz)            */
#define RTP_PT_G726_32_SAMPLES      (20)      /* G726_32 (8000 Hz)            */
#define RTP_PT_G726_40_SAMPLES      (20)      /* G726_32 (8000 Hz)            */
#define RTP_PT_PCMU_SAMPLES	    (20)      /* G.711 (ulaw) (8000 Hz)       */
#define RTP_PT_PCMA_SAMPLES	    (20)      /* G.711 (ulaw) (8000 Hz)       */
#define RTP_PT_L16_SAMPLES	    (20)      /* L16 			      */

/* Frame Based Codec - default frames/packet */

#define RTP_PT_G723_FRAMES   1          /* (30) / (30) G.723 (8000 Hz)  */
#define RTP_PT_G729_FRAMES   2	        /* (20) / (10) G.729 (8000 Hz)  */
#define RTP_PT_G728_FRAMES   8		/* (20) / (2.5)G.728 (8000 Hz)  */

#ifdef LITTLE_ENDIAN_ARCH


/* RTP Telephony EVENTS */
typedef struct rtpevent_rfc2833_ {

	UINT8_t event;
    UINT8_t volume: 6; /* expressed in dbm0 - range is 0 - -36dbm0 */
    UINT8_t reserved: 1;
    UINT8_t edge: 1;    
    UINT16_t duration;

} rtpevent_rfc2833_t;

#endif

#ifdef BIG_ENDIAN_ARCH

/* RTP Telephony EVENTS */
typedef struct rtpevent_rfc2833_ {

    UINT8_t event;
    UINT8_t edge: 1;
    UINT8_t reserved: 1;
    UINT8_t volume: 6;
    UINT16_t duration;

} rtpevent_rfc2833_t;

#endif

typedef struct {
    UINT16_t next_seq;
    UINT8_t csrc_count;  /* mixer level details */
    UINT32_t packsize; /* size of rtp encapsulated pack ready to send */
    struct sk_buff *pack;      /* rtp encapsulated pack ready for sending */
} rtp_out_pack_t;

#define IP_ADDR_SIZE (16)

typedef struct {
    char     ipv4[IP_ADDR_SIZE];
    UINT32_t ip;	//NET SEQ
    UINT16_t port; //NET SEQ
} udp_addr_t;


typedef struct {
	udp_addr_t from; /* source transport address of incomimg rtp pack */
	UINT8_t csrc_count;  /* mixer level details */
	UINT32_t *csrc_list[MAX_CSRC_LIST]; /* mixer level details */
	UINT8_t *payload_start;
	UINT32_t payload_len;
	UINT8_t pack[RTP_MAX_PACKSIZE];
	UINT32_t packsize;
} rtp_in_pack_t;


typedef struct {
    char     *str;
    UINT32_t len;
} string_t;


#define MAX_RTP_TELEEVENTS  64
/*******************************************************
The following DTMF and part of Line events are supported.
Trunk events, Fax events are not supported at present.
Please refer rfc2833 for clarification.
********************************************************/

typedef enum
{
    TELEPHEVENTNULL = -1,

    /* DTMF Named Events */
    DTMFEVENTDIGIT0,
    DTMFEVENTDIGIT1,
    DTMFEVENTDIGIT2,
    DTMFEVENTDIGIT3,
    DTMFEVENTDIGIT4,
    DTMFEVENTDIGIT5,
    DTMFEVENTDIGIT6,
    DTMFEVENTDIGIT7,
    DTMFEVENTDIGIT8,
    DTMFEVENTDIGIT9,
    DTMFEVENTDIGITSTAR,
    DTMFEVENTDIGITHASH,
    DTMFEVENTALPHA_A,
    DTMFEVENTALPHA_B,
    DTMFEVENTALPHA_C,
    DTMFEVENTALPHA_D,
    DTMFEVENTFLASH

#if 0
	/* PRESENTLY WE ONLY CODE SUPPORT FOR DTMF EVENTS.
	 * THE SUPPORT FOR THE REST OF THE FOLLOWING EVENTS
	 * ARE SUPPORTED IN THE LATER RELEASES
	*/
	/* Data Modem and Fax Named Events */
	,
	ANS_FAX_ECHO_ENABLE = 32,
	ANS_FAX_ECHO_DISABLE,
	ANS_MODEM_ECHO_ENABLE,
	ANS_MODEM_ECHO_DISABLE,
	CNG_TONE,
	V_21_1_0,  /* V.21 Channel 1, "0" bit */
	V_21_1_1,  /* V.21 Channel 1, "1" bit */
	V_21_2_0,  /* V.21 Channel 2, "0" bit */
	V_21_2_1,  /* V.21 Channel 2, "1" bit */
	CRDI_TONE, /* Capabilities request from initiating side */
	CRDR_TONE, /* Capabilities response to CRDI */
	CRE_TONE,
	ESI_SIGNAL,
	ESR_SIGNAL,
	MRDI_TONE,
	MRDR_TONE,
	MRE_TONE,
	CT_MODDEM_TONE,
	
    /* Line Named Events */
    OFFHK 		= 64,
    ONHK,
    DIALTONE,
    RINGINGTONE 	= 70,
    BUSYTONE 		= 72,
    OFFHK_WARNINGTONE 	= 88

#endif

} e_telephony_event;

/* end - support for RFC2833 */

/* CORE API's */
RTP_EXPORT Bool_t init_rtp(UINT32_t, UINT32_t);
RTP_EXPORT Bool_t vallid_rtp_session(UINT32_t);
RTP_EXPORT UINT32_t open_rtp_session(char *, UINT16_t *, char *, int);
/* RTP_EXPORT void send_rtp_pack(UINT32_t, UINT32_t, char *, UINT8_t,
						  	  UINT32_t, Bool_t, UINT8_t); */
RTP_EXPORT int send_rtp_pack(UINT32_t, UINT32_t, struct sk_buff *, UINT32_t, UINT8_t,
						  	  UINT32_t, Bool_t, UINT32_t); 
//RTP_EXPORT UINT32_t convert(char *);
RTP_EXPORT UINT8_t parse_rtp_pack(UINT32_t, osapi_sock_t, rtp_in_pack_t **,
                				 	 rtpevent_rfc2833_t **, UINT32_t *);
RTP_EXPORT INT32_t send_rtpteleph_events_pack (UINT32_t, UINT32_t,
          			                   		   rtpevent_rfc2833_t **,
                     			       		   UINT8_t, UINT32_t,UINT8_t);
RTP_EXPORT void clear_rtp_events_pack(rtpevent_rfc2833_t **, UINT32_t);
RTP_EXPORT void close_rtp_session(UINT32_t);
RTP_EXPORT void terminate_rtp(void);



/* GET API's */
RTP_EXPORT Bool_t get_current_timestamp(UINT32_t, UINT32_t *);
RTP_EXPORT UINT16_t get_local_port(UINT32_t);
RTP_EXPORT char * get_local_ipaddr(UINT32_t);
RTP_EXPORT UINT16_t get_remote_port(UINT32_t);
RTP_EXPORT char * get_remote_ipaddr(UINT32_t);
RTP_EXPORT UINT32_t get_packcount_sent(UINT32_t);
RTP_EXPORT UINT32_t get_octetcount_sent(UINT32_t);
RTP_EXPORT UINT32_t get_packcount_rcvd(UINT32_t);
RTP_EXPORT UINT32_t get_octetcount_rcvd(UINT32_t);
RTP_EXPORT UINT32_t get_jitter_val(UINT32_t);
RTP_EXPORT UINT32_t get_avtran_delay (UINT32_t);
RTP_EXPORT osapi_sock_t get_rtp_fd(UINT32_t);
RTP_EXPORT Bool_t get_rtp_hdr(UINT32_t sess_id, rtp_hdr_t **rtpHdr);
RTP_EXPORT UINT16_t get_current_seqno(UINT32_t);
RTP_EXPORT UINT8_t get_rfc2833_pltype_value (UINT32_t);



/* SET API's */
RTP_EXPORT Bool_t set_email_addr(UINT32_t, char *);
RTP_EXPORT Bool_t set_uname(UINT32_t, char *);
RTP_EXPORT Bool_t set_phno(UINT32_t, char *);
RTP_EXPORT Bool_t set_location(UINT32_t, char *);
RTP_EXPORT Bool_t set_session_local(UINT32_t sessid, char *local_ipv4, UINT16_t local_port);
RTP_EXPORT Bool_t set_session_remote(UINT32_t, char *, UINT16_t);
RTP_EXPORT INT32_t set_rtp_samplesize(UINT32_t, UINT32_t);
RTP_EXPORT INT32_t set_rtp_samples_count(UINT32_t, UINT32_t);
RTP_EXPORT INT32_t set_rfc2833_pltype_value (UINT32_t, UINT8_t);


#endif /* __RTP_INTERFACE_H__ */
