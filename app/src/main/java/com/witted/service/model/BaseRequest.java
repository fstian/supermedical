package com.witted.service.model;

public class BaseRequest<T> {
    public int msgtype;

    public int msgid;

    public String devid;

    public T content;

    public BaseRequest(int msgtype, int msgid, String devid, T content) {
        this.msgtype = msgtype;
        this.msgid = msgid;
        this.devid = devid;
        this.content = content;
    }
}
