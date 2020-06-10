package com.witted.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class DemoReceiver extends BroadcastReceiver {

    //sip功能能不能去掉  有没有原生的系统

    //


    @Override
    public void onReceive(Context context, Intent intent) {

        String action = intent.getAction();

        Log.i("DemoReceiver", "onReceive: "+action);

    }
}
