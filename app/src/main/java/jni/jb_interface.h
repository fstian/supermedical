#ifndef JB_INTERFACE_DEF
#define JB_INTERFACE_DEF

#ifdef __cplusplus
extern "C" {
#endif

void voip_init();
unsigned short JbConReq(unsigned short lineNo,unsigned long sessionId,unsigned char txCodec,unsigned char rxCodec,unsigned short maxAlSduFrames,unsigned short rem_ptime,unsigned char conn_mask,int owner);
int  drv_proc_rcvd_rtp(int jbline, void *rtp, unsigned int length);
int JbGetStatus(int jbline,char *pDst, int size);
unsigned short jbProcessRxCodec(unsigned char connIndex,unsigned char *pDst);


#ifdef __cplusplus
}
#endif

#endif

