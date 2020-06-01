package com.witted.service.remote;

public interface OnTcpConnectionListener {

    void connectSuccess();

    void connectFail(String err);

}
