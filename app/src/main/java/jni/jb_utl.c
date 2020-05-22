/********************************************************************20**

     Nhce:     Jitter Buffer - Utilities

     Type:     C source file

     Desc:     Utilities file for Jitter Buffer.  This file contains
               support functions for the jitter buffer

     File:     jb_utl.c

     Sid:      jb_utl.c 1.0  -  01/25/2000

     Prg:      sdg

     Revision History:

     04/08/2003  Venkat.C   Modified the arguements to the functions and
                 (HCLT)     corresponding changes in the code to reflect
                            the value of the arguement.

*********************************************************************21*/

/************************************************************************

The following functions are provided in this file:

   JbInsertPacket       Insert Packet into buffer
   JbRemovePacket       Remove Packet from buffer
   JbUpdateJitter       Update Current jitter

************************************************************************/

/* header include files (.h) */

#include "jb_def.h"            /* Jitter Buffer structures/prototypes */
#include "jb.h"

//#include "voip_oa_includes.h"

#include "rtp_interface.h"
#include "voip_oa_log.h"

//#include "biosapi.h"
/* local defines */

/* local typedefs */

/* @@ Venkat: modified the arguements
 *      forward references
 */
static INT16_t jbAddPacket ARGS((
JbConnCb* connCb,
UINT16_t seq,
UINT32_t     timeStamp,
UINT8_t* dBuf,
JBuf* dest,
UINT32_t     dataSize));

UINT32_t calcPacketTime(CoderInfo payloadType, UINT32_t frameSize);
UINT32_t getDfltPacketSize(CoderInfo payloadType);
UINT32_t getDfltPackeTime(CoderInfo payloadType);

/*
*       Fun:   JbInsertPacket
*       Desc:  This function is used by the FSM to
*              insert a packet into the Jitter Buffer.
*       Ret:   ROK on success and RFAILED on failure
*       Notes:
*       File:  jb_utl.c
*/

/* @@ Venkat: modified the arguements */
extern INT16_t
JbInsertPacket(JbConnCb* connCb, UINT8_t* dBuf, UINT32_t dataLen, rtp_hdr_t* rtpHdr)
{
    UINT16_t seq;
    UINT16_t numFull;
    UINT16_t numEmpty;
    INT16_t diff;
    INT16_t dropCount;
    INT32_t i;
    UINT32_t timeStamp;

    UINT8_t nextQ;

    UINT32_t tms;

    seq = rtpHdr->seq;
    timeStamp = rtpHdr->ts;

    connCb->sts.packetsRx++;

    /* numFull is the number of currently occupied slots in our queue */
    numFull = (UINT16_t) ((connCb->jitter.q - connCb->jitter.p + jbCb.genCfg.maxSize) & (jbCb.genCfg.maxSize - 1));

    /* numEmpty is the number of available slots in our queue */
    numEmpty = (UINT16_t) (jbCb.genCfg.maxSize - numFull);

    /* diff is the delta between sequence numbers */
    /* if this is the first packet, then set the diff to one */
    if (connCb->jitter.gotFirst == 0)
    {
        connCb->jitter.gotFirst = 1;
        connCb->jitter.curTimestamp = timeStamp;
        diff = 1;
    }
    else
    {
        diff = (INT16_t) ((INT16_t) seq - (INT16_t) connCb->jitter.lastSeqNum);
    }

    if (diff == 0 && connCb->jitter.p == connCb->jitter.q)
    {
#ifdef JB_PACKETS_DEBUG                
        VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert: diff=0 && p=q, seq %d num %d Here return should be OK", 
        seq, connCb->sts.numPacketTimes);
#endif      
        return (RRETRANSMIT);
    }

    /* if we get a newer sequence number */
    if (diff >= 0)                                                  /* sean,more newer seq,that is new packet. */
    {
        // Baojianyi 2009.12.22 
        // 如果中间丢几个包同时JB为空时，有可能会导致多PLAY 几个PACKETS. I THINKING.
        if (diff > 1 && connCb->jitter.p == connCb->jitter.q)
        {
#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert: p=q, diff %d\r\n", diff);
#endif    
            if (connCb->jitter.state != JB_ACTIVE_PLAY)
            {
                diff = 1;
            }
        }

        // 这种情况下不要再插入过多空包，不要加塞；
        if (diff > 2 && numFull > 6)
        {
#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"11:diff %d p %d q %d num %d %d\r\n", diff, connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes, seq);
#endif      
            diff = 2;
        }

        // 不要插入过多的连续空包
        if (diff >= 4)
        {
#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"22:diff %d p %d q %d num %d %d\r\n", diff, connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes, seq);
#endif       
            diff = 3;
        }
    
        /* if this sequence number will fit in one of our empty queue slots */
        if (diff < numEmpty)            /* have slot to put the new packet,sean  */
        {
            /* if we get a new sequence number, but it's beyond our max latency, then
            ** we need to drop the ones we already have in the previous queue slots,
            ** so we keep our packets within the latency range */
            
            /* how many old packets need to be dropped */   
            dropCount = (INT16_t) ((numFull + diff) - (connCb->jitter.maxLatency*JB_DFLT_PTIME/connCb->jitter.packetTime));
            
            if (dropCount > 0)
            {
                for (i = 0; i < dropCount; i++)
                {
                    //测试中会到这个判断，为什么?baojianyi
                    if (connCb->jitter.p == connCb->jitter.q)
                    {
#ifdef JB_PACKETS_DEBUG                
                        VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"!!!jb insert: p:%d q:%d %d %d %d %d\r\n", 
                            connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes, dropCount, diff, i);
#endif
                        break;
                    }
                    connCb->jitter.p = (UINT8_t) ((connCb->jitter.p + 1) & (jbCb.genCfg.maxSize - 1));

                    if (connCb->jitter.jBuf[connCb->jitter.p].status == PACKET_FULL)
                    {
                        /* slm: need to decrease num packets, but don't go below 0 */
                        tms = calcPacketTime(connCb->key.rxCodec, connCb->jitter.jBuf[connCb->jitter.p].len - connCb->jitter.jBuf[connCb->jitter.p].consumeLen);
                        if (connCb->sts.numPacketTimes > tms)
                        {
                            connCb->sts.numPacketTimes -= tms;
                        }
                        else
                        {
                            connCb->sts.numPacketTimes = 0;
                        }

                        connCb->jitter.jBuf[connCb->jitter.p].len = 0;
                        connCb->jitter.jBuf[connCb->jitter.p].status = PACKET_EMPTY;
                    }
                }
                
#ifdef JB_PACKETS_DEBUG                
                 VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert: drop old packet p:%d q:%d %d %d\r\n", 
                        connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes, dropCount );
#endif             
                /* slm: MR4213 Need to adjust our play timestamp to account for the drop */
                // nextP = (UINT8_t) ((connCb->jitter.p + 1) & (jbCb.genCfg.maxSize - 1));
                // connCb->jitter.curTimestamp = connCb->jitter.jBuf[nextP].timeStamp;
            }
        }
        else /* this sequence won't fit in our slots *//*diff >= numEmpty*/
        {
            /*
            ** There is no room for this packet, either because the jitter buffer is too
            ** small or because it is currently filled with older, unplayed packets.
            ** If lastSeqNum is not updated, diff = seq-lastSeqNum will be greater than 1,
            ** the more chances that (diff>numEmpty).
            */
#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert: fail too early %d %d %d %d %d\r\n", 
                seq, connCb->jitter.lastSeqNum, connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes);
#endif
            connCb->sts.packetsDropped++;
            connCb->sts.packetsOOS++;
            connCb->jitter.lastSeqNum = seq;

            return(RFAILED);
        }

        nextQ = (UINT8_t) ((connCb->jitter.q + diff) & (jbCb.genCfg.maxSize - 1));
        ASSERT(nextQ != connCb->jitter.p);

        /* slm: use len instead of seqNum, because 0 is a valid sequence
        ** if (connCb->jitter.jBuf[connCb->jitter.q].seqNum != 0)
        */
        if (connCb->jitter.jBuf[nextQ].status == PACKET_FULL)
        {
            connCb->sts.packetsDropped++;
            /* slm: packet is redundant only if seq numbers match */
            if (connCb->jitter.jBuf[nextQ].seqNum == seq)
            {
                ASSERT(diff == 0);
                ASSERT(connCb->jitter.jBuf[nextQ].timeStamp == timeStamp);
                
#ifdef JB_PACKETS_DEBUG                
                VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert: RETRANSMIT: %d %d %d %d %d\r\n", seq, 
                    connCb->jitter.lastSeqNum, connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes);
#endif   
                connCb->sts.packetsRedundant++;
                return (RRETRANSMIT);
            }

#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert - ERROR:%d %d %d %d %d %d\r\n", seq, 
                connCb->jitter.jBuf[nextQ].seqNum, connCb->jitter.lastSeqNum, 
                connCb->jitter.p, connCb->jitter.q, connCb->sts.numPacketTimes);
#endif     
        }

        connCb->jitter.q = nextQ;
        connCb->jitter.lastSeqNum = seq;
    }
    /* if we get an older sequence number */
    else
    {			
        if (abs(diff) >= numFull)
        {
#ifdef JB_PACKETS_DEBUG                
            // For Protect Update Last Seq ?
            if (connCb->sts.numPacketTimes == 0)
            {
                connCb->jitter.lastSeqNum = seq;
            }
#endif
            connCb->sts.packetsDropped++;
            connCb->sts.packetsLate++;
#ifdef JB_PACKETS_DEBUG                
           VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert: fail too later %d %d\r\n", diff, numFull);
#endif
            return(RFAILED);
        }

        nextQ = (UINT8_t) ((connCb->jitter.q + jbCb.genCfg.maxSize + diff) & (jbCb.genCfg.maxSize - 1));

        /* slm: use len instead of seqNum, because 0 is a valid sequence
        ** if (connCb->jitter.jBuf[(connCb->jitter.q+diff+
        **    jbCb.genCfg.maxSize)&(jbCb.genCfg.maxSize-1)].seqNum!=0)
        */
        if (connCb->jitter.jBuf[nextQ].status == PACKET_FULL)
        {
            /* slm: packet is redundant only if seq numbers match */
            if (connCb->jitter.jBuf[nextQ].seqNum == seq)
            {
#ifdef JB_PACKETS_DEBUG                
                VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb insert: fail already exist 2\r\n");
#endif           
                connCb->sts.packetsDropped++;
                connCb->sts.packetsRedundant++;
                return (RRETRANSMIT);
            }
            
        }
    }
    
    jbAddPacket(connCb, seq, timeStamp, dBuf, &connCb->jitter.jBuf[nextQ], dataLen);

    return(ROK);
} /* JbInsertPacket */

/*
*       Fun:   JbRemovePacket
*       Desc:  This function is used to remove a packet from the Jitter Buffer
*              and send it to the DSP
*       Ret:   ROK on success and RFAILED on failure
*       Notes:
*       File:  jb_utl.c
*/
extern INT16_t
JbRemovePacket(JbConnCb* connCb,unsigned char *pBuf,int *pBufSize)
{
    UINT32_t bufLen;
    UINT8_t nextP;
    UINT8_t* putBuffer;
    UINT32_t tms;
    UINT32_t dfltSize;
    UINT32_t dfltTm;
    UINT8_t audioData[JB_GEN_BUF_SIZE];
    UINT32_t audioLen;
    UINT32_t audioLeft;

    /* slm: MR4213 If our jitter has dropped down, we need to drop some packets to maintain
    ** nominal path delay, because when our jitter had increased, we had to pause
    ** our play to let rcv packets build up in the queue */
    if (connCb->jitter.bufferDrop)
    {
        while (connCb->sts.numPacketTimes > jbCb.jitCfg.minSize * JB_DFLT_PTIME)
        {
            nextP = (UINT8_t) ((connCb->jitter.p + 1) & (jbCb.genCfg.maxSize - 1));
            if (connCb->jitter.p == connCb->jitter.q || nextP == connCb->jitter.q)
            {
//                ASSERT(0);
                break;
            }

#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb remove: buf drop num:%d, buf:%d flag %d p %d q %d\n", 
                connCb->sts.numPacketTimes, connCb->sts.jitterBufferTime, connCb->jitter.bufferDrop, 
                connCb->jitter.p, connCb->jitter.q);
#endif            
            if (connCb->jitter.jBuf[nextP].status == PACKET_FULL)
            {
                tms = calcPacketTime(connCb->key.rxCodec, connCb->jitter.jBuf[nextP].len - connCb->jitter.jBuf[nextP].consumeLen);
                if (connCb->sts.numPacketTimes > tms)
                {
                    connCb->sts.numPacketTimes -= tms;
                }
                else
                {
                    connCb->sts.numPacketTimes = 0;
                }

                connCb->jitter.jBuf[nextP].len = 0;
                connCb->jitter.jBuf[nextP].status = PACKET_EMPTY;
            }
            
            connCb->jitter.p = nextP;

            connCb->sts.packetsDropped++;

            connCb->jitter.bufferDrop--;
            if (connCb->jitter.bufferDrop == 0)
            {
                break;
            }
        }
    }

    if (connCb->jitter.p == connCb->jitter.q)
    {
        connCb->sts.numPacketTimes = 0;
        return RFAILED;
    }
    
    nextP = (UINT8_t) ((connCb->jitter.p + 1) & (jbCb.genCfg.maxSize - 1));
    
    if (connCb->jitter.jBuf[nextP].status == PACKET_EMPTY)
    {
        connCb->jitter.p = nextP;
        connCb->sts.packetsLost++;
#ifdef JB_PACKETS_DEBUG                
        VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF,"jb remove: packet loss \r\n");
#endif

        if (connCb->jitter.p == connCb->jitter.q)
        {
#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF,"jb remove: p = q \r\n");
#endif
            return RFAILED;
        }

        // IF packet's time is 10MS, Check & Consume Next buffer.
        if (connCb->jitter.packetTime != JB_10MS_PTIME)
        {
            return RFAILED;
        }
        
        nextP = (UINT8_t) ((connCb->jitter.p + 1) & (jbCb.genCfg.maxSize - 1));
        if (connCb->jitter.jBuf[nextP].status == PACKET_EMPTY)
        {
#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF,"jb remove: next p empty \r\n");
#endif
            connCb->jitter.p = nextP;
            return RFAILED;
        }

        if (connCb->sts.numPacketTimes > JB_10MS_PTIME)
        {
            connCb->sts.numPacketTimes -= JB_10MS_PTIME;
        }
        else
        {
            connCb->sts.numPacketTimes = 0;
        }

        audioLen = connCb->jitter.jBuf[nextP].len - connCb->jitter.jBuf[nextP].consumeLen;
        memcpy(audioData, connCb->jitter.jBuf[nextP].buffer + connCb->jitter.jBuf[nextP].consumeLen, audioLen);
        memcpy(&audioData[audioLen], connCb->jitter.jBuf[nextP].buffer + connCb->jitter.jBuf[nextP].consumeLen, audioLen);
        audioLen += audioLen;
        putBuffer = audioData;

        connCb->jitter.jBuf[nextP].len = 0;
        connCb->jitter.jBuf[nextP].status = PACKET_EMPTY;
        connCb->jitter.p = nextP;
        
        VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

        /* slm: MR4386 Need semaphore to protect DSP writes */
        VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);      

        DlDSPPutBuffer(putBuffer, audioLen, connCb->key.lineNo);
        memcpy(pBuf,putBuffer,audioLen);
        *pBufSize = audioLen;

        VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);

        return ROK;
    }
    
    // GET CONTINUOUS 20MS AUDIO DATA
    dfltTm = getDfltPackeTime(connCb->key.rxCodec);
    dfltSize = getDfltPacketSize(connCb->key.rxCodec);
    putBuffer = NULL;
    audioLen = 0;
    
    do
    {
        bufLen = connCb->jitter.jBuf[nextP].len - connCb->jitter.jBuf[nextP].consumeLen;
        if (bufLen == dfltSize)
        {
            if (connCb->sts.numPacketTimes > dfltTm)
            {
                connCb->sts.numPacketTimes -= dfltTm;
            }
            else
            {
                connCb->sts.numPacketTimes = 0;
            }

            putBuffer = connCb->jitter.jBuf[nextP].buffer + connCb->jitter.jBuf[nextP].consumeLen;
            audioLen = bufLen;

            connCb->jitter.jBuf[nextP].len = 0;
            connCb->jitter.jBuf[nextP].status = PACKET_EMPTY;
            connCb->jitter.p = nextP;
        }
        else if (bufLen > dfltSize)
        {
            if (connCb->sts.numPacketTimes > dfltTm)
            {
                connCb->sts.numPacketTimes -= dfltTm;
            }
            else
            {
                connCb->sts.numPacketTimes = 0;
            }

            putBuffer = connCb->jitter.jBuf[nextP].buffer + connCb->jitter.jBuf[nextP].consumeLen;
            audioLen = dfltSize;
            // Update Comsume Data.
            connCb->jitter.jBuf[nextP].consumeLen = dfltSize;
        }
        else
        {
            tms = calcPacketTime(connCb->key.rxCodec, bufLen);
            if (connCb->sts.numPacketTimes > tms)
            {
                connCb->sts.numPacketTimes -= tms;
            }
            else
            {
                connCb->sts.numPacketTimes = 0;
            }

            memcpy(audioData, connCb->jitter.jBuf[nextP].buffer + connCb->jitter.jBuf[nextP].consumeLen, bufLen);
            
            connCb->jitter.jBuf[nextP].len = 0;
            connCb->jitter.jBuf[nextP].status = PACKET_EMPTY;
            connCb->jitter.p = nextP;

            putBuffer = audioData;
            audioLen = bufLen;
            
            if (connCb->jitter.p == connCb->jitter.q)
            {
                ASSERT(audioLen == (dfltSize/2));
                memcpy(putBuffer + audioLen, putBuffer, audioLen);
                audioLen = dfltSize;
                connCb->sts.numPacketTimes = 0;
                break;
            }
        
            nextP = (UINT8_t) ((connCb->jitter.p + 1) & (jbCb.genCfg.maxSize - 1));
                    
            if (connCb->jitter.jBuf[nextP].status == PACKET_EMPTY)
            {
                if (connCb->jitter.packetSize < dfltSize)
                {
                    // consume this packet. 
                    connCb->jitter.p = nextP;
                }                
                memcpy(putBuffer + audioLen, putBuffer, audioLen);
                audioLen = dfltSize;
                break;
            }

            audioLeft = dfltSize - audioLen;
            bufLen = connCb->jitter.jBuf[nextP].len - connCb->jitter.jBuf[nextP].consumeLen;
            if (bufLen == audioLeft)
            {
                tms = calcPacketTime(connCb->key.rxCodec, audioLeft);
                if (connCb->sts.numPacketTimes > tms)
                {
                    connCb->sts.numPacketTimes -= tms;
                }
                else
                {
                    connCb->sts.numPacketTimes = 0;
                }

                connCb->jitter.jBuf[nextP].len = 0;
                connCb->jitter.jBuf[nextP].status = PACKET_EMPTY;
                connCb->jitter.p = nextP;

                memcpy(putBuffer + audioLen, connCb->jitter.jBuf[nextP].buffer + connCb->jitter.jBuf[nextP].consumeLen, audioLeft);
                audioLen += audioLeft;
            }
            else if (bufLen > audioLeft)
            {
                tms = calcPacketTime(connCb->key.rxCodec, audioLeft);
                if (connCb->sts.numPacketTimes > tms)
                {
                    connCb->sts.numPacketTimes -= tms;
                }
                else
                {
                    connCb->sts.numPacketTimes = 0;
                }

                memcpy(putBuffer + audioLen, connCb->jitter.jBuf[nextP].buffer + connCb->jitter.jBuf[nextP].consumeLen, audioLeft);
                audioLen += audioLeft;

                // Update Comsume Data.
                connCb->jitter.jBuf[nextP].consumeLen = audioLeft;
            }
            else
            {
                ASSERT(0);
            }
        }
    }while (0);

    connCb->jitter.curTimestamp += dfltTm;
        
    VOIP_OA_MutexUnlock(jbCb.sem[connCb->connIndex]);    

    /* slm: MR4386 Need semaphore to protect DSP writes */
    VOIP_OA_MutexLock(jbCb.dspSem[connCb->key.lineNo]);      

    DlDSPPutBuffer(putBuffer, audioLen, connCb->key.lineNo);
    memcpy(pBuf,putBuffer,audioLen);
    *pBufSize = audioLen;

    VOIP_OA_MutexUnlock(jbCb.dspSem[connCb->key.lineNo]);

    return(ROK);
} /* JbRemovePacket */

/*
*       Fun:   JbUpdateJitter
*       Desc:  This function is used to update the buffer parameters
*       Ret:   ROK on success and RFAILED on failure
*       Notes:
*       File:  jb_utl.c
*/
extern INT16_t
JbUpdateJitter(JbConnCb* connCb, JbEvent event, rtp_hdr_t* rtpHdr)
{
    UINT8_t currentJitter = 0; /* slm: fix compiler warning */
    INT32_t tempSize;
    UINT32_t jitterTime;

    TRC3(JbUpdateJitter)

    if (connCb->jitter.gotFirst == 0)
    {
        connCb->jitter.receivePacketCount = 0;
        return 0;
    }

    /* current jitter should be 1 */
    currentJitter = (UINT8_t)abs(connCb->jitter.receivePacketCount - connCb->jitter.playPacketCount);

    if (event == JB_RECEIVE_PACKET)
    {
        //如果对方有VAD功能，这个判断JITTER是有问题的
        //需要增加rtpHeader->m标志的判断。baojianyi 2009.12.23
        if (rtpHdr && rtpHdr->m)
        {
#ifdef JB_PACKETS_DEBUG                
            VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb update: m = 1, %d %d %d\r\n", 
                connCb->jitter.playPacketCount, connCb->sts.numPacketTimes, connCb->sts.jitterBufferTime);
#endif
            connCb->jitter.playPacketCount = 0;
            return (ROK);
        }

        if (connCb->jitter.playPacketCount == 0)
            return(ROK);

        connCb->jitter.playPacketCount = 0;
    }
    else
    {
        if (connCb->jitter.receivePacketCount == 0)
            return(ROK);
        connCb->jitter.receivePacketCount = 0;
    }
         
#ifdef JB_PACKETS_DEBUG                
//        VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb update: n %d j %d s %d time %d\r\n", 
//        				connCb->sts.numPacketTimes, currentJitter, connCb->sts.jitterBufferTime,
//        				connCb->jitter.currentStableTime);
#endif      

    connCb->jitter.maxJitter = (UINT8_t) MAX(currentJitter, connCb->jitter.maxJitter);

    connCb->sts.currentJitter = currentJitter;

    if (event == JB_RECEIVE_PACKET)
    {
        jitterTime = JB_DFLT_PTIME * currentJitter;
    }
    else
    {
        jitterTime = connCb->jitter.packetTime * currentJitter;
    }
    
    /* BufferSize is the threshhold for adaptive buffer */
    if (jitterTime > connCb->sts.jitterBufferTime)
    {
        tempSize = MULTPERCENT(jitterTime, jbCb.jitCfg.incAmount);

        if (jitterTime > connCb->jitter.maxLatency * JB_DFLT_PTIME)
        {
            tempSize = connCb->jitter.maxLatency * JB_DFLT_PTIME;
        }

        connCb->sts.jitterBufferTime = tempSize;

        connCb->jitter.currentStableTime = 0;
        connCb->jitter.bufferCreep = 0;
        connCb->jitter.bufferDown= 0; 
#ifdef JB_PACKETS_DEBUG                
        VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"XXENLARGE SIZE: num %d jit %d size %d\r\n", 
        				connCb->sts.numPacketTimes, currentJitter, connCb->sts.jitterBufferTime );
#endif      
    }
    else /* current jitter is <= jitter Buffer Size */
    {
        /* Bump stable time only if it hasn't already hit the limit (prevent rollover) */
        if (connCb->jitter.currentStableTime < jbCb.jitCfg.minStableTime)
        {
            connCb->jitter.currentStableTime++;
        }
        
        if (connCb->jitter.currentStableTime >= jbCb.jitCfg.minStableTime)
        {
            // tempSize = MULTPERCENT(connCb->sts.jitterBufferSize, jbCb.jitCfg.decAmount);
            tempSize = connCb->sts.jitterBufferTime - JB_DFLT_PTIME; 
            if (tempSize >= jbCb.jitCfg.minSize * JB_DFLT_PTIME)
            {
#ifdef JB_PACKETS_DEBUG                              
                VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,
                                  "JB:j<b jit:%u buf:%u, tem:%d, num:%d\n",
                                  currentJitter,
                                  connCb->sts.jitterBufferTime,
                                  tempSize,
                                  connCb->sts.numPacketTimes);
#endif
                connCb->sts.jitterBufferTime = tempSize;
                connCb->jitter.currentStableTime = 0;
                /* MR4213
                ** our jitter has dropped down, so we may need to adjust
                ** the number of packets in our queue in order to maintain
                ** minimal the path delay */
                if (connCb->sts.numPacketTimes > (connCb->sts.jitterBufferTime + JB_DFLT_PTIME))
                {
                    connCb->jitter.bufferDrop = 1;
                }
                connCb->jitter.bufferCreep = 0;
                connCb->jitter.bufferDown = 0;
            }
            /* check for buffer creep */
            /* If we are actively playing packets and the number of packets in the JB is
            ** growing bigger than the jitterbuffer size, it means we are rcving packets
            ** at a slightly higher rate than we are playing them out.  We need to adjust
            ** for this creep by dropping a buffer */
            else if ((event == JB_PLAY_PACKET) &&
                     (connCb->jitter.state == JB_ACTIVE_PLAY) )
            {
                if (connCb->sts.numPacketTimes > (connCb->sts.jitterBufferTime + JB_DFLT_PTIME))
                {
                    connCb->jitter.bufferCreep++;
                    connCb->jitter.bufferDown = 0;
                    if (connCb->jitter.bufferCreep > 80)
                    {
                        connCb->jitter.bufferDrop = 1;
                        connCb->jitter.bufferCreep = 0;
#ifdef JB_PACKETS_DEBUG                                
                        VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb update: playing packet slow creep to 60, bufferDrop==1 num: %d, buf %d p %d q %d\r\n",
    					connCb->sts.numPacketTimes, connCb->sts.jitterBufferTime, connCb->jitter.p, connCb->jitter.q);
#endif
                    }
                }
                else if (connCb->sts.numPacketTimes < connCb->sts.jitterBufferTime)
                {
                    connCb->jitter.bufferDown++;
                    connCb->jitter.bufferCreep = 0;
                    if (connCb->jitter.bufferDown > 60)
                    {
                        connCb->jitter.bufferInsert = 1;
                        connCb->jitter.bufferDown = 0;
#ifdef JB_PACKETS_DEBUG                                
                        VOIP_OA_LOG_PRINT(P_WARN,M_JITBUF,"jb update: playing packet slow down to 60, bufferinsert =1 num: %d, buf %d, p %d, q %d\r\n",
    					connCb->sts.numPacketTimes, connCb->sts.jitterBufferTime, connCb->jitter.p, connCb->jitter.q);
#endif
                    }
                }
            }
        }
    }

    return(ROK);
} /* JbUpdateJitter */

/*
*       Fun:   jbAddPacket
*       Desc:  This function add a packet to the buffer
*       Ret:   ROK on success and RFAILED on failure
*       Notes:
*       File:  jb_utl.c
*/

/* @@ Venkat: Modified the arguements */
static INT16_t
jbAddPacket(JbConnCb* connCb, UINT16_t seq, UINT32_t timeStamp, UINT8_t* dBuf, JBuf* dest, UINT32_t  dataSize)
{
    UINT32_t tm;
    
    TRC3(jbAddPacket)
    
    connCb->jitter.receivePacketCount++;
    
    connCb->sts.numPacketTimes += calcPacketTime(connCb->key.rxCodec, dataSize);;
    
    dest->status = PACKET_FULL;
    dest->len = dataSize;
    dest->consumeLen = 0;

    VOIP_OA_MEMCPY(dest->buffer, dBuf, dataSize);
    dest->seqNum = seq;
    dest->timeStamp = timeStamp;
    return(ROK);
} /* jbAddPacket */

/* GET rtp Packet time(MS) */
UINT32_t calcPacketTime(CoderInfo payloadType, UINT32_t frameSize)
{
    UINT32_t tms;
    
    ASSERT(frameSize);
    switch (payloadType)
    {
      case JB_G729:
        tms = frameSize/JB_G729_SIZE_PER_1MS;
        break;

      case JB_G711_ULAW:
        tms = frameSize/JB_G711_SIZE_PER_1MS;
        break;

      case JB_G711_ALAW:
        tms = frameSize/JB_G711_SIZE_PER_1MS;
        break;

      default :
        ASSERT(0);
        break;
    }
    
    ASSERT(tms >= 10 && tms <= 40);
    return tms;
}

UINT32_t getDfltPacketSize(CoderInfo payloadType)
{
    UINT32_t size;
    
    switch (payloadType)
    {
      case JB_G729:
        size = RTP_G729_20MS_PAYLOAD_SIZE;
        break;

      case JB_G711_ULAW:
      case JB_G711_ALAW:
        size = RTP_G711_20MS_PAYLOAD_SIZE;
        break;

      default :
        ASSERT(0);
        break;
    }
    return size;
}

UINT32_t getDfltPackeTime(CoderInfo payloadType)
{
    UINT32_t tms;
    
    switch (payloadType)
    {
      case JB_G729:
        tms = RTP_G729_20MS_PAYLOAD_SIZE/JB_G729_SIZE_PER_1MS;
        break;

      case JB_G711_ULAW:
        tms = RTP_G711_20MS_PAYLOAD_SIZE/JB_G711_SIZE_PER_1MS;
        break;

      case JB_G711_ALAW:
        tms = RTP_G711_20MS_PAYLOAD_SIZE/JB_G711_SIZE_PER_1MS;
        break;

      default :
        ASSERT(0);
        break;
    }
    return tms;
}

