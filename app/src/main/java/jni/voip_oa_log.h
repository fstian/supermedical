#ifndef VOIP_OA_DEF
#define VOIP_OA_DEF

typedef enum 
{
    P_DEBUG,  // 调试信息，在RELEASE不会打印
    P_INFO,   // 比较重要的信息，但不至于出现告警打印
    P_WARN,   // 告警信息
    P_ERROR   // 错误信息
}LOGLEV;

typedef enum
{	
    M_DEBUG,
    M_SIPUA,
    M_SIPLIB,
    M_TRANSP,
    M_VOIPOA,
    M_JITBUF,
    M_SIPMSG,   // 5
    M_STUN,
    M_RTP,
    M_AUDIOSTREAM,
    M_NETMGR,
    M_WFDRV,
    M_PTT,
    M_WPA,
    M_NET,
    M_LOG_MAX
}LOGMOD;

typedef enum {
	VOIP_OA_SUCCESS = 1, //VOIP_OA_SUCCESS
	NO_SYSTEM_RESOURCE ,
	QUEUE_OPERATE_FAIL ,
    MEM_ALLOC_FAIL  ,
    TASK_CREATE_FAIL
} RESULT_t;


#define VOIP_OA_AllocMemory     malloc
#define VOIP_OA_AllocZeroMemory malloc 
#define VOIP_OA_FreeMemory(a)       free(*(a))
#define VOIP_OA_FreeMem      free
#define VOIP_OA_MEMSET             memset
#define VOIP_OA_MEMCPY             memcpy 
#define VOIP_OA_MEMMOV           memmove
#define	VOIP_OA_ReallocMemory	realloc

#define VOIP_OA_INVALID_HANDLE_MUTEX  -1

#define VOIP_OA_INVALID_HANDLE_TASK -1

#define JB_RDSOCKET_TASK_STACK_SIZE     1024
#define JB_PUT_TASK_STACK_SIZE          1024
#define JB_GET_TASK_STACK_SIZE      1024

#define TASK_JBPUT_PRIORITY         10
#define TASK_JBGET_PRIORITY         10
#define TASK_JBREAD_PRIORITY        10

#define SLEEP_JB            10

typedef int  VoipTimer_t;

typedef int VOIP_OA_HANDLE_TASK;

typedef void (*VOIP_OA_TASK_ENTRY)(void *);

typedef int VOIP_OA_HANDLE_MUTEX;

int VOIP_OA_LOG_PRINT (LOGLEV level,LOGMOD module,char *option,...);

int VOIP_OA_MutexLock(VOIP_OA_HANDLE_MUTEX sem);
int VOIP_OA_MutexUnlock(VOIP_OA_HANDLE_MUTEX sem);

int ASSERT(int result);


#endif


