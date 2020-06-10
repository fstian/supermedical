package com.witted;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.witted.ptttalk.util.JitterBuffer;
import com.witted.service.AppService;
import com.witted.service.NettyService;
import com.witted.service.remote.GeneralCallback;
import com.witted.service.remote.RemoteManager;
import com.witted.utils.LogUtils;
import com.witted.viewmodel.BaseResult;
import com.witted.viewmodel.TestViewModel;
import com.witted.viewmodel.User;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = MainActivity.class.getName();
    private Intent mIntent;
    private AppService mAppService;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

//        for (int i = 0; i < 1; i++) {
//            test();
//
//        }




        startService(new Intent(this, NettyService.class));

    }

    private void test(){

        new Thread(new Runnable() {
            @Override
            public void run() {

                try {
                    Socket socket = new Socket("172.16.1.172", 11676);

                    new Thread(new Runnable() {
                        @Override
                        public void run() {

                            try {

                                OutputStream os = socket.getOutputStream();

                                new Thread(new Runnable() {
                                    @Override
                                    public void run() {
                                        InputStream is = null;
                                        try {
                                            is = socket.getInputStream();
                                            BufferedReader br = new BufferedReader(new InputStreamReader(is));

                                            while (true){
                                                String line = br.readLine();
                                                Log.i(TAG, "run:_receive "+ line);
                                            }

                                        } catch (IOException e) {
                                            e.printStackTrace();
                                        }


                                    }
                                }).start();

                                while (true){
//                                    String line = br.readLine();
//                                    Log.i(TAG, "message "+line );
                                    os.write("heart".getBytes());
                                    os.flush();
                                    Log.i(TAG, "run: "+"sendmsg");
                                    Thread.sleep(5*1000);

                                }
                            } catch (IOException e) {
                                Log.e(TAG, "run1: err "+e.getMessage() );
                                e.printStackTrace();
                            } catch (InterruptedException e) {
                                Log.e(TAG, "run: err "+e.getMessage() );

                                e.printStackTrace();
                            }

                        }
                    }).start();

                } catch (IOException e) {

                    Log.e(TAG, "run:+ERR "+e.getMessage() );
                    e.printStackTrace();
                }


            }
        }).start();

    }

//
//    @Override
//    public void onWindowFocusChanged(boolean hasFocus) {
//        super.onWindowFocusChanged(hasFocus);
//        if (hasFocus) {
//            getWindow().getDecorView().setSystemUiVisibility(
//                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
//                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
//                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
//                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
//                            | View.SYSTEM_UI_FLAG_FULLSCREEN
//                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
//        }
//    }


}
