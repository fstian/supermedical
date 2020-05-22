package com.witted.service.remote;

import android.app.Application;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

import com.google.gson.Gson;
import com.witted.service.AppService;
import com.witted.service.model.BaseRequest;
import com.witted.utils.LogUtils;

public class RemoteManager {

    private static Context sContext;

    public static RemoteManager INST;
    private static final String TAG = RemoteManager.class.getName();
    private AppService mClient;

    private static RemoteManager instance() throws Exception {
        if (INST == null) {
            throw new Exception("RemoteManager not init");
        }
        return INST;

    }

    public static void init(Application application) {
        //inited
        if (INST != null) {
            return;
        }
        sContext = application.getApplicationContext();
        INST = new RemoteManager();
        INST.bindService();
    }


    /**
     * sContext.bindService返回true 不一定意味着mClient不为空,   异步
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


    public void connectTcp(String ip, int port, GeneralCallback callback) {
        if (!bindService()) {
            callback.onFail(111);
            return;
        }
        mClient.connect(ip, port, callback);
    }


    public boolean sendMsg(BaseRequest baseRequest) {
        if (mClient != null) {
            Gson gson = new Gson();
            String msg = gson.toJson(baseRequest) + "\n";
            return mClient.sendMsg(msg);
        }
        return false;
    }

    public boolean serviceConnectSuccess() {
        if (mClient != null) {
            return true;
        }
        return false;
    }


    ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            AppService.LocalBinder binder = (AppService.LocalBinder) service;
            mClient = binder.getService();
            Log.i(TAG, "onCreateremote: " + System.currentTimeMillis());

            LogUtils.i(TAG, "onServiceConnected");

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            LogUtils.i(TAG, "onServiceDisconnected");

        }
    };


}
