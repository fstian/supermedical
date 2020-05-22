
/********************************************************************20**

     Name:    Jitter Buffer

     Type:    C include file

     Desc:    Defines required by Jitter Buffer

     File:    jb.h

     Sid:     jb.h 1.0  -  01/21/2000

     Prg:     sdg

     Revision History
     09/27/2000         RFG         Changed JB_G729_FRAME_SIZE to 30
                                    Changed JB_G729_CODEC_INTERVAL to 3

************************************************************************/


#ifndef __JBH__
#define __JBH__

#include "voip_oa_log.h"

#define LONG_SILENCE_TIMERQ  "JbSTimerQ"
#ifdef JB_TRACE
#define TRC3(a)         DBG_PRINT(#a "\n");
#else
#define TRC3(a)
#endif

#define __FUNCTION__        "unknow"


typedef  char   INT8_t ;
typedef  unsigned char UINT8_t ;
typedef  short  int INT16_t ;
typedef  unsigned short int UINT16_t ;
typedef  int    INT32_t ;
typedef  unsigned int UINT32_t ;
typedef  void VOID_t;
typedef  char CHAR_t;

typedef int INT64_t;
typedef unsigned long UINT64_t;

typedef enum {
  BOOL_FALSE = 0,
  BOOL_TRUE = 1
} Bool_t;

typedef int osapi_sock_t;


typedef void* HTIMER;
#define VOIP_TIMER_NULL (HTIMER)NULL
typedef void (*TimerCallbackFunc_t)(HTIMER hTimer, UINT32_t type, void *arg);


#define  JB_DFLT_JBMaxConn                  2
#define  JB_DFLT_JBBufSize                  64       
#define  JB_DFLT_JBMaxLatency               18  //MAX=JB_DFLT_JBBufSize-2
#define JB_DFLT_PTT_MAX_LATENCY         24
#define  JB_DFLT_JBMaxHoldThreshold         125
#define  JB_DFLT_JBMinBufSize               4    
#define JB_DFLT_PTT_MinBufSize              6
#define  JB_DFLT_JBMinStableEvent         50
#define  JB_DFLT_JBGrowRate                 100      
#define  JB_DFLT_JBShrinkRate               80
#define  JB_DFLT_JBSilencTime               1000

#define JB_DFLT_JBHold_MaxSize 6

#define JB_DFLT_PTIME (20)  //MS
#define JB_10MS_PTIME (10)

//#define JB_PACKETS_DEBUG

/***********************************************************************
               defines related to codecs
 ***********************************************************************/

typedef enum coderInfo
{
    JB_G711_ULAW = 0,
    JB_G723 = 4,
    JB_G711_ALAW = 8,
    JB_G722 = 9,
    JB_G729 = 18,
    JB_G726_32 = 96,
    JB_NUM_CODERS
} CoderInfo;

#define JB_10MS_TIMESTAMP           (80)  /* RTP tstamp for a 10ms interval */

/* change RAW FRAME definition for G711_ULAW/G722_ALAW/G722/G726 to be for 1ms (not 10ms)
 * this is required to interop with terminals that encode in non-multiples
 * of 10ms
 */

#define JB_G729_SIZE_PER_1MS (1)
#define JB_G711_SIZE_PER_1MS (8)

#define JB_G711_ULAW_RAW_FRAME_SIZE     (8) /* size of 1ms g711 Ulaw samples */
#define JB_G711_ALAW_RAW_FRAME_SIZE     (8) /* size of 1ms g711 Alaw samples */
#define JB_G722_RAW_FRAME_SIZE      (8) /* size of 1ms g722 samples */
#define JB_G723_6_RAW_FRAME_SIZE    (24) /* size of a 30ms g723 6.3kbps frame */
#define JB_G723_5_RAW_FRAME_SIZE    (20) /* size of a 30ms g723 5.3kbps frame */
#define JB_G723_SID_RAW_FRAME_SIZE  (4) /* size of a g723 Silence frame */

#define JB_G726_32_RAW_FRAME_SIZE   (4) /* size of 1ms g726 32kbps samples */
#define JB_G729_RAW_FRAME_SIZE      (10) /* size of a 10ms g729 frame */

#define JB_G723_MS_PER_FRAME        (30)
#define JB_G729_MS_PER_FRAME        (10)

#define JB_G711_ULAW_MS_PER_FRAME        (10)
#define JB_G711_ALAW_MS_PER_FRAME        (10)
#define JB_G722_MS_PER_FRAME        (10)
#define JB_G726_32_MS_PER_FRAME     (10)

#define JB_MAX_MS                   (40)  /*in R600 platform, only supoprt 40ms packets*/
/* Venkat - Since raw frame size for all audio codecs
                            is 8, we may take ULaW raw frame size for
                            caluculations here. Other wise take care for codec
                            types other than Audio.
                        */
#define JB_MAX_BUF_SIZE             (JB_G711_ULAW_RAW_FRAME_SIZE * JB_MAX_MS)

 #define JB_MAX_SKB_RESERVE_SIZE (400)
/* Venkat - 07/01/2004 - Adding the support for providing the generalized buffer size
 * for jitter, so that the modification to codec is posible in mid call */
#define G711_MAX_PPERIOD            (JB_MAX_MS) /* maximum packetization period for G711(a or u law)
                                                  * is 30 msecs - source RFC3551  - Venkat - 7/1/04 */
#define JB_GEN_BUF_SIZE             (JB_G711_ULAW_RAW_FRAME_SIZE * G711_MAX_PPERIOD)

/* end - generalized buffer support */

/*
#define JB_G723_MAX_FRAMES          (JB_MAX_MS / JB_G723_MS_PER_FRAME)
#define JB_G729_MAX_FRAMES          (JB_MAX_MS / JB_G729_MS_PER_FRAME)

#define JB_G711_ULAW_MAX_MS              (JB_MAX_MS)
#define JB_G711_ALAW_MAX_MS              (JB_MAX_MS)
#define JB_G722_MAX_MS              (JB_MAX_MS)
#define JB_G726_32_MAX_MS           (JB_MAX_MS)
*/
/***********************************************************************
               macros for jb_utl.c
 ***********************************************************************/
#define MULTPERCENT(val, percent) \
    (UINT16_t)(((UINT32_t)(val)*(UINT32_t)percent*(UINT32_t)656)>>16)

#define GETRAWFRAMESIZE(Codec, frameSize) \
    switch (Codec) \
    { \
        case JB_G711_ULAW: \
            frameSize = JB_G711_ULAW_RAW_FRAME_SIZE; \
            break; \
        case JB_G711_ALAW: \
            frameSize = JB_G711_ALAW_RAW_FRAME_SIZE; \
            break; \
        case JB_G722: \
            frameSize = JB_G722_RAW_FRAME_SIZE; \
            break; \
        case JB_G723: \
            frameSize = JB_G723_6_RAW_FRAME_SIZE; \
            break; \
        case JB_G726_32: \
            frameSize = JB_G726_32_RAW_FRAME_SIZE; \
            break; \
        case JB_G729: \
            frameSize = JB_G729_RAW_FRAME_SIZE; \
            break; \
        default:\
          VOIP_OA_LOG_PRINT( P_ERROR,M_JITBUF, "FATAL ERROR: unknow codec type = %d\n", Codec);\
    }

/***********************************************************************
                      general defines
 ***********************************************************************/
typedef enum configType
{
    GEN = 0,
    JIT
} ConfigType;

typedef enum statsType
{
    CB = 0,
    CONN
} StatsType;

typedef enum jbState
{
    JB_UNDERFLOW_LONG_SILENCE = 0,
    JB_ACTIVE_HOLD,
    JB_ACTIVE_PLAY,
    JB_UNDERFLOW
} JbState;

typedef enum jbEvent
{
    JB_RECEIVE_PACKET = 0,
    JB_PLAY_PACKET,
    JB_LONG_SILENCE
} JbEvent;

#define NMB_JB_EVNT                     (3)
#define NMB_JB_ST                       (4)
#define JB_TQSIZE                       (128)

#ifndef SS_VW
#define JB_DEFAULT_TIMER_RES            (10)  /* 1s */
#else
#define JB_DEFAULT_TIMER_RES            (1) /* 100ms */
#endif

struct sk_buff {
    unsigned int 	len;			/* Length of actual data			*/
    unsigned char	*head;			/* Head of buffer 				*/
    unsigned char	*data;			/* Data head pointer				*/
    unsigned char	*tail;			/* Tail pointer					*/
    unsigned char 	*end;			/* End pointer					*/
};

typedef enum {
    
    dtmf_0,
    dtmf_1,
    dtmf_2,
    dtmf_3,
    dtmf_4,
    dtmf_5,
    dtmf_6,
    dtmf_7,
    dtmf_8,
    dtmf_9,
    dtmf_star,  /*  "*"  */
    dtmf_hash,  /*  "#"  */
    dtmf_a,
    dtmf_b,
    dtmf_c,
    dtmf_d,
    null_dtmf_tone
} dtmf_tone_type;


#endif /* __JBH__ */
/********************************************************************30**

         End of file: jb.h 1.0  -  01/21/2000

*********************************************************************31*/

/********************************************************************40**

        Notes:

*********************************************************************41*/

/********************************************************************50**

*********************************************************************51*/

/********************************************************************60**

        Revision history:

*********************************************************************61*/

/********************************************************************90**


*********************************************************************91*/





















