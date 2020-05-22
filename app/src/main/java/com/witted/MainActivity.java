package com.witted;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.witted.ptttalk.util.JitterBuffer;
import com.witted.service.AppService;
import com.witted.service.remote.GeneralCallback;
import com.witted.service.remote.RemoteManager;
import com.witted.utils.LogUtils;
import com.witted.viewmodel.BaseResult;
import com.witted.viewmodel.TestViewModel;
import com.witted.viewmodel.User;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;

public class MainActivity extends AppCompatActivity {

    public static final String TAG=MainActivity.class.getName();
    private Intent mIntent;
    private AppService mAppService;
//
//    static {
//        System.loadLibrary("jitterbuffer");
//    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.i(TAG, "onCreateMain: "+System.currentTimeMillis());


        TestViewModel testViewModel = ViewModelProvider.AndroidViewModelFactory.getInstance(getApplication()).create(TestViewModel.class);

        testViewModel.getLoginResult(new User("yt1","123 ")).observe(this, new Observer<BaseResult<String>>() {
            @Override
            public void onChanged(BaseResult<String> stringBaseResult) {
                Log.i(TAG, "onChanged: "+stringBaseResult);
            }
        });


//        int i = JitterBuffer.initJb();

//        Log.i(TAG, "onCreateMain: "+" jni   "+i);
//
//        new Handler().postDelayed(new Runnable() {
//            @Override
//            public void run() {
//                RemoteManager.INST.connectTcp("172.16.2.89", 8878, new GeneralCallback() {
//
//                    @Override
//                    public void onSuccess(String success) {
//                        LogUtils.logi(TAG,success);
//                    }
//
//                    @Override
//                    public void onFail(int errCode) {
//                        LogUtils.logi(TAG,""+errCode);
//                    }
//
//                });
//            }
//        },1000);






    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindService(mServiceConnection);
    }

    ServiceConnection mServiceConnection= new ServiceConnection(){
       @Override
       public void onServiceConnected(ComponentName name, IBinder service) {
           LogUtils.i(TAG,"onServiceConnected");
           AppService.LocalBinder binder= (AppService.LocalBinder) service;
           mAppService = binder.getService();

       }

       @Override
       public void onServiceDisconnected(ComponentName name) {

           LogUtils.i(TAG,"onServiceDisconnected");

       }
   };

}
