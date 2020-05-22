#include "jb.h"
#include "jb_def.h"
#include "voip_oa_log.h"


char* pSlientData=NULL;
char SlientDatalen=0;
unsigned short noframe = 0;


int VOIP_OA_LOG_PRINT (LOGLEV level,LOGMOD module,char *option,...)
{
    return 0;    
}

int VOIP_OA_MutexCreate(VOIP_OA_HANDLE_MUTEX *pSem)
{
    *pSem = 0;
    return 0;
}

int VOIP_OA_MutexLock(VOIP_OA_HANDLE_MUTEX sem)
{
    return 0;
}

int VOIP_OA_MutexUnlock(VOIP_OA_HANDLE_MUTEX sem)
{
    return 0;
}

int VOIP_OA_MutexLock_Ex(VOIP_OA_HANDLE_MUTEX sem)
{
    return 0;
}

int VOIP_OA_MutexUnlock_Ex(VOIP_OA_HANDLE_MUTEX sem)
{
    return 0;
}

int VOIP_OA_MutexDelete(VOIP_OA_HANDLE_MUTEX *pSem)
{
    return 0;
}

int ASSERT(int result)
{
    return 0;
}

int VOIP_OA_ASSERT(int result)
{
    return 0;
}

void 	DlDSPGenerateSilence( UINT16_t lineno, UINT8_t nframes )
{
}

void 	DlDSPPacketLossConcealment( UINT16_t Handle, UINT8_t nframes)
{
}

int DlDSPPutBuffer(unsigned char * pbuf, unsigned int buflen, unsigned short lineno)
{
    return 0;
}

int UlDSPGetBuffer(unsigned char * pbuf, unsigned short* nframe, unsigned short* buflen, unsigned short lineno)
{
    return 0;
}

void Exit_Sleep(int flag)
{
}

void Enter_Sleep(int flag)
{
}

void VOIP_OA_TaskDelayTick(int tick)
{
}

RESULT_t		VOIP_OA_TaskCreate( VOIP_OA_HANDLE_TASK* pHTask, 
     VOIP_OA_TASK_ENTRY taskEntry,
     char*	lpName,
     void* pTSBase, 
     unsigned int tsSize, 
     unsigned int nPriority,
     void* lpTaskParameter)
{
    return VOIP_OA_SUCCESS; 
	
}

int VOIP_OA_TaskStart(VOIP_OA_HANDLE_TASK task)
{
    return 0;
}


int VOIP_OA_TaskSuspend(VOIP_OA_HANDLE_TASK task)
{
    return 0;
}

int VOIP_OA_TaskResume(VOIP_OA_HANDLE_TASK task)
{
    return 0;
}

int VOIP_OA_TaskThisHandle()
{
    return 0;
}

int taskDelay(int time)
{
    return 0;
}

HTIMER voip_timer_start(UINT32_t type,
                                            UINT32_t period, 
                                            TimerCallbackFunc_t callback, 
                                            void * arg,
                                            UINT32_t loop)
{

    JbLSTCntxt *pls = (JbLSTCntxt *)arg;

    pls->WaitTick = period/20;
    pls->TimeTick = 0;
    pls->cb = callback;
    return 1;
}

void voip_timer_stop(HTIMER hTimer)
{
}


void skb_reserve(struct sk_buff *skb, unsigned int len)
{
}

unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
    unsigned char *tmp=skb->tail;
    return tmp;
}

void dev_kfree_skb(struct sk_buff *skb)
{
    free(skb);
}

struct sk_buff *dev_alloc_skb(int size)
{
    return (struct sk_buff *)malloc(size);
}

unsigned long VOIP_OA_htonl(unsigned long value)
{
    unsigned long ulRtn;

    ulRtn = (value&0xff)<<24;
    ulRtn += (value&0xff00)<<8;
    ulRtn += (value&0xff0000)>>8;
    ulRtn += (value&0xff000000)>>24;
    return ulRtn;    
}

unsigned short VOIP_OA_htons(unsigned short value)
{
    unsigned short usRtn;

    usRtn = (value&0xff)<<8;
    usRtn += (value&0xff00)>>8;

    return usRtn;
}


unsigned short  VOIP_OA_ntohs(unsigned short value)
{
    return VOIP_OA_htons(value);
}

unsigned long VOIP_OA_ntohl(unsigned long value)
{
    return VOIP_OA_htonl(value);
}

unsigned long sg_local_rand = 100;

unsigned long rand()
{
    return sg_local_rand++;
}

