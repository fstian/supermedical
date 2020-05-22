
/********************************************************************20**

     Name:    JitterBuffer

     Type:    C include file

     Desc:    Structures, variables and typedefs required by H.323
              control layer

     File:    jb.x

     Sid:     jb.x 1.0  -  01/19/2000

     Prg:     sdg

 Revision History:

  04/08/2003    Venkat.C (HCLT)    Modified the structures to integrate with
                                   HCLT MGCP Component.

************************************************************************21*/


#ifndef _JB_X_
#define _JB_X_



#include "jb.h"


/* include all the module specific files */
//#include "voip_oa_includes.h"
#include "rtp_private.h"
//#include "dspinterface.h"
#include "voip_oa_log.h"

#define ARGS(m)            			m
#define EXTERN 						extern

#define MAX_CONNECTIONS_PER_LINE   	(1)
#define MAX_LINES  					      (2)
#define MAXIMUM_CONNECTIONS 			((MAX_CONNECTIONS_PER_LINE) * (MAX_LINES))

#define RFAILED                 (-1)
#define ROK			(1)
#define RRETRANSMIT (2)


#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#ifndef NOERR
#define NOERR  0
#endif

#ifndef ERROR
#define ERROR -1
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))         /* get maximum value */
#endif //MAX

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif 

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


/***********************************************************************
                 Gen Config Structure
 ***********************************************************************/

typedef struct jbGenCfg
{
    Bool_t     cfgDone;            /* Flag to say if GenCfg is done */
    INT16_t     maxConnections;     /* Max number of active conn */
    INT16_t     maxSize;            /* Max size of buffer in packets
                                       (must be a power of 2) */
} JbGenCfg;

/***********************************************************************
                 Buffer Config Structure
 ***********************************************************************/

typedef struct jbJitterCfg
{
    UINT16_t     maxLatency;         /* Maximum level the Jitter Buffer
                                    	is allowed to fill in packets */
    UINT16_t     maxHoldTime;        /* Maximum number of playAttempts to wait
                                    	before releasing a packet, represented
                                    	as a percentage of the HWM */
    UINT16_t     minSize;            /* Minimum size of the Buffer in packets */
    UINT16_t      minStableTime;      /* Minimum time the jitter needs to be less
                                    	than the HWM before the HWM is reduced*/
    UINT16_t     incAmount;          /* Used to increase the HWM */
    UINT16_t     decAmount;          /* Used to decrease the HWM */
	UINT16_t	 longSTimerVal;		 /*  Used to determibe the cause
									  *  for Underflow */
} JbJitterCfg;

/***********************************************************************
                 Buffer Structure
 ***********************************************************************/
typedef enum
{
	PACKET_EMPTY = 0,
	PACKET_FULL,
}JB_BUF_STATUS;


typedef struct jBuf
{
    JB_BUF_STATUS	status;		
    UINT32_t     seqNum;             /* sequence number of packet. */
    UINT32_t     len;                /* length of data in buffer */
    UINT32_t     consumeLen;               /* indicates current consume audio data */
    UINT32_t     timeStamp;          /* timestamp of packet */
    UINT8_t      *buffer;            /* pointer to audio data buffer */
} JBuf;

/***********************************************************************
                 Jitter Structure
 ***********************************************************************/

typedef struct jitter
{
    UINT8_t      p;                  /* play index in buffer */
    UINT8_t      q;                  /* put index in buffer */
    UINT8_t      maxJitter;          /* max jitter in currentStableTime */
    UINT8_t      playAttempts;       /* number playAtmpts in ActiveHold state */
    UINT8_t      receivePacketCount; /* number of received packets in a row */
    UINT8_t      playPacketCount;    /* number of played packet in a row */
    UINT8_t      gotFirst;           /* got first pkt */
    UINT8_t      bufferDrop;         /* flag to indicate drop buff */
    UINT8_t      bufferCreep;        /* counter to watch for buff creep */
    UINT8_t      bufferInsert;         /* flag to indicate insert buff */
    UINT8_t      bufferDown;        /* counter to watch for buff down */
    UINT16_t      currentStableTime;  /* time jitter has been less than hwm */
    UINT16_t     lastSeqNum;         /* last seq# received */
    UINT16_t     maxLatency;         /* max latency in packets */
    UINT16_t     dropWait;           /* wait count for stable pack before drop*/
    UINT16_t     maxAlSduFrames;     /* max frames */
    UINT32_t   	 curTimestamp; 	     /* current timestamp */
    //DataHandle_t *	 jitterBuf;	     /* return of BuffAlloc for JBuf */
    JBuf 	 *jBuf;              /* pointer to the jitter buffer, is a
                                        read pointer in jitterBuf */
    JbState 	 state;              /* jitter state */
    UINT32_t            packetSize; /* size of PER remote packet(byte)*/
    UINT32_t            packetTime; /* time of PER remote packet(MS)*/
} Jitter;

/***********************************************************************
                 Connection stats
 ***********************************************************************/

typedef struct jbConnSts
{
    INT32_t  packetsTx;          /* Transmitted packets to  DSP*/
    INT32_t  packetsRx;          /* Received packets from  RTP*/
    INT32_t  packetsLate;        /* This gets incremented when there
                                    isn't room to put a packet before
                                    the q pointer */
    INT32_t  packetsDropped;     /* Total number of packets dropped */
    INT32_t  packetsNewSeq;      /* Total number of packets dropped due to new seq# */
    INT32_t  packetsOOS;         /* Total number of packets dropped due to out of seq */
    INT32_t  packetsLost;        /* This gets incremented when the buffer
                                    tries to play an empty packet */
    INT32_t  packetsRedundant;   /* This gets incremented when the
                                    buffer attempts to insert a packet
                                    in a non-empty slot in the buffer */
    INT32_t  breaks;             /* Total number of speech breaks */
    INT32_t  numPacketTimes;      /* Current time(MS) level of jitter buffer */
    INT32_t  jitterBufferTime;   /* Current High Water Mark (MS) */
    INT32_t  currentJitter;
    INT32_t  plc;                /* number of PLC calls */
    INT32_t  silence;            /* number of silence calls */
} JbConnSts;

/***********************************************************************
                 Connection key
 ***********************************************************************/

typedef struct jbConnKey
{
    UINT32_t    sessionId;          /* ID of current session */
    CoderInfo   txCodec;            /* Tx Codec type */
    CoderInfo   rxCodec;            /* Rx Codec type */
    INT16_t     lineNo;             /* line number */
} JbConnKey;

/***********************************************************************
                 Jitter Buffer Stats
 ***********************************************************************/

typedef struct jbCbSts
{
//    INT32_t         totalConnections;  /* Total connections since init */
//    INT32_t         activeConnections; /* Current number of connections */
    INT32_t unused;
} JbCbSts;

/*************************************************************************
                          RFC 2833 Event structure
**************************************************************************/
/*
typedef struct rfc2833_event_
{
	UINT8_t		event;
	Bool_t		edge;
	UINT8_t		volume;
	UINT16_t	duration;
//	TimerId_t   timerId;

	VoipTimer_t * hTimer;

} rfc2833_event_t;
*/
/*************************************************************************
            context for reading the rtp message from the message Q
**************************************************************************/

typedef struct rtpfd_cntxt_
{
	UINT8_t				connIndex;      /* connId  corresponding to line */
    UINT32_t            sessionId;
    INT16_t				lineNo;

} rtpfd_cntxt_t;

/*************************************************************************
                          Long Silence Timer Context
**************************************************************************/

typedef struct jbLSTCntxt
{
//	TimerId_t  timerId; 		/* Timer ID */
	UINT8_t    connId; 			/* connection Id */
//	Bool_t	   TimerFlag; 	    /* falg that will be set in jbE00S03 in Jb_fsm.c */

	VoipTimer_t * pTimer;

	int TimeTick;
	int WaitTick; 
	TimerCallbackFunc_t cb;
} JbLSTCntxt;

/***********************************************************************
                 Connection control block
 ***********************************************************************/

typedef struct taskParamStruct
{
//	OsapiEvList_t * pEventList;
	UINT8_t ConnIndex;
			
} taskParam_t;


typedef struct jbConnCb
{
#define CONN_INACTIVE 	0x00
#define CONN_RECV 		0x01
#define CONN_SEND		0x02

    UINT8_t          connIndex;      /* Index into jbCb connection array */
    JbConnKey        key;            /* Hash list key */

//    rfc2833_event_t	 tele_event;	 /* stores the values of the last received rfc2833 event packet */
    Bool_t 		 rfc2833_Tev_cancel; /* received rfc2833 timer event */
    UINT32_t		 lastTimeStamp;   /* time stamp value of the last received rfc2833 packet */
    JbLSTCntxt	     *jbLSTimer;     /* Long Silence timer context */
    JbConnSts        sts;            /* Connection stats */
    Jitter           jitter;         /* Buffer Structure */
    UINT32_t		 remote_ssrc;    /* RTP header - remote ssrc */
    UINT8_t			 conn_mask;
    UINT32_t 		numframes;		 /* Tx numframes */

    //added variable for os adaptor porting
    VOIP_OA_HANDLE_TASK hGetTask; 
    //void * pGetTaskStack;		
    taskParam_t GetTaskParam ;

    VOIP_OA_HANDLE_TASK hPutTask; 
    //void * pPutTaskStack;
    taskParam_t PutTaskParam ;

    VOIP_OA_HANDLE_TASK hRdSocketTask; 
    //void * pRdSocketTaskStack;
    taskParam_t RdSocketTaskParam ;
} JbConnCb;

/***********************************************************************
                 Management Structure
 ***********************************************************************/
typedef struct jbCfg
{
    ConfigType type;                /* type of config request */
    union
    {
        JbGenCfg    genCfg;         /* General Config Structure */
        JbJitterCfg jitterCfg;      /* Buffer Config Structure */
    } c;
} JbCfg;

typedef struct jbSts
{
    StatsType type;                 /* type of stats requested */
    Bool_t   clearConnSts;         /* Flag to clear conn stats */
    union
    {
        JbCbSts     jbCbSts;        /* Control Block Stats */
        JbConnSts   jbConnSts;      /* Connection Stats */
    } s;
} JbSts;

/***********************************************************************
                       JitterBuffer Global Control Block
 ***********************************************************************/

typedef struct jbCbStruct
{
    //DataHandle_t *connBuf[MAXIMUM_CONNECTIONS];/* return of BuffAlloc for JbConnCb*/
    JbConnCb    *conn[MAXIMUM_CONNECTIONS]; /* Array of Connections. Each member
	                                      * is a read pointer in the connBuf
									      */

    JbGenCfg      genCfg;                 /* General config structure */
    JbJitterCfg   jitCfg;                 /* Buffer config structure */
    JbCbSts       sts;                    /* Jitter Buffer System Stats */
    INT8_t        activeConn[MAX_LINES];
    INT16_t       timerRes;


	VOIP_OA_HANDLE_MUTEX sem[MAXIMUM_CONNECTIONS];
	
	VOIP_OA_HANDLE_MUTEX      dspSem[MAX_LINES]; /* protect accesses
                                            to DSP from multiple writes */
#if 0
	VoipTimer_t *activeTimer[MAXIMUM_CONNECTIONS];
#endif											
} JbCb_t;

typedef INT16_t (*PFJBFSM) ARGS((JbConnCb *connCb, UINT8_t *mBuf,
								 UINT32_t dataLen,  rtp_hdr_t *rtpHdr));
EXTERN PFJBFSM JbFSM[NMB_JB_EVNT][NMB_JB_ST];

/***************************************************************************
                          External References
****************************************************************************/

/*--------------------------------------------------------------------
                          Global variables
 ---------------------------------------------------------------------*/

EXTERN JbCb_t  jbCb;                       /* H.323 global control block */


/*--------------------------------------------------------------------
                 public functions exported by JB module
 ---------------------------------------------------------------------*/
EXTERN INT16_t JbCfgReq ARGS((JbCfg *cfg));

EXTERN INT16_t JbStaReq    ARGS((UINT32_t sessionId, JbSts *sts));

EXTERN INT16_t JbCntrlReq  ARGS((UINT32_t sessionId, UINT8_t mode));

EXTERN INT8_t getJbConMask  ARGS((UINT32_t sessionId));

EXTERN INT16_t JbConReq    ARGS((INT16_t lineNo, UINT32_t sessionId, \
								UINT8_t txCodec, UINT8_t rxCodec, \
    			      			UINT16_t maxAlSduFrames,
    			      			UINT16_t rem_ptime, UINT8_t conn_mask, int owner));

EXTERN INT16_t JbConModReq ARGS((UINT32_t sessionId, UINT8_t txCodec, \
								UINT8_t rxCodec, UINT16_t maxAlSduFrames,
								UINT16_t rem_ptime));

EXTERN INT16_t JbDiscReq   ARGS((UINT32_t sessionId));

EXTERN INT16_t JbDatInd    ARGS((UINT8_t connIndex,\
								rtp_hdr_t *rtpHdr, UINT8_t  *dBuf,\
								UINT32_t dataLen));

EXTERN INT16_t   jbActvInit           ARGS ((void));
EXTERN INT16_t	 JbStartTimer ARGS((JbConnCb *connCb));
EXTERN INT16_t 	 JbStopTimer ARGS((JbConnCb *connCb));


#ifdef JB_BYPASS
EXTERN void JbBypassPutDSP ARGS ((JbConnCb *connCb, UINT32_t dataLen, UINT8_t *dBuf));
#endif




/*--------------------------------------------------------------------
                 system services interface functions
 ---------------------------------------------------------------------*/

EXTERN INT32_t     jbActvTmr            ARGS (( JbLSTCntxt* LSTimer));


//#ifdef TASK_CARRY_PARAM 
//VOIP_OA_TASK_PROC(jbReadSocket, pInTaskParam );
//VOIP_OA_TASK_PROC(jbDspGetManager, pInTaskParam );
//VOIP_OA_TASK_PROC(jbDspPutManager, pInTaskParam );
//#else
void jbReadSocket(void *arg);
void jbDspGetManager(void *arg);
void jbDspPutManager(void *arg);
//#endif

EXTERN INT16_t     JbResetJitterReq     ARGS ((UINT8_t connIndex));

/*--------------------------------------------------------------------
                 public functions used internally by JB module
 ---------------------------------------------------------------------*/


EXTERN INT16_t	 JbUpdateJitter ARGS((JbConnCb* connCb, JbEvent event, rtp_hdr_t* rtpHdr));
EXTERN INT16_t	 JbInsertPacket ARGS((JbConnCb *connCb, UINT8_t *mBuf,
									  UINT32_t dataLen, rtp_hdr_t *rtpHdr));
EXTERN INT16_t	 JbRemovePacket ARGS((JbConnCb *connCb,unsigned char *pBuf,int *pBufSize));

#endif /* _JB_X_ */

