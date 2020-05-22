package com.witted.ptttalk.util;

/**
 * Created by YT on 2018.7.12.
 */

public class JitterBuffer {

    public static native int initJb();

    public static native int openJb(int codec,int ptime);
    public static native int closeJb(int jbline);

    public static native int addPackage(int jbline,byte[] src,int size);

    public static native int getPackage(int jbline,byte[] dst,int size);
    public static native int getStatus(int jbline,byte[] dst,int size);
    public static native int getJbVer();
    public static native int deInitJb();
}
