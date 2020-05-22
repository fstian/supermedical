/*******************************************************************20**

     Nhce:     Jitter Buffer - Finite State Machine

     Type:     C source file

     Desc:     Finite State Machine file for Jitter Buffer.

     File:     jb_fsm.cpp

     Sid:      jb_fsm.c 1.0  -  01/24/2000

     Prg:      sdg

Revision History:
================

     04/08/2003  Venkat. C (HCLT)  Modified the arguements to all the
                                   functions and the corresponding
                                   changes in the code.

*********************************************************************21*/

/************************************************************************

Upper layer primitives:

The following functions are provided in this file:

    jbFSM
************************************************************************/

/* header include files (.h) */

#include "jb_def.h"        /* Jitter Buffer structures/prototypes */
#include "voip_oa_log.h"

/* local defines */

/* local typedefs */

#ifdef __cplusplus
extern "C"
{
#endif //#ifdef __cplusplus
extern void* hDlDSP[MAX_LINES];
extern void* hUlDSP[MAX_LINES];
#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#ifdef JB_PACKETS_DEBUG

#define JBSTATE(targetstate) \
    do  \
    { \
        char *fromState="?", *toState="?"; \
        switch (targetstate) \
        { \
            case JB_UNDERFLOW_LONG_SILENCE: \
                toState = "JB_UNDERFLOW_LONG_SILENCE"; \
                break; \
            case JB_ACTIVE_HOLD: \
                toState = "JB_ACTIVE_HOLD"; \
                break; \
            case JB_ACTIVE_PLAY: \
                toState = "JB_ACTIVE_PLAY"; \
                break; \
            case JB_UNDERFLOW: \
                toState = "JB_UNDERFLOW"; \
                break; \
        } \
        switch (connCb->jitter.state) \
        { \
            case JB_UNDERFLOW_LONG_SILENCE: \
                fromState = "JB_UNDERFLOW_LONG_SILENCE"; \
                break; \
            case JB_ACTIVE_HOLD: \
                fromState = "JB_ACTIVE_HOLD"; \
                break; \
            case JB_ACTIVE_PLAY: \
                fromState = "JB_ACTIVE_PLAY"; \
                break; \
            case JB_UNDERFLOW: \
                fromState = "JB_UNDERFLOW";\
                break; \
        } \
        LPT( P_WARN, ("JB:State changed from %s to %s %d\n", fromState, toState, connCb->sts.numPacketTimes));\
        connCb->jitter.state = targetstate; \
    }while (0)

#else /* JB_PACKETS_DEBUG */
#define JBSTATE(targetstate) connCb->jitter.state = targetstate
#endif /* JB_PACKETS_DEBUG */


/* Venkat @@ Not used in this file. Hence commenting.
EXTERN CdspEncoder *dspApiEncoder[DSP_MAX_ENCODER_USERS];
*/

/* forward references */
static INT16_t jbIgnore ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE00S00 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE00S01 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE00S02 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE00S03 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE01S00 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE01S01 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE01S02 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE01S03 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));
static INT16_t jbE02S03 ARGS((JbConnCb* connCb, UINT8_t* dBuf,
UINT32_t dataLen,  rtp_hdr_t* rtpHdr));

/* private variable declarations */

PFJBFSM JbFSM[NMB_JB_EVNT][NMB_JB_ST] =
{
    /* Receive Packet Event */
    {
    jbE00S00,         /* Underflow Long Silence */
    jbE00S01,         /* Active Hold */
    jbE00S02,         /* Active Play */
    jbE00S03,         /* Underflow */
    }, {
    /* Play Packet Event */
    jbE01S00,         /* Underflow Long Silence */
    jbE01S01,         /* Active Hold */
    jbE01S02,         /* Active Play */
    jbE01S03,         /* Underflow */
    }, {
    /* Long Silence Event */
    jbIgnore,         /* Underflow Long Silence */
    jbIgnore,         /* Active Hold */
    jbIgnore,         /* Active Play */
    jbE02S03,         /* Underflow */
    }
};

/*
*       Fun:   jbIgnore
*       Desc:  This function is for undefined events
*       Ret:   RFAILED
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbIgnore(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbIgnore)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: Enter jbIgnore-indicate undefined events.\n");

    return(RFAILED);
}


static int jbGetFullSlotsNum(JbConnCb* connCb)
{
    UINT16_t numFull;
    
    numFull = (UINT16_t) ((connCb->jitter.q - connCb->jitter.p + jbCb.genCfg.maxSize) & (jbCb.genCfg.maxSize - 1));

    return (int)numFull;
}

/*
*       Fun:   jbE00S00
*       Desc:  This function is for Ev=ReceivePacket St=UnderflowLongSilence
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbE00S00(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbE00S00)

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      
    if (JbInsertPacket(connCb, dBuf, dataLen, rtpHdr) != RRETRANSMIT)
    {
        JbUpdateJitter(connCb, JB_RECEIVE_PACKET, rtpHdr);
    }
    
    // if (connCb->sts.numPackets >= connCb->sts.jitterBufferSize) baojianyi 2007.02.02
    //if (connCb->sts.numPackets >= jbCb.jitCfg.minSize)
    if (connCb->sts.numPacketTimes >= jbCb.jitCfg.minSize * JB_DFLT_PTIME)
    {
        /*active play means there are a few packtes(bigger than threshhold) which can be played from now.
           otherwise the packets is not so many.sean yang.2005/7/5*/
        JBSTATE(JB_ACTIVE_PLAY);
    }
    else
    {
        JBSTATE(JB_ACTIVE_HOLD);
    }


    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

    return(ROK);
}

/*
*       Fun:   jbE00S01
*       Desc:  This function is for Ev=ReceivePacket St=ActiveHold
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbE00S01(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbE00S01)

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      
    if (JbInsertPacket(connCb, dBuf, dataLen, rtpHdr) != RRETRANSMIT)
    {
        JbUpdateJitter(connCb, JB_RECEIVE_PACKET, rtpHdr);
    }
    // if (connCb->sts.numPackets >= connCb->sts.jitterBufferSize) baojianyi 2007.02.02
    // if (connCb->sts.numPackets >= jbCb.jitCfg.minSize)
    if (connCb->sts.numPacketTimes >= (jbCb.jitCfg.minSize - 1)* JB_DFLT_PTIME)
    {
        JBSTATE(JB_ACTIVE_PLAY);
    }

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    
    return(ROK);
}

/*
*       Fun:   jbE00S02
*       Desc:  This function is for Ev=ReceivePacket St=ActivePlay
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbE00S02(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbE00S02)

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      
    if (JbInsertPacket(connCb, dBuf, dataLen, rtpHdr) != RRETRANSMIT)
    {
        JbUpdateJitter(connCb, JB_RECEIVE_PACKET, rtpHdr);
    }
    /* in active play and receive packet, so must be activeplay state */

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    
    return(ROK);
}

/*
*       Fun:   jbE00S03
*       Desc:  This function is for Ev=ReceivePacket St=Underflow
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/

static INT16_t
jbE00S03(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbE00S03)

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      

    /* in uderflow state and recieve packet, so first stop the long silence timer*/
    JbStopTimer(connCb);

    connCb->sts.breaks++;
    if (JbInsertPacket(connCb, dBuf, dataLen, rtpHdr) != RRETRANSMIT)
    {
        JbUpdateJitter(connCb, JB_RECEIVE_PACKET, rtpHdr);
    }

    if (rtpHdr->m)
    {
        JBSTATE(JB_ACTIVE_HOLD);
    }
    
    //if (connCb->sts.numPackets >= buffersize)
    if (jbGetFullSlotsNum(connCb) >= (jbCb.jitCfg.minSize - 1))
    {
        JBSTATE(JB_ACTIVE_PLAY);
    }

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

    return(ROK);
}

/*
*       Fun:   jbE01S00
*       Desc:  This function is for Ev=PlayPacket St=UnderflowLongSilence
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbE01S00(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbE01S00)

    ASSERT(connCb != NULL);

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      

    connCb->jitter.playPacketCount++;
    /* slm: MR4213 reset buffer creep flag */
    connCb->jitter.bufferCreep = 0; //buffercreep detect receive packet quick than play packet ,count to 100 cause will  buffer drop
    connCb->jitter.bufferDown= 0; 

    JbUpdateJitter(connCb, JB_PLAY_PACKET, rtpHdr);

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

    /* slm: MR8849 changed to index based on line number */
    if (connCb->connIndex == jbCb.activeConn[connCb->key.lineNo])
    {
        /* slm: MR4386 Need semaphore to protect DSP writes */

        VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);      

        DlDSPGenerateSilence(connCb->key.lineNo, 1);

        VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);         

        // VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      
        connCb->sts.silence++;
        // VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);
    }
    return(ROK);
}

/*
*       Fun:   jbE01S01
*       Desc:  This function is for Ev=PlayPacket St=ActiveHold
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbE01S01(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbE01S01)

    ASSERT(connCb != NULL);
    
    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);        
    connCb->jitter.playPacketCount++;
    /* slm: MR4213 reset buffer creep flag */
    connCb->jitter.bufferCreep = 0;
    connCb->jitter.bufferDown= 0; 
    JbUpdateJitter(connCb, JB_PLAY_PACKET, rtpHdr);

    /* If we have ANY packets in the queue and our timestamp is >= the first packet,
    ** then play it now.
    ** This could happen if the far end is sending silence frames.
    ** In this case we underflow when empty and go to this state (active hold) after
    ** we rcv a packet.  When we're in active hold, monitor the timestamp of the 1st
    ** packet in the queue to see if we need to play it */

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

    /* slm: MR8849 changed to index based on line number */
    if (connCb->connIndex == jbCb.activeConn[connCb->key.lineNo])
    {
        /* slm: MR4386 Need semaphore to protect DSP writes */
        VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);      

        /* slm: MR4213
        ** If we're holding (because need to wait for more packets in the JB),
        ** then we should call PLC instead of silence
        ** Call PLC for 1 frame at a time
        */
        DlDSPPacketLossConcealment(connCb->key.lineNo, 1);

        VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);    

        //VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);                  
        /* number of plc calls */
        connCb->sts.plc++;            
        //VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);
    }
    return(ROK);
}

/*
*       Fun:   jbE01S02
*       Desc:  This function is for Ev=PlayPacket St=ActivePlay
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbE01S02(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    static UINT8_t ulLowPktCount = 0;
    int bufSize = 0;
    
    TRC3(jbE01S02)

    ASSERT(connCb != NULL);

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      

    /* Bump play count before updating jitter */
    connCb->jitter.playPacketCount++;
    JbUpdateJitter(connCb, JB_PLAY_PACKET, rtpHdr);

    if (connCb->jitter.p == connCb->jitter.q)
    {
        connCb->sts.numPacketTimes = 0;

        JbStartTimer(connCb);
        JBSTATE(JB_UNDERFLOW);

        VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

        VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);       

        ASSERT(hDlDSP[connCb->key.lineNo] != NULL);
        DlDSPPacketLossConcealment(connCb->key.lineNo, 1);

        VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);    

        /* number of plc calls */
        connCb->sts.plc++;

        return(ROK);
    }

#if 1
    if (connCb->jitter.bufferInsert)
    {
        connCb->jitter.bufferInsert = 0;
#ifdef JB_PACKETS_DEBUG
         VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"ActivePlay insert: p:%d q:%d num:%d size:%d\r\n", 
                connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes, connCb->sts.jitterBufferTime);
#endif

        VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

        VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);       

        ASSERT(hDlDSP[connCb->key.lineNo] != NULL);
        DlDSPPacketLossConcealment(connCb->key.lineNo, 1);

        VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);    

        return(ROK);
    }
    
    if (JB_WFIsRoaming() && (jbGetFullSlotsNum(connCb) < jbCb.jitCfg.minSize))
    {
        // 插入空包；改善后续连续的丢包?!
        ulLowPktCount++;
#ifdef JB_PACKETS_DEBUG
         VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"##ActivePlay %d: p:%d q:%d num:%d size:%d\r\n", ulLowPktCount,
                connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes, connCb->sts.jitterBufferTime);
#endif
        if ((ulLowPktCount % 0x03) == 0)
        {
            VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

            VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);       

            ASSERT(hDlDSP[connCb->key.lineNo] != NULL);
            DlDSPPacketLossConcealment(connCb->key.lineNo, 1);

            VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);    

            return(ROK);
        }
    }
#endif

    if (JbRemovePacket(connCb,dBuf,&bufSize) != ROK)
    {
        VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

        /* slm: MR4386 Need semaphore to protect DSP writes */
        VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);       
        DlDSPPacketLossConcealment(connCb->key.lineNo, 1);
        VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);    

        /* number of plc calls */
        connCb->sts.plc++;
        return (ROK);
    }

    return(bufSize);
}

/*
*       Fun:   jbE01S03
*       Desc:  This function is for Ev=PlayPacket St=Underflow
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*/
static INT16_t
jbE01S03(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    TRC3(jbE01S03)

    ASSERT(connCb != NULL);

    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      

    connCb->jitter.playPacketCount++;
    /* slm: MR4213 reset buffer creep flag */
    connCb->jitter.bufferCreep = 0;
    connCb->jitter.bufferDown= 0; 
    JbUpdateJitter(connCb, JB_PLAY_PACKET, rtpHdr);
    /* slm: clear playAttempts so we restart from 0 */
    connCb->jitter.playAttempts = 0;

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

    VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);      
    DlDSPPacketLossConcealment(connCb->key.lineNo, 1);
    VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);    

    connCb->sts.plc++;

    return(ROK);
}

/*
*       Fun:   jbE02S03
*       Desc:  This function is for Ev=LongSilence St=Underflow
*       Ret:   ROK on sucesss, RFAILED on failure
*       Notes:
*       File:  jb_fsm.c
*
*/
static INT16_t
jbE02S03(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    INT16_t i;

    TRC3(jbE02S03)

    //reset the buffer to empty
    VOIP_OA_MutexLock(jbCb.sem[connCb->connIndex]);      

    connCb->jitter.p = 0;
    connCb->jitter.q = 0;
    connCb->jitter.maxJitter = 0;
    connCb->jitter.currentStableTime = 0;
    connCb->jitter.playAttempts = 0;
    connCb->jitter.receivePacketCount = 0;
    connCb->jitter.playPacketCount = 0;
    connCb->sts.numPacketTimes = 0;

    for (i = 0; i < jbCb.genCfg.maxSize; i++)
    {
        /* slm: use len instead of seqNum since 0 is a valid seqnum
        ** connCb->jitter.jBuf[i].seqNum = 0;
        */
        connCb->jitter.jBuf[i].len = 0;
        connCb->jitter.jBuf[i].status = PACKET_EMPTY;
    }

    JBSTATE(JB_UNDERFLOW_LONG_SILENCE);

    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

    return(ROK);
}

