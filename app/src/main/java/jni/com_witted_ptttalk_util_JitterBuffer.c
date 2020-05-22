#include <jni.h>

#define JB_VERSION			999


JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_initJb( JNIEnv* env,jclass thiz)
{
    voip_init();
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_openJb( JNIEnv* env,jclass thiz,jint codec,jint ptime)
{
    int ret;
    ret = JbConReq(0,0,codec,codec,ptime,ptime,1,1);
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_closeJb( JNIEnv* env,jclass thiz,jint jbline)
{
    JbDiscReq(jbline);
    return 0;
}


JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_addPackage( JNIEnv* env,jclass thiz,jint jbline,jbyteArray src, jint size)
{
    int srclen;
    jbyte *srcjbarray;
    unsigned char *psrc;

    srclen = (*env)->GetArrayLength(env,src);
    srclen = srclen<size?srclen:size;

    srcjbarray = (*env)->GetByteArrayElements(env,src,(void *)0);
    psrc = (unsigned char *)srcjbarray;

    drv_proc_rcvd_rtp(jbline,psrc,srclen);
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_getPackage( JNIEnv* env,jclass thiz,jint jbline,jbyteArray dst, jint size)
{
    unsigned char arRecvData[200];
    int recvlen;

    recvlen = jbProcessRxCodec(jbline,arRecvData);

    if(recvlen>0){
        recvlen = recvlen<size?recvlen:size;
        (*env)->SetByteArrayRegion(env,dst,0,recvlen,arRecvData);
    }

    return recvlen;
}

JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_getStatus( JNIEnv* env,jclass thiz,jint jbline,jbyteArray dst, jint size)
{
    unsigned char arRecvData[2000];
    int recvlen;

    recvlen = JbGetStatus(jbline,arRecvData,size);

    if(recvlen>0){
        recvlen = recvlen<size?recvlen:size;
        (*env)->SetByteArrayRegion(env,dst,0,recvlen,arRecvData);
    }

    return recvlen;
}

JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_getJbVer( JNIEnv* env,jclass thiz)
{
    return JB_VERSION;
}

JNIEXPORT jint JNICALL
Java_com_witted_ptttalk_util_JitterBuffer_deInitJb( JNIEnv* env,jclass thiz)
{
    jbuninit();
    return 0;
}

