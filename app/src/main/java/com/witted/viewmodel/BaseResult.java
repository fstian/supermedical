package com.witted.viewmodel;

public class BaseResult<T> {

    public int code;
    public String msg;

    public T t;

    @Override
    public String toString() {
        return "BaseResult{" +
                "code=" + code +
                ", msg='" + msg + '\'' +
                ", t=" + t +
                '}';
    }
}
