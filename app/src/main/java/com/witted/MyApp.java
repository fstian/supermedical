package com.witted;

import android.app.Application;
import android.util.Log;

import com.witted.service.remote.RemoteManager;

import java.net.DatagramSocket;
import java.net.SocketException;

public class MyApp extends Application {


    @Override
    public void onCreate() {
        super.onCreate();
        Log.i("", "onCreatereapp: "+System.currentTimeMillis());

//        RemoteManager.init(this);|



    }
}
