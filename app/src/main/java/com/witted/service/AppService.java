package com.witted.service;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.witted.service.remote.OnReceiveMessageListener;
import com.witted.service.remote.OnServiceConnectionListener;
import com.witted.service.remote.OnTcpConnectionListener;
import com.witted.utils.LogUtils;
import com.witted.utils.ThreadPoolUtils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;

public class AppService extends Service {
    public static final String TAG = AppService.class.getName();

    private Socket mSocket;


    private OutputStream mOs;
    private OnTcpConnectionListener mTcpConnectListener;
    private OnReceiveMessageListener mOnReceiveMessageListener;

    @Override
    public IBinder onBind(Intent intent) {
        return new LocalBinder();
    }

    @Override
    public void onCreate() {
        super.onCreate();

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mIsReceiving = false;
    }


    /**
     * 服务连接成功,连接失败
     * <p>
     * Tcp连接成功,失败
     */

    public class LocalBinder extends Binder {

        public AppService getService() {
            return AppService.this;
        }

    }

    public void releaseSocket() {
        if (mSocket != null) {
            try {
                if (!mSocket.isClosed()) {
                    mSocket.close();
                }
                mSocket = null;
                mIsReceiving = false;
            } catch (Exception e) {

            }
        }
    }


    public void setTcpConnectListener(OnTcpConnectionListener onTcpConnectionListener) {
        mTcpConnectListener = onTcpConnectionListener;
    }

    public void setReceiveMessgaeListener(OnReceiveMessageListener onReceiveMessageListener) {
        mOnReceiveMessageListener = onReceiveMessageListener;
    }


    public void connect(final String ip, final int port) {

        ThreadPoolUtils.getInstance().execute(() -> {
            try {
                mSocket = new Socket(ip, port);
                mIsReceiving = true;
                ThreadPoolUtils.getInstance().execute(new ReceiveRunnable(mSocket));
                if (mTcpConnectListener != null) {
                    mTcpConnectListener.connectSuccess();
                }
            } catch (final IOException e) {
                LogUtils.i(TAG, "socket  connect err" + e.getMessage());
                if (mTcpConnectListener != null) {
                    mTcpConnectListener.connectFail("" + e.getMessage());
                }
                e.printStackTrace();
            } finally {

            }
        });
    }


    boolean mIsReceiving = true;

    public String sendMsg(String msg) {

        if (mSocket != null) {
            try {
                mOs = mSocket.getOutputStream();
                LogUtils.i(TAG, "socket sendMsg" + msg);
                mOs.write(msg.getBytes());
                mOs.flush();
                return "success";
            } catch (IOException e) {
                String errMessage = e.getMessage();
                LogUtils.i("socket sendMsg err", errMessage);
                if (mTcpConnectListener != null) {
                    mTcpConnectListener.connectFail("" + errMessage);
                }
                releaseSocket();
                e.printStackTrace();
                return ""+ errMessage;
            }
        }
        return "sendMsg err";
    }


    class ReceiveRunnable implements Runnable {

        private Socket socket1;

        ReceiveRunnable(Socket socket) {
            socket1 = socket;
        }

        @Override
        public void run() {
            try {
                InputStream is = socket1.getInputStream();
                BufferedReader br = new BufferedReader(new InputStreamReader(is));
                while (mIsReceiving && mSocket != null) {
                    String line = br.readLine();
                    Log.i(TAG, "run:+line " + line);

                    if (mOnReceiveMessageListener != null) {
                        mOnReceiveMessageListener.onReceiveMessage(line);
                    }
                }

            } catch (IOException e) {
                LogUtils.i(TAG, "socket receive err" + e.getMessage());
                releaseSocket();
                if (mTcpConnectListener != null) {
                    mTcpConnectListener.connectFail("" + e.getMessage());
                }
                mIsReceiving = false;
                e.printStackTrace();
            }


        }
    }
}
