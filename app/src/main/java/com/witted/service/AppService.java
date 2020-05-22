package com.witted.service;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.witted.constant.ErrorCode;
import com.witted.service.remote.OnConnectionStatusChangeListener;
import com.witted.service.remote.GeneralCallback;
import com.witted.utils.LogUtils;
import com.witted.utils.ThreadPoolUtils;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.util.ArrayList;

public class AppService extends Service {
    public static final String TAG = AppService.class.getName();

    private Socket mSocket;

    private Handler mainHandler;

    private ArrayList<OnConnectionStatusChangeListener> mOnConnectionStatusChangeListeners = new ArrayList();
    private OutputStream mOs;

    @Override
    public IBinder onBind(Intent intent) {
        return new LocalBinder();
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mainHandler = new Handler();
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

    public void addConnectionChangeListener(OnConnectionStatusChangeListener listener) {
        if (listener == null) {
            return;
        }
        if (!mOnConnectionStatusChangeListeners.contains(listener)) {
            mOnConnectionStatusChangeListeners.add(listener);
        }
    }

    public void removeConnectionChangeListener(OnConnectionStatusChangeListener listener) {
        if (listener == null) {
            return;
        }
        mOnConnectionStatusChangeListeners.remove(listener);
    }

    public void onConnectionStatusChange(int status) {
        for (OnConnectionStatusChangeListener next : mOnConnectionStatusChangeListeners) {
            next.onConnectionStatusChange(status);
        }
    }


    public class LocalBinder extends Binder {

        public AppService getService() {
            return AppService.this;
        }

    }


    public void connect(final String ip, final int port, final GeneralCallback callback) {

        ThreadPoolUtils.getInstance().execute(() -> {
            try {
                mSocket = new Socket(ip, port);
                ThreadPoolUtils.getInstance().execute(new ReceiveRunnable(mSocket));
                mainHandler.post(() -> callback.onSuccess("success"));
            } catch (final IOException e) {
                LogUtils.i(TAG,"socketcon err"+e.getMessage());
                mainHandler.post(() -> callback.onFail(ErrorCode.SERVICE_DIED));
                e.printStackTrace();
            } finally {

            }
        });
    }

    boolean mIsReceiving = true;

    public boolean sendMsg(String msg) {
        if (mSocket != null) {
            try {
                mOs = mSocket.getOutputStream();
                LogUtils.i(TAG, "sendMsg" + msg);
                mOs.write(msg.getBytes());
                mOs.flush();
                return true;
            } catch (IOException e) {
                LogUtils.i("sendMsg err",e.getMessage());
                e.printStackTrace();
                return false;
            }
        }
        return false;
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
                    while (mIsReceiving&&mSocket!=null){
                        String line = br.readLine();
                        Log.i(TAG, "run:+line "+line);
                    }



            } catch (IOException e) {
                LogUtils.i(TAG,"receive err"+e.getMessage());
                e.printStackTrace();
            }


        }
    }
}
