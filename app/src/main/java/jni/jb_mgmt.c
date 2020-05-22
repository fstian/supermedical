
/********************************************************************20**

     Nhce:     Jitter Buffer - management

     Type:     C source file

     Desc:     Management file for Jitter Buffer.  This file is the
               interface to the Wrapper Module

     File:     jb_mgmt.c

     Sid:      jb_mgmt.c 1.0  -  01/21/2000

     Copyright (c) 2003 Agere Systems Inc.
     All Rights Reserved

     This is unpublished proprietary information of Agere Systems Inc.
     This copyright notice does not evidence publication.

     The use of the software, documentation, methodologies and other
     information contained herein is governed solely by the associated
     license agreements.  Any inconsistent use shall be deemed to be a
     misappropriation of the intellectual property of Agere Systems Inc.
     and treated accordingly.

     Prg:      sdg

     Revision History
     11/06/2000         mwb             Fixed latency value in JbConReq()

     04/08/2003         Venkat.C(HCLT)  Modified the file to integrate with
                    HCLT RTP library and buffer alloc
                    Mechanism

*********************************************************************21*/

/************************************************************************
  Upper layer primitives:

  The following functions are provided in this file:

  JbConfigReq      Config Request
  JbCntrlReq        Control Request
  JbConReq         Connect Request
  JbDiscReq        Disconnect Request
  JbStaReq         Statistics Request
  jbActvInit       Task Init routine

*************************************************************************/

/* header include files (.h) */

#include "jb_def.h"            /* Jitter Buffer structures/prototypes */
//#include "voip_oa_types.h"   /* CAPI data types inclusion file */
//#include "voip_oa_event.h"   /* CAPI events inclusion file */
#include "rtp_interface.h" /* rtp interface functions inclusion file */
//#include "biosapi.h"
#include "voip_oa_log.h"

extern void CloseRTPSession(UINT32_t sessid);

void* hDlDSP[MAX_LINES] =
{
    NULL
};
void* hUlDSP[MAX_LINES] =
{
    NULL
};

typedef struct
{
    int rtp_port;
    unsigned int rtp_sessid;
    int dspGetTid;
    int dspPutTid;
    int socketReadTid;
}JBPARAMTABLE;

JBPARAMTABLE jbParamTable[2];

/* Forward Declarations*/
static void codec2numframes(UINT8_t payloadtype, UINT16_t framesize, UINT32_t* encRate);
Bool_t findConnId(UINT8_t* connId, UINT32_t sessionId);

/* public variable declarations */
int fRequestjbReadTaskStop[MAXIMUM_CONNECTIONS] =
{
    0
};
int fRequestGetMgrTaskStop[MAXIMUM_CONNECTIONS] =
{
    0
};
int fRequestPutMgrTaskStop[MAXIMUM_CONNECTIONS] =
{
    0
};

JbCb_t jbCb;
INT32_t jbTId = 0;

/*
**
**       Fun:   jbActvInit
**
**       Desc:  This function is used to initialize the JitterBuffer
**
**       Ret:   Nothing
**
**       Notes:
**
**       File:  jb_mgmt.c
**
**/
extern INT16_t
jbActvInit()                  /* init the jitterbuffer module */
{
    INT16_t i;

    TRC3(jbActvInit)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:Enter jbActvInit\n");

    VOIP_OA_MEMSET(&jbCb, 0, sizeof(jbCb));
    jbCb.genCfg.maxConnections = JB_DFLT_JBMaxConn ;
    jbCb.genCfg.maxSize = JB_DFLT_JBBufSize ;

    jbCb.jitCfg.maxLatency = JB_DFLT_JBMaxLatency;
    jbCb.jitCfg.maxHoldTime = JB_DFLT_JBMaxHoldThreshold;
    jbCb.jitCfg.minSize = JB_DFLT_JBMinBufSize;
    jbCb.jitCfg.minStableTime = JB_DFLT_JBMinStableEvent;
    jbCb.jitCfg.incAmount = JB_DFLT_JBGrowRate;
    jbCb.jitCfg.decAmount = JB_DFLT_JBShrinkRate;
    jbCb.jitCfg.longSTimerVal = JB_DFLT_JBSilencTime;



    jbCb.genCfg.cfgDone = FALSE;
    jbCb.timerRes = JB_DEFAULT_TIMER_RES;


    for (i = 0; i < MAXIMUM_CONNECTIONS; i++)
    {
        jbCb.sem[i] = VOIP_OA_INVALID_HANDLE_MUTEX;
        if (VOIP_OA_SUCCESS != VOIP_OA_MutexCreate(&jbCb.sem[i]))
        {
            VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "JB: %s[%d]:MutexCreate fail\n", __FUNCTION__, __LINE__);

            VOIP_OA_ASSERT(0);
        }
    }

    for (i = 0; i < MAX_LINES ; i++)
    {
        jbCb.dspSem[i] = VOIP_OA_INVALID_HANDLE_MUTEX;
        if (VOIP_OA_SUCCESS != VOIP_OA_MutexCreate(&jbCb.dspSem[i]))
        {
            VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "%s[%d]:MutexCreate fail\n", __FUNCTION__, __LINE__);
            VOIP_OA_ASSERT(0);
        }
    }

    /* Initialize active connections to -1 */
    for (i = 0; i < MAX_LINES; i++)
    {
        jbCb.activeConn[i] = -1;
        hDlDSP[i] = hUlDSP[i] = NULL;
    }

    /* Initialize connection array of pointers to NULL */
    for (i = 0; i < MAXIMUM_CONNECTIONS; i++)
    {
        jbCb.conn[i] = NULL;
    }

    for (i = 0; i < sizeof(jbParamTable) / sizeof(JBPARAMTABLE); i++)
    {
        jbParamTable[i].dspGetTid = ERROR;
        jbParamTable[i].rtp_port = 0;
        jbParamTable[i].dspPutTid = ERROR;
        jbParamTable[i].socketReadTid = ERROR;
        jbParamTable[i].rtp_sessid = RTP_INVALID_SESSID;
    }

    return(ROK);
} /* jbActvInit */

/*  To be added for uninit*/
extern void
jbuninit()
{
    int i = 0;
    for (i = 0; i < MAXIMUM_CONNECTIONS; i++)
    {
        if (NULL != jbCb.sem[i] && VOIP_OA_INVALID_HANDLE_MUTEX != jbCb.sem[i])
        {
            VOIP_OA_MutexDelete(&jbCb.sem[i]);
            jbCb.sem[i] = NULL;
        }
    }

    for (i = 0; i < MAX_LINES ; i++)
    {
        if (NULL != jbCb.dspSem[i] && VOIP_OA_INVALID_HANDLE_MUTEX != jbCb.dspSem[i])
        {
            VOIP_OA_MutexDelete(&jbCb.dspSem[i]);
            jbCb.dspSem[i] = NULL;
        }
    }
}


/*
**
**       Fun:   JbCfgReq
**
**       Desc:  This function is used by the wrapper to
**              configure the Jitter Buffer.
**
**       Ret:   ROK on success and RFAILED on failure
**
**       Notes:
**
**       File:  jb_mgmt.c
**
*/
extern INT16_t
JbCfgReq(JbCfg* cfg)
{
    INT16_t i;

    TRC3(JbCfgReq)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:Enter JbCfgReq\n");

    switch (cfg->type)
    {
      case GEN:
        /* general configuration request */
        VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:Enter JbCfgReq  case GEN\n");
        if (jbCb.genCfg.cfgDone || cfg->c.genCfg.maxSize % 2 != 0)
        {
            return(RFAILED);
        }

        VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:Enter JbCfgReq  case GEN1\n");
        jbCb.genCfg.cfgDone = TRUE;
        jbCb.genCfg.maxConnections = cfg->c.genCfg.maxConnections;

        if (cfg->c.genCfg.maxSize % 2 != 0)
        {
            VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:JbCfgReq-cfg->c.genCfg.maxSize%2 != 0 \n");          
            VOIP_OA_ASSERT(0);
        }

        jbCb.genCfg.maxSize = cfg->c.genCfg.maxSize;


        for (i = 0; i < jbCb.genCfg.maxConnections; i++)
        {
            jbCb.conn[i] = NULL;
        }
        VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:Enter JbCfgReq  case GEN2\n");
        break;

      case JIT:
        /* buffer configuration request */

        jbCb.jitCfg.maxLatency = cfg->c.jitterCfg.maxLatency;
        jbCb.jitCfg.maxHoldTime = cfg->c.jitterCfg.maxHoldTime;
        jbCb.jitCfg.minSize = cfg->c.jitterCfg.minSize;
        jbCb.jitCfg.minStableTime = cfg->c.jitterCfg.minStableTime;
        jbCb.jitCfg.incAmount = cfg->c.jitterCfg.incAmount;
        jbCb.jitCfg.decAmount = cfg->c.jitterCfg.decAmount;
        jbCb.jitCfg.longSTimerVal = cfg->c.jitterCfg.longSTimerVal;
        break;
    }
    return(ROK);
} /* JbCfgReq */

/*
**
**       Fun:   JbCntrlReq
**
**       Desc:  This function is used by the Wrapper to
**              set the active connection
**
**       Ret:   ROK on success and RFAILED when connection
**              specified does not exist.
**
**       Notes:
**
**       File:  jb_mgmt.c
**
*/
extern INT16_t
JbCntrlReq(UINT32_t sessionId, UINT8_t mask)
{
    UINT8_t connIndex = 0;
    INT16_t lineNo;

    TRC3(JbCntrlReq)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:Enter JbCntrlReq\n");

    if (!findConnId(&connIndex, sessionId))
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n%s[%d]:Failed in fetching connection Index\n", __FUNCTION__, __LINE__);
        return(RFAILED);
    }


    VOIP_OA_MutexLock(jbCb.sem[connIndex]);


    jbCb.conn[connIndex]->conn_mask = mask;
    lineNo = jbCb.conn[connIndex]->key.lineNo;

    jbCb.activeConn[lineNo] = connIndex;

    VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);

    return(ROK);
}

/*
**
**       Fun:   getJbConMask
**
**       Desc:  This function is used by the Wrapper to
**              get the the connection mask
**
**       Ret:   Returns  connection mask
**
**       Notes:
**
**       File:  jb_mgmt.c
**
*/
extern INT8_t
getJbConMask(UINT32_t sessionId)
{
    UINT8_t connIndex = 0;
    /*    INT16_t lineNo; */
    UINT8_t connMask;

    TRC3(getJbConMask)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB:Enter getJbConMask\n");

    if (!findConnId(&connIndex, sessionId))
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, " \n JB: %s[%d]:Failed in fetching connection Index\n", __FUNCTION__, __LINE__);
        return(RFAILED);
    }


    VOIP_OA_MutexLock(jbCb.sem[connIndex]);
    connMask = jbCb.conn[connIndex]->conn_mask;
    VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);


    return(connMask);
}

/*
**
**       Fun:
**
**       Desc:  This function is used to connect
**              the jitter buffer up to an RTP session and DSP
**
**       Ret:   ROK on success, RFAILED on failure
**
**       Notes:
**
**       File:  jb_mgmt.c
**
*/
extern INT16_t
JbConReq(INT16_t lineNo,
         UINT32_t sessionId,
         UINT8_t txCodec,
         UINT8_t rxCodec,
         UINT16_t maxAlSduFrames,
         UINT16_t rem_ptime,
         UINT8_t conn_mask,
         int owner)
{
    JbConnCb* connCb = NULL; /* slm: fix compiler warning */
    INT16_t i, j, k;    /* loop variables */
    /*    UINT32_t frameSize; */
    UINT32_t encRate;
    UINT8_t connId = 0; /* slm: fix compiler warning */

    TRC3(JbConReq)
    VOIP_OA_LOG_PRINT(P_DEBUG, M_JITBUF, "JB:Enter JbConReq\n");

//    if (jbCb.sts.activeConnections >= jbCb.genCfg.maxConnections)
    {
//        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "JB: jbConReq: Max connections (%u) exceeded\n", jbCb.genCfg.maxConnections);
//        return(RFAILED);
    }

    /* go through all the connections, use the first empty one */
//    for (i = 0; (i < jbCb.genCfg.maxConnections); i++)
    i = 0;
    
    {
        connCb = NULL;
        // found a NULL entry 
        if ((jbCb.conn[i]) == (JbConnCb *) NULL)
        {
            connCb = (JbConnCb *)VOIP_OA_AllocMemory(sizeof(JbConnCb));
            if (NULL == connCb)
            {
                VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "JB: jbConReq: jbConnCb buffer alloc failed\n");
                return(RFAILED);
            }
            VOIP_OA_MEMSET(connCb, 0, sizeof(JbConnCb));
            jbCb.conn[i] = connCb;
            connId = (UINT8_t) i;

            connCb->hGetTask = VOIP_OA_INVALID_HANDLE_TASK; 
            connCb->hPutTask = VOIP_OA_INVALID_HANDLE_TASK; 
            connCb->hRdSocketTask = VOIP_OA_INVALID_HANDLE_TASK; 

            connCb->connIndex = (UINT8_t) i;
            connCb->key.sessionId = sessionId;
            connCb->key.lineNo = lineNo;
            connCb->key.txCodec = txCodec;
            connCb->key.rxCodec = rxCodec;
            if (owner == SESSION_OWNER_SIP)
            {
                connCb->jitter.maxLatency = jbCb.jitCfg.maxLatency;
                jbCb.jitCfg.minSize = JB_DFLT_JBMinBufSize;
            }
            else
            {
                connCb->jitter.maxLatency = JB_DFLT_PTT_MAX_LATENCY;
                jbCb.jitCfg.minSize = JB_DFLT_PTT_MinBufSize;
            }
            connCb->jitter.jBuf = (JBuf *)VOIP_OA_AllocMemory(jbCb.genCfg.maxSize * sizeof(JBuf));
            if (connCb->jitter.jBuf == NULL)
            {
                VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "JB: jbConReq: jBuff buffer allocation failed\n");
                VOIP_OA_FreeMem(jbCb.conn[i]);
                jbCb.conn[i] = NULL;
                return(RFAILED);
            }
            VOIP_OA_MEMSET(connCb->jitter.jBuf, 0, jbCb.genCfg.maxSize * sizeof(JBuf));

            connCb->jitter.maxAlSduFrames = maxAlSduFrames;

            for (j = 0; j < jbCb.genCfg.maxSize; j++)
            {
                connCb->jitter.jBuf[j].buffer = (UINT8_t *)VOIP_OA_AllocMemory(JB_GEN_BUF_SIZE * sizeof(UINT8_t));
                ASSERT (NULL != connCb->jitter.jBuf[j].buffer);

                //VOIP_OA_MEMSET(connCb->jitter.jBuf[j].buffer, 0, JB_GEN_BUF_SIZE * sizeof(UINT8_t));
                if(NULL == connCb->jitter.jBuf[j].buffer)
                    break;
                connCb->jitter.jBuf[j].len = 0;
                connCb->jitter.jBuf[j].status = PACKET_EMPTY;
            }

            connCb->sts.jitterBufferTime = jbCb.jitCfg.minSize * JB_DFLT_PTIME;
        }else{
            connCb = jbCb.conn[i];
            connId = (UINT8_t) i;
        }
        

    }

    if (NULL == connCb)
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "\n JB: all connections is occupied\n");
        return (RFAILED);
    }

//    jbCb.sts.activeConnections++;
//    jbCb.sts.totalConnections++;
    JbCntrlReq(sessionId, conn_mask);

    // @@ venkat: add the support for all the Agere specific codec's
    // in codec2numframes(). Here encRate is number of frames per
    // packet
    //
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "\nThe txcodec[%d]\trxCodec[%d]\n", txCodec, rxCodec);
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "\nThe maxAlSduFrames[%d]\n", maxAlSduFrames);
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "\nThe Remote Ptime[%d]\n", rem_ptime);
    codec2numframes(txCodec, maxAlSduFrames, &encRate);

    connCb->numframes = encRate;

    // reset variable that controls the task exit 
    fRequestGetMgrTaskStop[connId] = 0;

    //sprintf(tName, "tGt%d", connId);

    connCb->GetTaskParam.ConnIndex = connId;

    //reset variable that controls the task exit 
    fRequestPutMgrTaskStop[connId] = 0;

    connCb->PutTaskParam.ConnIndex = connId;
//    connCb->PutTaskParam.pEventList = NULL;

    // reset variable that controls the task exit 
    fRequestjbReadTaskStop[connId] = 0;

    connCb->RdSocketTaskParam.ConnIndex = connId;

    return connId;

}


int JbSetDirectionReq(UINT32_t sessionId, int direction)
{
    UINT8_t connIndex;
    JbConnCb* connCb;
    
    if (!vallid_rtp_session(sessionId))
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "JB: JbDiscReq - valid rtp session \n");
        return(RFAILED);
    }

    /* Get connection index */
    if (!findConnId(&connIndex, sessionId))
    {
        VOIP_OA_LOG_PRINT(P_ERROR,M_JITBUF, "JB: JbDiscReq - findConnId fail \n");
        return(RFAILED);
    }

    connCb = jbCb.conn[connIndex];

    connCb->conn_mask = direction;

    // Baojianyi 清空JB中的语音包否则下次启动对讲时会有上次的声音2009.12.30
    if ((connCb->conn_mask & CONN_RECV) == 0)
    {
        JbResetJitterReq(connIndex);
    }

    return 0;
}
/*
**       Fun:   JbDiscReq
**       Desc:  This function is used to disconnect
**              the jitter buffer from an RTP session and DSP
**       Ret:   ROK on success, RFAILED on failure
**       Notes:
**       File:  jb_mgmt.c
*/

extern INT16_t JbSuspendReq()
{
    INT16_t i;
    JbConnCb* connCb;

    for(i=0;i<MAXIMUM_CONNECTIONS;i++)
    {
        VOIP_OA_MutexLock_Ex(jbCb.sem[i]);
        connCb = jbCb.conn[i];
        if(connCb!=NULL)
        {
            if(connCb->hGetTask!=VOIP_OA_INVALID_HANDLE_TASK)
            {
                VOIP_OA_TaskSuspend(connCb->hGetTask);
            }
            if(connCb->hPutTask!=VOIP_OA_INVALID_HANDLE_TASK)
            {
                VOIP_OA_TaskSuspend(connCb->hPutTask);
            }
        }
        
        VOIP_OA_MutexUnlock_Ex(jbCb.sem[i]);
    }
    return 0;
}

extern INT16_t JbResumeReq()
{
    INT16_t i;
    JbConnCb* connCb;

    for(i=0;i<MAXIMUM_CONNECTIONS;i++)
    {
        VOIP_OA_MutexLock(jbCb.sem[i]);
        connCb = jbCb.conn[i];
        if(connCb!=NULL)
        {
            if(connCb->hGetTask!=VOIP_OA_INVALID_HANDLE_TASK)
            {
                VOIP_OA_TaskResume(connCb->hGetTask);
            }
            if(connCb->hPutTask!=VOIP_OA_INVALID_HANDLE_TASK)
            {
                VOIP_OA_TaskResume(connCb->hPutTask);
            }
        }
        
        VOIP_OA_MutexUnlock(jbCb.sem[i]);
    }
    return 0;
}

extern INT16_t
JbDiscReq(UINT32_t jbline)
{
    UINT8_t connIndex;
    JbConnCb* connCb;
    INT16_t lineNo;
    INT16_t i; /* loop variables */

    TRC3(JbDiscReq)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: Enter JbDiscReq\n");
    
    /* Check for vallid session - RTP interface */

    /* stop the jbRead & other tasks
    ** 1 - Request outstanding that the timer task stop
    ** 0 - Timer task has stopped
    **/

    /* request it to stop */

    connIndex =  jbline;
    if(connIndex>=MAXIMUM_CONNECTIONS)
        return RFAILED;
    
    VOIP_OA_MutexLock(jbCb.sem[connIndex]);
    connCb = jbCb.conn[connIndex];

    if(connCb==NULL)
        return RFAILED;
    
    lineNo = connCb->key.lineNo;

    /* Release the data buffer */
    for (i = 0; i < jbCb.genCfg.maxSize; i++) 
    {
        if(connCb->jitter.jBuf[i].buffer!=NULL){
            VOIP_OA_FreeMem(connCb->jitter.jBuf[i].buffer);
            connCb->jitter.jBuf[i].buffer = NULL;
        }
    }

    /* Release jitter buffer */
    VOIP_OA_FreeMem(connCb->jitter.jBuf);
    connCb->jitter.jBuf = NULL;

    connCb->hGetTask = VOIP_OA_INVALID_HANDLE_TASK; 
    connCb->hPutTask = VOIP_OA_INVALID_HANDLE_TASK; 
    connCb->hRdSocketTask = VOIP_OA_INVALID_HANDLE_TASK; 

    JbStopTimer( jbCb.conn[connIndex] );
    
    /* Release the connection buffer */
    VOIP_OA_FreeMem(jbCb.conn[connIndex]);
    jbCb.conn[connIndex] = NULL;

    if (jbCb.activeConn[lineNo] == connIndex)
    {
        jbCb.activeConn[lineNo] = -1;
    }
//    jbCb.sts.activeConnections--;

    VOIP_OA_MutexUnlock(jbCb.sem[connIndex]);
    
    return(ROK);
}

/*
*       Fun:   JbDatInd
*       Desc:  This function is used to indicate data
*              from the session
*       Ret:   ROK on success, RFAILED on failure
*       Notes:
*       File:  jb_mgmt.c
*/

extern INT16_t
JbDatInd( UINT8_t connIndex, rtp_hdr_t* rtpHdr, UINT8_t* dBuf, UINT32_t dataLen)
{
    JbConnCb* connCb;
    JbState state;

    TRC3(JbDatInd)
        
    connCb = jbCb.conn[connIndex];

    if(connCb==NULL)
        return RFAILED;

    state = connCb->jitter.state;

    JbFSM[JB_RECEIVE_PACKET][state](connCb, dBuf, dataLen, rtpHdr);
    return(ROK);
}

/*
*       Fun:   findConnId
*       Desc:  This function is used to locate
*              the index of sessionId in the connection array
*       Ret:   TRUE on success or FALSE on Failure
*       Notes:
*       File:  jb_mgmt.c
*/

/* @@@ Venkat: findConnId() will return connectionID for the corresponding
 *             sessionId.
 */
Bool_t
findConnId(UINT8_t* connId, UINT32_t sessionId)
{
    INT16_t i;
    UINT32_t sessId;

    TRC3(findSessionId)
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "JB: Enter findSessionId.\n");

    for (i = 0; i < jbCb.genCfg.maxConnections; i++)
    {
        if (jbCb.conn[i] == NULL)
        {
            continue;
        }

        VOIP_OA_MutexLock(jbCb.sem[i]);
        sessId = jbCb.conn[i]->key.sessionId;
        VOIP_OA_MutexUnlock(jbCb.sem[i]);

        if (sessId == sessionId)
        {
            *connId = (UINT8_t) i;
            return TRUE;;
        }
    }
    return FALSE;
}

static void
codec2numframes(UINT8_t payloadType, UINT16_t frameSize, UINT32_t* encRate)
{
    switch (payloadType)
    {
      case JB_G722:
        *encRate = frameSize / JB_G722_MS_PER_FRAME;
        break;

      case JB_G726_32:
        *encRate = frameSize / JB_G726_32_MS_PER_FRAME;
        break;

      case JB_G723:
      case JB_G729:
        *encRate = frameSize / JB_G729_MS_PER_FRAME ;
        break;

      case JB_G711_ULAW:
        *encRate = frameSize / JB_G711_ULAW_MS_PER_FRAME;
        break;

      case JB_G711_ALAW:
        *encRate = frameSize / JB_G711_ALAW_MS_PER_FRAME;
        break;

      default :
        //           DBG_PRINT( "\n%s[%d]Unsupported payload type, %d \n",
        //                    __FUNCTION__, __LINE__, payloadType);
        break;
    }
}

void
jbSetConfig()
{
    JbCfg cfg;
    cfg.type = GEN;
    cfg.c.genCfg.maxConnections = JB_DFLT_JBMaxConn;
    cfg.c.genCfg.maxSize = JB_DFLT_JBBufSize;
    JbCfgReq(&cfg);
    VOIP_OA_LOG_PRINT(P_INFO,M_JITBUF, "after jbSetConfig()\n");
}

void jbInitialize()
{
    static int initd = 0;

    if (initd > 0)
    {
        return;
    }
    initd = 1;

    jbActvInit();
    jbSetConfig();
}

void voip_init()
{
    jbInitialize();
    init_rtp(0, 0);
}

static char *GetJbStateString(int state)
{
    char *pDst = "unknow";

    switch(state){
    case 0:
        pDst = "Long Silence";
        break;
    case 1:
        pDst = "Active Hold";
        break;
    case 2:
        pDst = "Active Play";
        break;
    case 3:
        pDst = "UnderFlow";
        break;
    }
    return pDst;
}

int JbGetStatus(int jbline,char *pDst, int size)
{
    JbConnCb  *pconnCb;
    JbJitterCfg *pjbCfg = &jbCb.jitCfg;
    int pos = 0;
    
    sprintf(pDst,"JB %d \r\n",jbline);
    pos += strlen(pDst+pos);

    if(jbline>=MAXIMUM_CONNECTIONS){
        sprintf(pDst+pos,"Fail");
        pos += strlen(pDst+pos);
    }else{
        pconnCb = jbCb.conn[jbline];
        if(pconnCb==NULL){
            sprintf(pDst+pos,"Fail connCB is NULL");
            pos += strlen(pDst+pos);
        }else{
            sprintf(pDst+pos,"jitter.status = %s\r\n",GetJbStateString(pconnCb->jitter.state));
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"jitter.p = %d\r\n",pconnCb->jitter.p);
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"jitter.q = %d\r\n",pconnCb->jitter.q);
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"jitter.receivePacketCount = %d\r\n",pconnCb->jitter.receivePacketCount);
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"jitter.playPacketCount = %d\r\n",pconnCb->jitter.playPacketCount);
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"jitconfig.maxLatency = %d\r\n",pjbCfg->maxLatency);
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"jitconfig.minSize = %d\r\n",pjbCfg->minSize);
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"sts.numPacketTimes = %d\r\n",pconnCb->sts.numPacketTimes);
            pos += strlen(pDst+pos);
            sprintf(pDst+pos,"sts.jitterBufferTime = %d\r\n",pconnCb->sts.jitterBufferTime);
            pos += strlen(pDst+pos);
        }
    }
    
    return strlen(pDst);
}


