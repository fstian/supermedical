#include "jb.h"
#include "jb_def.h"
#include "rtp_private.h"
#include "rtp_interface.h"
#include "rtp_sess.h"

#include "voip_oa_log.h"

VOIP_OA_HANDLE_MUTEX rtp_data_lock = VOIP_OA_INVALID_HANDLE_MUTEX;
static UINT32_t ssrc_incr; /* increament this variable for each session */

void init_random(void)
{
}


Bool_t get_rtpsess(UINT32_t sessid
		  , rtp_sess_t **rtpsess_p
		  , string_t *rtp_hash_key
		  , char *sessid_str)
{
    if (sessid == RTP_INVALID_SESSID)
    {
        return FALSE;
    }

    *rtpsess_p = (rtp_sess_t *)sessid;
    return TRUE;
}


Bool_t vallid_rtp_session(UINT32_t sessid)
{
	rtp_sess_t *rtpsess_p	= NULL;
	char sessid_str[32];
	string_t rtp_hash_key;

	if (!get_rtpsess(sessid, &rtpsess_p, &rtp_hash_key, sessid_str))
	{
		return(FALSE);
	}

	return(TRUE);
}

Bool_t init_rtp(UINT32_t rtp_portrange_low, UINT32_t rtp_portrange_high)
{
    static UINT32_t inited = 0;
    UINT32_t tbl_size	= 0;

    if (inited > 0)
    {
        return 0;
    }
    
    inited = 1;
    
    init_random();

    ssrc_incr = rand();

    rtp_data_lock = VOIP_OA_INVALID_HANDLE_MUTEX;
    VOIP_OA_MutexCreate(&rtp_data_lock);
    VOIP_OA_ASSERT( VOIP_OA_INVALID_HANDLE_MUTEX != rtp_data_lock);

    return TRUE;
}

RTP_EXPORT UINT8_t get_rfc2833_pltype_value(UINT32_t sessid)
{
	rtp_sess_t *rtpsess_p	= NULL;
	string_t rtp_hash_key;
	char sessid_str[32];
	UINT8_t pload;

	VOIP_OA_MutexLock(rtp_data_lock);
	if (!get_rtpsess(sessid, &rtpsess_p, &rtp_hash_key, sessid_str))
	{
		return (UNDEFINED_PAYLOAD);
	}
	pload = rtpsess_p->rfc2833_pl_value;
	VOIP_OA_MutexUnlock(rtp_data_lock);

	return (pload);
}

char * get_remote_ipaddr(UINT32_t sess_id)
{
    char *ip_addr;
    rtp_sess_t *rtpsess_p	= NULL;
    string_t rtp_hash_key;
    char sessid_str[32];

    if (!get_rtpsess(sess_id, &rtpsess_p, &rtp_hash_key, sessid_str))
    {
        return NULL;
    }
    ip_addr = rtpsess_p->remote_rtp_addr.ipv4;
    return (ip_addr);
}

UINT16_t get_remote_port(UINT32_t sess_id)
{
    UINT16_t remote_port;
    rtp_sess_t *rtpsess_p	= NULL;
    string_t rtp_hash_key;
    char sessid_str[32];

    if (!get_rtpsess(sess_id, &rtpsess_p, &rtp_hash_key, sessid_str))
    {
        return 0;
    }
    remote_port = rtpsess_p->remote_rtp_addr.port;
    return (remote_port);
}

UINT16_t get_local_port(UINT32_t sess_id)
{
    UINT16_t rtp_port;
    rtp_sess_t *rtpsess_p	= NULL;
    string_t rtp_hash_key;
    char sessid_str[32];

    if (!get_rtpsess(sess_id, &rtpsess_p, &rtp_hash_key, sessid_str))
    {
        return 0;
    }
    rtp_port = rtpsess_p->local_rtp_addr.port;
    return (rtp_port);
}

int check_dtmf_sending()
{
    return 0;
}

int getActiveRtpSessionNum(void)
{
    return 1;
}

int send_rtp_pack(UINT32_t sessid
		  , UINT32_t payloadsize
		  , struct sk_buff *skb 
		  , UINT32_t n_frames
		  , UINT8_t payloadtype
		  , UINT32_t timestamp
		  , Bool_t markerbit
		  , UINT32_t csrc)
{
    return 0;
}

void drvInitRtpHandler(void *puser, unsigned int ip, unsigned short port, unsigned short localPort)
{
}

void drvCleanRtpHandler(void)
{
}


