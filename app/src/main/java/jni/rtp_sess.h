/*
 * rtp_sess.h
 *
 * RTP user application should include thios header file in order
 * to access RTP interface functions.
 *
 * Venkat. C
 * Jan, 2003
 *
 * Copyright (c) 2003 HCL Technologies Ltd.
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
 * $Log: rtp_sess.h,v $
 * Revision 1.2  2009/12/17 08:27:34  baojianyi
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
 * Revision 1.5.8.6  2003/12/10 08:34:54  vcuddapa
 * Fix to Bug Id: 1743
 *
 * Revision 1.5.8.5  2003/11/21 10:27:43  vcuddapa
 * Added support to RFC2833 for sending DTMF digits (sec3.10 of RFC2833)
 *
 * Revision 1.5.8.4  2003/08/08 11:49:53  vcuddapa
 * MOdified to suppport RTP imestamp caluculations for different codecs.
 *
 * Revision 1.5.8.3  2003/08/05 15:24:40  vcuddapa
 * Removed cntrl M charecteres added because of copying the files from windows to linux.
 *
 * Revision 1.5.8.2  2003/08/05 15:05:05  vcuddapa
 * Modified to support Agere platform
 *
 * Revision 1.5.8.1  2003/07/21 08:59:56  vcuddapa
 * Fix for bug id 1329.
 *
 * Revision 1.5  2003/05/15 07:28:36  vcuddapa
 * Integrated with OSAPI, Added Windows support
 *
 * Revision 1.4  2003/03/24 10:49:24  vcuddapa
 * Debug flags added
 *
 * Revision 1.3  2003/03/03 11:37:35  vcuddapa
 * modified
 *
 * Revision 1.2  2003/02/18 12:49:35  vcuddapa
 * rtp support for mixer application is provided. Multicast support is added.
 *
 * $EndLog$
 */

#ifndef __RTP_SESS_H__
#define __RTP_SESS_H__

typedef struct rtp_sess_ {
    UINT32_t       sess_id;
    UINT32_t       local_ssrc;
    UINT32_t       rem_ssrc;
    UINT32_t 	   samplesize; /* rtp sample/frame size, based on codec */
    UINT32_t 	   num_samples; /* total number of samples/frames per packet*/ 

    Bool_t        hdr_comp; /* RTP header comprression enabled? */
    osapi_sock_t   rtp_sockfd;
    udp_addr_t     local_rtp_addr;  /* our rtp transport address */
    udp_addr_t     remote_rtp_addr; /* rtp transport address of
                                     * remote end of the RTP session */
    Bool_t 	   multicast_support; /* rtpfd is binded to multicast address
									   * if this member is TRUE */
    UINT32_t       next_timestamp;
    rtp_out_pack_t out_rtp;
    rtp_hdr_t      *hdr; /* hdr info of the incoming rtp pack */
    UINT32_t	   hdrlen;
    rtp_in_pack_t  in_rtp;
	
	UINT8_t		   rfc2833_pl_value; /* For rfc2833 support. 
									  * Application updates this
									  * filed during SDP negotiation */
	UINT32_t	   last_rcvd_ts;      /* last rcvd timestamp of rfc2833 packet
									  */ 
    int owner;									  	

} rtp_sess_t;

#endif

