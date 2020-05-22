

/*************************************************************************
     Nhce:     Jitter Buffer - Timer Processing Module
     Type:     C source file
     Desc:     Timer Processing File for Jitter Buffer.
     File:     jb_timer.c
     Sid:      jb_timer.c 1.0  -  01/25/2000
     Prg:      sdg
  Copyright (c) 2003 Agere Systems Inc.
  All Rights Reserved

  This is unpublished proprietary information of Agere Systems Inc.
  This copyright notice does not evidence publication.

  The use of the software, documentation, methodologies and other
  information contained herein is governed solely by the associated
  license agreements.  Any inconsistent use shall be deemed to be a
  misappropriation of the intellectual property of Agere Systems Inc.
  and treated accordingly.

  Change Log:

  Date        By                        Description
  ========    ===               ======================================
  03/12/2001  mwb               Add support for third-party pause

  04/08/2003  Venkat.C (HCLT)   Modified this file to integrate with HCLT
                                RTP Library.


***************************************************************************/

/***************************************************************************

The following functions are provided in this file:

   jbActvTmr            Main timer activation routine
   JbStartTimer         Start a long silence timer
   JbStopTimer          Stop a long silence timer

***************************************************************************/

/* header include files (.h) */


//#include "linux_oa.h"

#include "jb_def.h"        /* Jitter Buffer structures/prototypes */

//#include "voip_oa_includes.h"

#include "rtp_interface.h"
#include "rtp_sess.h"
#include "voip_oa_log.h"

/* local defines */
#define MAX_MESSAGE_SIZE            (1024) /* Max length of timer message */
#define NAME_LEN_MAX                (32)
#define LS_TIMER_QUEUE              "LongSilenceTimerQ"
#define TELEEVENT_TIMER_QUEUE       "Rfc2833EventTimerQ"
#define INTER_ARRAIVAL_JITTER_DELAY (50)

int check_dtmf_sending();
INT32_t JB_UpdateRoamTime();

/* forward references */
INT16_t jbProcessRxCodec ARGS((UINT8_t connIndex,unsigned char *pDst));
/* neal: 1/11/2005 */
static INT16_t jbProcessTxCodec ARGS((JbConnCb* connCb,
UINT32_t size,
UINT32_t num, struct sk_buff *skb));
static void jbProcessLongSilence ARGS((JbConnCb* connCb));

//static ev_ret_t* LSTimerCallBack(void* arg1, void* arg2);
static void LSTimerCallBack(HTIMER, UINT32_t, void *);
//static ev_ret_t* TeleEvTimerCallBack(void* arg1, void* arg2);
static void TeleEvTimerCallBack(HTIMER, UINT32_t, void *);

static UINT8_t codecJB2RTP(CoderInfo txCodec);
/* extern INT16_t JbResetJitterReq (UINT8_t connIndex); */
static dtmf_tone_type convert2DSPEvent(UINT8_t event);
static void process_rfc2833_event_pack(UINT32_t connId, rtpevent_rfc2833_t** rtp_ev_pack, UINT32_t ev_count);

UINT32_t get_rtp_remote_ipaddr(UINT32_t sessid);

/* public variable declarations */
extern void* hDlDSP[MAX_LINES];
extern void* hUlDSP[MAX_LINES];

INT32_t gSessionNoPacketCount[MAXIMUM_CONNECTIONS];
extern INT32_t fRequestjbReadTaskStop[];
extern INT32_t fRequestGetMgrTaskStop[];
extern int fRequestPutMgrTaskStop[];

void drvInitRtpHandler(void *puser, unsigned int ip, unsigned short port, unsigned short localPort);
void drvCleanRtpHandler(void);

/*
**       Fun:   jbReadSocket
**       Desc:  This task is spawned by jbConReq and
**              is responsible for receiving RTP packet
**              from the n/w and raise an calls JbDatInd()
**
**       Ret:   ROK on success and RFAILED on failure
**       Notes:
**       File:  jb_timer.c
**/

void ResetRtpStream(void *pUser)
{
    rtpfd_cntxt_t       *cntxt = NULL;
    cntxt = (rtpfd_cntxt_t *)pUser;
    if (jbCb.conn[cntxt->connIndex] == NULL)
    {
        return;
    }

    VOIP_OA_MutexLock(jbCb.sem[cntxt->connIndex]);
    JbResetJitterReq(cntxt->connIndex);
    VOIP_OA_MutexUnlock(jbCb.sem[cntxt->connIndex]);
}


int  drv_proc_rcvd_rtp(int jbline, void *rtp, unsigned int length)
{
    UINT8_t *audioBufPtr;
    UINT32_t audioBufLen;
    UINT32_t rtpHeaderLen;
    
    UINT32_t headers[6];
    rtp_hdr_t* rtpHdr;
    UINT8_t payload_type;

    unsigned int moreDataFlag = 0;
    unsigned int bMoreData = 0;

    UINT8_t *rtpData = NULL;
    unsigned int rtpLength = 0;

    int putlength = 0;
//    rtp_sess_t *rtpsess_p = NULL;
	
    rtpData = (UINT8_t *)rtp;
    rtpLength = length;
    

    /* If we are exiting, stop the task
    ** 0 - normal operation
    ** 1 - Request outstanding that the timer task stop
    **/
    if(jbline>=MAXIMUM_CONNECTIONS)
        return RFAILED;
    
    if (jbCb.conn[jbline] == NULL)
    {
        return RFAILED;
    }

    if (!(jbCb.conn[jbline]->conn_mask & CONN_RECV))
    {
        return RFAILED;
    }
    

    do 
    {
        memcpy((char *)headers, rtpData, sizeof(rtp_hdr_t));
        rtpHdr = (rtp_hdr_t*)headers;

        payload_type = rtpHdr->pt;

        if (payload_type == UNDEFINED_PAYLOAD)
        {
            return RFAILED;
        }

        if (payload_type == RTP_AUDIO_PAYLOAD_COMFORT_NOISE)
        {
            // Comfortable Noise
            return RFAILED;
        }

        // Support Below Codec
        if (payload_type != RTP_AUDIO_PAYLOAD_PCMU &&
            payload_type != RTP_AUDIO_PAYLOAD_PCMA &&
            payload_type != RTP_AUDIO_PAYLOAD_G729)
        {
            return RFAILED;
        }

        rtpHdr->seq = VOIP_OA_ntohs(rtpHdr->seq);
        rtpHdr->ts = VOIP_OA_ntohl(rtpHdr->ts);
        rtpHdr->ssrc = VOIP_OA_ntohl(rtpHdr->ssrc);

        if (jbCb.conn[jbline]->remote_ssrc != rtpHdr->ssrc/* BJY 2009.02.23|| rtpHdr->m*/)
        {
            VOIP_OA_MutexLock(jbCb.sem[jbline]);
            jbCb.conn[jbline]->remote_ssrc = rtpHdr->ssrc;
            JbResetJitterReq(jbline);
            VOIP_OA_MutexUnlock(jbCb.sem[jbline]);
        }

        //Cal RTP HEADER LENGTH
        rtpHeaderLen = sizeof(rtp_hdr_t) - sizeof(UINT32_t)/* Reduce CC */ + sizeof(UINT32_t) * rtpHdr->cc;
        if (rtpLength <= rtpHeaderLen)
        {
            ASSERT(0);
            return RFAILED;
        }

        audioBufLen = rtpLength - rtpHeaderLen;
        audioBufPtr = (UINT8_t *)rtpData;
        audioBufPtr += rtpHeaderLen;

        bMoreData = 0;
        if (1)//rtpsess_p->owner == SESSION_OWNER_PTT)
        {
            int iPayloadSize;
            if(rtpHdr->pt==18)
                iPayloadSize = RTP_G729_20MS_PAYLOAD_SIZE;
            else
                iPayloadSize = RTP_G711_20MS_PAYLOAD_SIZE;

            // 20MS G.729 PACKAGE SIZE IS 20 BYTES
            // Check More Package Flags
            if (audioBufLen > iPayloadSize)
            {
                memcpy(&moreDataFlag, (audioBufPtr + iPayloadSize), RTP_MORE_DATA_FLAG_SIZE);
                if (moreDataFlag == RTP_MORE_DATA_FLAG_VALUE)
                {
                    bMoreData = 1;
                }
            }

            jbCb.conn[jbline]->jitter.packetSize = iPayloadSize;
            jbCb.conn[jbline]->jitter.packetTime = JB_DFLT_PTIME;

            if (bMoreData)
            {
                JbDatInd(jbline, rtpHdr, audioBufPtr, iPayloadSize);
                rtpData += (rtpHeaderLen + iPayloadSize + RTP_MORE_DATA_FLAG_SIZE);
                rtpLength -= (rtpHeaderLen + iPayloadSize + RTP_MORE_DATA_FLAG_SIZE);
                putlength += iPayloadSize;
            }
            else
            {
                JbDatInd(jbline, rtpHdr, audioBufPtr, iPayloadSize);
                putlength += iPayloadSize;
            }

        }
        else
        {
            if (payload_type == RTP_AUDIO_PAYLOAD_G729)
            {
                if (audioBufLen < RTP_G729_10MS_PAYLOAD_SIZE || audioBufLen > RTP_G729_40MS_PAYLOAD_SIZE)
                {
                    break;
                }
                // 10MS整数倍
                audioBufLen = (audioBufLen/RTP_G729_10MS_PAYLOAD_SIZE)*RTP_G729_10MS_PAYLOAD_SIZE;
            }

            if (payload_type == RTP_AUDIO_PAYLOAD_PCMA ||
                payload_type == RTP_AUDIO_PAYLOAD_PCMU)
            {
                if (audioBufLen < RTP_G711_10MS_PAYLOAD_SIZE || audioBufLen > RTP_G711_40MS_PAYLOAD_SIZE)
                {
                    break;
                }

                // 10MS整数倍
                audioBufLen = (audioBufLen/RTP_G711_10MS_PAYLOAD_SIZE)*RTP_G711_10MS_PAYLOAD_SIZE;
            }

            jbCb.conn[jbline]->jitter.packetSize = audioBufLen;
            jbCb.conn[jbline]->jitter.packetTime = calcPacketTime(jbCb.conn[jbline]->key.rxCodec, audioBufLen);
            
            JbDatInd(jbline, rtpHdr, audioBufPtr, audioBufLen);
            
        }
    }while (bMoreData);
    return putlength;
}

void *VOIP_OA_CurTaskGetParameter()
{
	UINT8_t connIndex;
	VOIP_OA_HANDLE_TASK h = VOIP_OA_TaskThisHandle();
	for( connIndex = 0; connIndex < MAXIMUM_CONNECTIONS; connIndex++ )
	{
             if (jbCb.conn[connIndex] == NULL)
             {
                continue;
             }
             
		if( jbCb.conn[connIndex]->hGetTask == h )
			return &(jbCb.conn[connIndex]->GetTaskParam);
		else if(  jbCb.conn[connIndex]->hPutTask == h )
			return &(jbCb.conn[connIndex]->PutTaskParam);
		else if(  jbCb.conn[connIndex]->hRdSocketTask == h )
			return &(jbCb.conn[connIndex]->RdSocketTaskParam);
	}

	VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n%s[%d]:VOIP_OA_CurTaskGetParameter error\n", __FUNCTION__, __LINE__);
	return NULL;
}

void jbReadSocket(void *arg)
{
    rtpfd_cntxt_t* cntxt = NULL;

    UINT8_t connIndex = 0;
    taskParam_t* pTaskParam = NULL;
    unsigned int remoteIp;
    unsigned short remotePort;
    unsigned short localPort;

    pTaskParam = (taskParam_t *) VOIP_OA_CurTaskGetParameter();
    connIndex = pTaskParam->ConnIndex;

    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: Enter jbReadSocket.\n");

    cntxt = (rtpfd_cntxt_t *) VOIP_OA_AllocMemory(sizeof(rtpfd_cntxt_t));
    if (cntxt == NULL)
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n%s[%d]:Memory allocation error\n", __FUNCTION__, __LINE__);       
        goto Exit;
    }
    VOIP_OA_MEMSET(cntxt, 0, sizeof(rtpfd_cntxt_t));

    VOIP_OA_MutexLock(jbCb.sem[connIndex]);


    cntxt->lineNo = jbCb.conn[connIndex]->key.lineNo;
    cntxt->sessionId = jbCb.conn[connIndex]->key.sessionId;


    VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);

    cntxt->connIndex = connIndex;

    gSessionNoPacketCount[cntxt->connIndex] = 0;;
    
//    remoteIp = inet_addr(get_remote_ipaddr(cntxt->sessionId));
    remotePort = get_remote_port(cntxt->sessionId);
    localPort = get_local_port(cntxt->sessionId);
    drvInitRtpHandler(cntxt, remoteIp, remotePort, localPort);

    while ( 1 /* baojianyi vallid_rtp_session(sessionId)*/)
    {
        taskDelay(2);
        
        gSessionNoPacketCount[cntxt->connIndex]++;

        if (fRequestjbReadTaskStop[cntxt->connIndex])
        {
            break;
        }
    }
 
    drvCleanRtpHandler();

Exit:

    if (cntxt != NULL)
    {
        VOIP_OA_FreeMemory(&cntxt);
        cntxt = NULL;
    }
    
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: exit jbReadSocket thred.\n");
    fRequestjbReadTaskStop[connIndex] = 0;
}

/*
**       Fun:   jbDspGetManager
**       Desc:  This task is spawned by jbConReq and
**              is responsible for getting and putting
**              Dsp Buffers
**       Ret:   ROK on success and RFAILED on failure
**       Notes:
**       File:  jb_timer.c
*/


extern char* pSlientData;
extern char SlientDatalen;
extern UINT16_t noframe;


void jbDspGetManager(void *arg)
{
    UINT16_t size=0;
    UINT16_t nframes;
    UINT32_t sessionId;
    UINT8_t txCodec;

    INT16_t lineNo;

    UINT8_t connIndex = 0;
	int iTmp = 0;

    struct sk_buff *jbSkb;

    taskParam_t* pTaskParam = (taskParam_t*) VOIP_OA_CurTaskGetParameter();

    connIndex = pTaskParam->ConnIndex;


    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: enter jbDspGetManager thread.\n");


    VOIP_OA_MutexLock(jbCb.sem[connIndex]);

    lineNo = jbCb.conn[connIndex]->key.lineNo;
    sessionId = jbCb.conn[connIndex]->key.sessionId;
    txCodec = (UINT8_t) jbCb.conn[connIndex]->key.txCodec;
    nframes = (UINT16_t) jbCb.conn[connIndex]->numframes;  /* neal: 1/3/2005 */

    VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);
        /* Alloc memory include RTP_HEADER_LEN */
       
// send slient rtp to awake other side

    for(iTmp=0;iTmp<3;iTmp++)
    {
        jbSkb = dev_alloc_skb(JB_MAX_BUF_SIZE + JB_MAX_SKB_RESERVE_SIZE/*NET HEADER PRE-RESERVE*/);
        skb_reserve(jbSkb, JB_MAX_SKB_RESERVE_SIZE);
        
        nframes = (UINT16_t) jbCb.conn[connIndex]->numframes;
        
        size = 0;
        memcpy(jbSkb->data,pSlientData,SlientDatalen);
        size = SlientDatalen;
        nframes = noframe;

        skb_put(jbSkb, (int)size);
        
        if (nframes != 0 && size != 0)
        {
            if (jbProcessTxCodec(jbCb.conn[connIndex], size, nframes, jbSkb) != 0)
            {
                dev_kfree_skb(jbSkb);
            }
        }
        else
        {
            dev_kfree_skb(jbSkb);
        }
    }

    /* RTP Interface     */
    while ( 1 /* baojianyi vallid_rtp_session(sessionId)*/)
    {
        /* If we are exitting, stop the task
        **
        ** 0 - normal operation
        ** 1 - Request outstanding that the timer task stop
        **/
        if (fRequestGetMgrTaskStop[connIndex])
        {
            //fRequestGetMgrTaskStop[connIndex] = 0;
            break;
        }

        /* If this task is not for the active channel, then we do NOT want
        ** it to eat up any of the encoded (transmit) bytes of audio!
        **/
        ASSERT(connIndex == jbCb.activeConn[lineNo]);


        ASSERT(NULL != hUlDSP[lineNo]);

        jbSkb = dev_alloc_skb(JB_MAX_BUF_SIZE + JB_MAX_SKB_RESERVE_SIZE/*NET HEADER PRE-RESERVE*/);
        skb_reserve(jbSkb, JB_MAX_SKB_RESERVE_SIZE);
        
        nframes = (UINT16_t) jbCb.conn[connIndex]->numframes;
        
        size = 0;
        UlDSPGetBuffer(jbSkb->data, &nframes, &size, lineNo);
//        AddNetDebug(jbSkb->data, size);

        skb_put(jbSkb, (int)size);
        
        if (nframes != 0 && size != 0)
        {
            if (jbProcessTxCodec(jbCb.conn[connIndex], size, nframes, jbSkb) != 0)
            {
                dev_kfree_skb(jbSkb);
            }
        }
        else
        {
            dev_kfree_skb(jbSkb);
        }

        JB_UpdateRoamTime();
        
    }

    if (fRequestGetMgrTaskStop[connIndex])
    {
        fRequestGetMgrTaskStop[connIndex] = 0;
    }

    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:	exit jbDspGetManager thread.\n");
}

/*
**       Fun:   jbProcessTxCodec
**       Desc:  This function is used to retrieve data from DSP and send to RTP
**       Ret:   ROK on success and RFAILED on failure
**       Notes:
**       File:  jb_timer.c
*/

/* neal: 1/11/2005 */


static INT16_t
jbProcessTxCodec(JbConnCb* connCb, UINT32_t framesize, UINT32_t numframes, struct sk_buff * skb)
{
    UINT32_t n_frames = 0;
    UINT32_t sessionId;
    INT16_t lineNo;
    CoderInfo txCodec;
    UINT8_t codec;
    UINT32_t csrc = 0;
    
    /*Bool_t codec_match = FALSE;*/


    TRC3(jbProcessTxCodec)

    if (check_dtmf_sending())
    {
        return -1;
    }

    if (!(connCb->conn_mask & CONN_SEND))
    {
        return -1;
    }

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);

    sessionId = connCb->key.sessionId;
    lineNo = connCb->key.lineNo;
    txCodec = connCb->key.txCodec;

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);

    if ((txCodec == JB_G711_ULAW) || (txCodec == JB_G711_ALAW) || (txCodec == JB_G722) || (txCodec == JB_G726_32))
    {
        /* In the case of sample based codecs multiply the numframes with 10msec time stamp
               * to get number of samples in the rtp packet In this case n_frames is nothing but
               * number of samples
             */
        n_frames = JB_10MS_TIMESTAMP * numframes;
    }
    else
    {
        /* In the case of frames based codecs, it will be just numframes and n_frames
               * represents number of frames in the rtp packet
               */
        n_frames = numframes;
    }

    codec = codecJB2RTP(txCodec);
    if (codec == UNDEFINED_PAYLOAD)
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n JB: Unsupported Codec[%d]\n", txCodec);
        return (-1);
    }
    return send_rtp_pack(sessionId, framesize, skb, n_frames, codec, 0, FALSE, csrc);
}


/*
*       Fun:   jbDspPutManager
*       Desc:  This task is spawned by JbConReq() and
*              is responsible for getting and putting
*              Dsp Buffers
*       Ret:   ROK on success and RFAILED on failure
*       Notes:
*       File:  jb_timer.c
*/

void jbDspPutManager(void *arg)
{
    UINT32_t sessionId;
    UINT8_t connIndex = 0;

    taskParam_t* pTaskParam = NULL;

    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: enter jbDspPutManager thread.\n");

    pTaskParam = (taskParam_t *) VOIP_OA_CurTaskGetParameter();

    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: PTaskParm is %x\n", pTaskParam);
    connIndex = pTaskParam->ConnIndex;

    VOIP_OA_MutexLock(jbCb.sem[connIndex]);

    sessionId = jbCb.conn[connIndex]->key.sessionId;

    VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);

    /* RTP Interface */
    while ( 1 /* baojianyi vallid_rtp_session(sessionId)*/)
    {
        /* If we are exitting, stop the task
        **
        ** 0 - normal operation
        ** 1 - Request outstanding that the timer task stop
        */
        if (fRequestPutMgrTaskStop[connIndex])
        {
            break;
        }

        VOIP_OA_MutexLock(jbCb.sem[connIndex]);
        
        /* If this task is not for the active channel, then we do NOT want
        ** it to eat up any of the encoded (transmit) bytes of audio!
        **/
        if (connIndex != jbCb.activeConn[jbCb.conn[connIndex]->key.lineNo])
        {
            VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);
            VOIP_OA_TaskDelayTick(4);
            continue;       /* sleep for a bit before checking again */
        }

        VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);
        
        if (jbCb.conn[connIndex]->jitter.gotFirst == 0)
        {
            DlDSPGenerateSilence(jbCb.conn[connIndex]->key.lineNo, 0);
            continue;
        }

        if (!(jbCb.conn[connIndex]->conn_mask & CONN_RECV))
        {
            DlDSPGenerateSilence(jbCb.conn[connIndex]->key.lineNo, 0);
            continue;
        }
        
        if (jbProcessRxCodec(connIndex,NULL) == RFAILED)
        {
            VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n%sProcessing received codec failed\n", __FUNCTION__);
            ASSERT(0);
            break;
        }
        
    }

    if (fRequestPutMgrTaskStop[connIndex])
    {
        fRequestPutMgrTaskStop[connIndex] = 0;
    }
		
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: exit jbDspPutManager thread.\n");
}


/*
**       Fun:   jbProcessRxCodec
**       Desc:  This function is used to process and send an RX Codec event
**       Ret:   ROK on success and RFAILED on failure
**       Notes:
**       File:  jb_timer.c
*/

void LongSilenceHook(int index)
{
    JbConnCb* connCb;

    connCb = jbCb.conn[index];

    if(connCb->jbLSTimer!=NULL){
        if(connCb->jbLSTimer->pTimer!=NULL){
            if(connCb->jbLSTimer->TimeTick<connCb->jbLSTimer->WaitTick){
                connCb->jbLSTimer->TimeTick++;
                if(connCb->jbLSTimer->TimeTick==connCb->jbLSTimer->WaitTick){
                    connCb->jbLSTimer->cb(connCb->jbLSTimer->pTimer,0,connCb->jbLSTimer);
                }
            }
        }        
    }

}

INT16_t
jbProcessRxCodec(UINT8_t connIndex,unsigned char *pDst)
{
    JbConnCb* connCb;
    INT16_t ret;
    JbState state;

    TRC3(jbProcessRxCodec);

    connCb = jbCb.conn[connIndex];
    
    ASSERT(connCb != NULL);

    if(connCb==NULL)
        return RFAILED;
    VOIP_OA_MutexLock(jbCb.sem[connIndex]);    
    state = connCb->jitter.state;
    VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);

    ret = JbFSM[JB_PLAY_PACKET][state](connCb, pDst, 0, NULL);
    LongSilenceHook(connIndex);
    return(ret);
}

/*
**       Fun:   jbProcessLongSilence
**       Desc:  This function is used to process and send a LongSilence Event
**       Ret:   Nothing
**       Notes:
**       File:  jb_timer.cpp
*/

static void
jbProcessLongSilence(JbConnCb* connCb)
{
    JbState state;

    TRC3(jbProcessLongSilence)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: enter jbProcessLongSilence.\n");


    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);
    state = connCb->jitter.state;
    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);

    if (connCb != NULL)
    {
        if (state == JB_UNDERFLOW)
        {
            VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "\nJB:%s:The connection index is %u\n", __FUNCTION__, connCb->jbLSTimer->connId);

            JbFSM[JB_LONG_SILENCE][JB_UNDERFLOW](connCb, NULL, 0, NULL);
        }
    }
} /*jbProcessLongSilence*/



/*
**       Fun:   jbActvTmr
**       Desc:  This function is registered in JbCfgReq() to
**              periodically handle long silence timer events
**
**       Ret:   ROK on success and RFAILED on failure
**       Notes:
**       File:  jb_timer.c
*/
extern INT32_t
jbActvTmr( JbLSTCntxt* LSTimer)
{
    UINT8_t connId;

    connId = LSTimer->connId;

    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: enter jbActvTmr.\n");

    /* S.K -
     * Shouldn't we semTake before the foll. check
     */
    if (jbCb.conn[connId] == NULL)
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n Connection no more exists\n");
        return (-1);
    }

        jbProcessLongSilence(jbCb.conn[LSTimer->connId]);
//        VOIP_OA_MEMSET(jbCb.conn[LSTimer->connId]->jbLSTimer, 0, sizeof(JbLSTCntxt));

       VOIP_OA_MutexLock(jbCb.sem[connId]);

	VOIP_OA_FreeMemory(&(jbCb.conn[connId]->jbLSTimer));   
    	jbCb.conn[connId]->jbLSTimer = NULL;
		
       VOIP_OA_MutexUnlock(jbCb.sem[connId]);

    return 0;
}

/**LSTimerCallBack()
 * @param void *    - pointer to arg1.
 * @param void *    - pointer to arg2.
 *
 * @return void *
 *
 * @doc             - When timer id is set this function is called
 *                    to process the timer event.
 */
static void
LSTimerCallBack(HTIMER hTimer, UINT32_t type, void *arg)
{

	JbLSTCntxt * LSTimer;

    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: enter LSTimerCallBack.\n");


	LSTimer = (JbLSTCntxt *)arg;
	
    if ((jbActvTmr(LSTimer)) == -1)
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "%s:%d:Long silence timer message process failed\n", __FUNCTION__, __LINE__);
    }
    return;
}


/*
**       Fun:   JbStartTimer
**       Desc:  This function is used to start a long silence timer
**       Ret:   ROK on success and RFAILED on failure
**       Notes:
**       File:  jb_timer.c
*/
extern INT16_t
JbStartTimer(JbConnCb* connCb)
{
    INT32_t duration_ms;

	VoipTimer_t* timer;

    TRC3(JbStartTimer)
//    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: enter JbStartTimer.\n");


    duration_ms = jbCb.jitCfg.longSTimerVal;
    if (connCb->jbLSTimer == NULL)
    {
        connCb->jbLSTimer = (JbLSTCntxt *) VOIP_OA_AllocMemory(sizeof(JbLSTCntxt));

        if (connCb->jbLSTimer == NULL)
        {
            VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n%s[%d]: Memory allocation failed\n", __FUNCTION__, __LINE__);       
            return(RFAILED);
        }
        VOIP_OA_MEMSET(connCb->jbLSTimer, 0, sizeof(JbLSTCntxt));
    }
   else
    {           
        if (VOIP_TIMER_NULL != connCb->jbLSTimer->pTimer)
        {
		voip_timer_stop(connCb->jbLSTimer->pTimer);
            connCb->jbLSTimer->pTimer = VOIP_TIMER_NULL;
        }
    }

    connCb->jbLSTimer->connId = connCb->connIndex;

	timer = (VoipTimer_t *)voip_timer_start( 18, duration_ms, LSTimerCallBack, connCb->jbLSTimer, FALSE );
	
	connCb->jbLSTimer->pTimer = timer;
	
    if (VOIP_TIMER_NULL == timer)
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\nError: Starting the timer failed\n");


        VOIP_OA_FreeMemory(&(connCb->jbLSTimer));

        connCb->jbLSTimer = NULL;
        return(RFAILED);
    }
    return(ROK);
}

/*
**       Fun:   JbStopTimer
**       Desc:  This function is used to stop a long silence timer
**       Ret:   ROK on success and RFAILED on failure
**       Notes:
**       File:  jb_timer.c
*/
extern INT16_t
JbStopTimer(JbConnCb* connCb)
{
    /*  INT8_t ret; */
    TRC3(JbStopTimer)

    if ((connCb == NULL) || (connCb->jbLSTimer == NULL))
    {
        return (RFAILED);
    }

//    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: Enter JbStopTimer.\n");

    if (VOIP_TIMER_NULL != connCb->jbLSTimer->pTimer)
    {
	voip_timer_stop( connCb->jbLSTimer->pTimer );
    }

    VOIP_OA_FreeMemory(&(connCb->jbLSTimer));   
    connCb->jbLSTimer = NULL;
    return (ROK);
}

static UINT8_t
codecJB2RTP(CoderInfo txCodec)
{
    switch (txCodec)
    {
      case JB_G711_ULAW:
        return RTP_AUDIO_PAYLOAD_PCMU;

      case JB_G711_ALAW:
        return RTP_AUDIO_PAYLOAD_PCMA;

      case JB_G722:
        return RTP_AUDIO_PAYLOAD_G722;

      case JB_G723:
        return RTP_AUDIO_PAYLOAD_G723;

      case JB_G726_32:
        return RTP_AUDIO_PAYLOAD_G726_32;

      case JB_G729:
        return RTP_AUDIO_PAYLOAD_G729;

      default:
        return UNDEFINED_PAYLOAD;
    }
}

/*
**
**       Fun:   JbResetJitterReq
**
**       Desc:  This function is used to reset the jitter variables
**              during the call sequence.
**
**       Ret:   ROK on success, RFAILED on failure
**
**       Notes:
**
**       File:  jb_timer.cpp
**
*/
extern INT16_t
JbResetJitterReq(UINT8_t connIndex)
{
    INT16_t i;
    JbConnCb* connCb = NULL;
    
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: enter JbResetJitterReq.\n");

    connCb = jbCb.conn[connIndex];
    if(connCb==NULL)
        return  0;
    connCb->jitter.p = 0;
    connCb->jitter.q = 0;
    connCb->jitter.gotFirst = 0;
    connCb->jitter.lastSeqNum = 0;
    connCb->jitter.curTimestamp = 0;
    connCb->jitter.receivePacketCount = 0;
    connCb->sts.numPacketTimes = 0;

    for (i = 0; i < jbCb.genCfg.maxSize; i++)
    {
        connCb->jitter.jBuf[i].len = 0;
        connCb->jitter.jBuf[i].status = PACKET_EMPTY;
    }
    connCb->jitter.state = JB_UNDERFLOW_LONG_SILENCE;

    JbStopTimer(connCb);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
#define JB_ROAM_TIME_NUM 8

static UINT32_t g_JbRoamTime = 0;
static UINT32_t g_JbRoamingFlag = 0;

extern int getActiveRtpSessionNum(void);

INT32_t JB_WfStartRoaming()
{
    VOIP_OA_LOG_PRINT(P_DEBUG, M_JITBUF, ("JB_WfStartRoaming: Enter"));

    if (getActiveRtpSessionNum() > 0)
    {
        g_JbRoamingFlag = TRUE;
        g_JbRoamTime = JB_ROAM_TIME_NUM;
    }

    return 0;
}

INT32_t JB_WfFinishRoaming()
{
    VOIP_OA_LOG_PRINT(P_DEBUG, M_JITBUF, ("JB_WfFinishRoaming: Enter"));

    g_JbRoamTime = 0;
    g_JbRoamingFlag = FALSE;

    return 0;
}

INT32_t JB_UpdateRoamTime()
{
    if (g_JbRoamTime)
    {
        g_JbRoamTime--;
    }
    else
    {
        g_JbRoamingFlag = 0;
    }

    return 0;
}

INT32_t JB_WFIsRoaming()
{
    return (INT32_t)g_JbRoamingFlag;
}


