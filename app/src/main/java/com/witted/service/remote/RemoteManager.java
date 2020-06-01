package com.witted.service.remote;

import android.app.Application;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.google.gson.Gson;
import com.witted.service.AppService;
import com.witted.service.model.BaseRequest;
import com.witted.utils.LogUtils;
import com.witted.utils.ThreadPoolUtils;

import java.util.ArrayList;

public class RemoteManager {

    private static final String TAG = RemoteManager.class.getName();

    private static Context sContext;

    public static RemoteManager INST;

    private Handler mainHandler;

    private AppService mClient;


    private ArrayList<OnTcpConnectionListener> mTcpConnectListeners = new ArrayList();
    private ArrayList<OnServiceConnectionListener> mOnServiceConnectionListeners = new ArrayList();


    private static RemoteManager instance() throws Exception {
        if (INST == null) {
            throw new Exception("RemoteManager not init");
        }
        return INST;

    }

    public static void init(Application application) {
        if (INST != null) {
            return;
        }
        sContext = application.getApplicationContext();
        INST = new RemoteManager();
        INST.bindService();

        INST.mainHandler = new Handler();
    }


    /**
     * sContext.bindService返回true 不代表bindService连接成功,连接是异步的
     *
     * @return
     */

    public boolean bindService() {
        if (INST != null) {
            if (mClient != null) {
                return true;
            }
            Intent intent = new Intent(sContext, AppService.class);
            boolean bindService = sContext.bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
            LogUtils.loge(TAG, "RemoteManager" + bindService);
        } else {
            LogUtils.loge(TAG, "RemoteManager not initialized");
        }
        return false;
    }


    public void shutdown() {
        if (mClient != null) {
            sContext.unbindService(mServiceConnection);
        }
    }


    /**
     * 在serviceconnectsuccess后调用
     *
     * @param ip
     * @param port
     */
    public void connectTcp(String ip, int port) {

        if (mClient == null) {
            throw new RuntimeException("mClient == null");
        }
        mClient.connect(ip, port);
    }


    public void sendMsg(BaseRequest baseRequest,GeneralCallback callback) {

        ThreadPoolUtils.getInstance().execute(new Runnable() {
            @Override
            public void run() {
                if (mClient != null) {
                    Gson gson = new Gson();
                    String msg = gson.toJson(baseRequest) + "\n";
                    String sendMsg = mClient.sendMsg(msg);

                    if("success".equals(sendMsg)){
                        mainHandler.post(callback::onSuccess);
                    }else {
                        mainHandler.post(()->callback.onFail(sendMsg));
                    }
                    return;
                }
                mainHandler.post(()->callback.onFail("mClient = null"));

            }
        });


    }

    public boolean isServiceConnectSuccess() {
        if (mClient != null) {
            return true;
        }
        return false;
    }


 public    ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            AppService.LocalBinder binder = (AppService.LocalBinder) service;
            mClient = binder.getService();
            Log.i(TAG, "onCreateremote: " + System.currentTimeMillis());

            LogUtils.i(TAG, "onServiceConnected");

            mClient.setTcpConnectListener(mTcpConnectListener);

            mClient.setReceiveMessgaeListener(mOnReceiveMessageListener);

            for (OnServiceConnectionListener onServiceConnectionListener : mOnServiceConnectionListeners) {
                onServiceConnectionListener.onServiceConnected();

            }

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            LogUtils.i(TAG, "onServiceDisconnected");
            for (OnServiceConnectionListener onServiceConnectionListener : mOnServiceConnectionListeners) {
                onServiceConnectionListener.onServiceDisconnected();
            }

        }
    };


    private OnReceiveMessageListener mOnReceiveMessageListener = new OnReceiveMessageListener() {
        @Override
        public void onReceiveMessage(String msg) {

            LogUtils.i(TAG,"receiveMsg"+msg);

        }
    };


    private OnTcpConnectionListener mTcpConnectListener = new OnTcpConnectionListener() {

        @Override
        public void connectSuccess() {
            mainHandler.post(() -> {
                for (OnTcpConnectionListener tcpConnectListener : mTcpConnectListeners) {
                    tcpConnectListener.connectSuccess();
                }
            });


        }

        @Override
        public void connectFail(String err) {

            mainHandler.post(() -> {
                for (OnTcpConnectionListener tcpConnectListener : mTcpConnectListeners) {
                    tcpConnectListener.connectFail(err);
                }
            });


        }
    };

    public void addOnTcpConnectionListener(OnTcpConnectionListener listener) {
        if (listener == null) {
            return;
        }
        mainHandler.post(() -> {
            if (!mTcpConnectListeners.contains(listener)) {
                mTcpConnectListeners.add(listener);
            }
        });

    }

    public void removeOnTcpConnectionListener(OnTcpConnectionListener listener) {
        if (listener == null) {
            return;
        }
        mainHandler.post(() -> {

            mTcpConnectListeners.remove(listener);
        });

    }


    public void addOnServiceConnectionListener(OnServiceConnectionListener listener) {
        if (listener == null) {
            return;
        }
        mainHandler.post(() -> {

            if (!mOnServiceConnectionListeners.contains(listener)) {
                mOnServiceConnectionListeners.add(listener);
            }
        });


    }

    public void removeOnServiceConnectionListener(OnServiceConnectionListener listener) {
        if (listener == null) {
            return;
        }
        mainHandler.post(() -> {

            mOnServiceConnectionListeners.remove(listener);
        });

    }


}
