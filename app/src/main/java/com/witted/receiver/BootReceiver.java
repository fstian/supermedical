package com.witted.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.witted.MainActivity;
import com.witted.utils.LogUtils;

public class BootReceiver extends BroadcastReceiver {

    private static final String TAG =BootReceiver.class.getName();

    @Override
    public void onReceive(Context context, Intent intent) {
        LogUtils.i(TAG,intent.getAction());

        Intent intent1 = new Intent(context, MainActivity.class);
        intent1.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent1);
    }
}
