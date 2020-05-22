package com.witted.utils;

import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by YT on 2017.4.5.
 */

public class LogUtils {

    public static LogUtils sLogUtils;
    private final File mFile;
    private static FileOutputStream mFs;

    public static LogUtils getInstance(){
        if(sLogUtils==null){
            sLogUtils=new LogUtils();
        }
        return sLogUtils;
    }

    public LogUtils() {
        mFile = new File(Environment.getExternalStorageDirectory(), "log.txt");
        try {
            mFs = new FileOutputStream(mFile, true);
        } catch (FileNotFoundException e) {
            Log.i("LogUtils", "LogUtils: "+e.getMessage());
            e.printStackTrace();
        }
    }


    private static boolean on=true;
    public static void i(String tag, String content){
        if(on){
            Log.i(tag,content);
        }
    }
    public static void logv(String tag, String content){
        if(on){
            try {
                String date = CommonUtils.ms2Date(System.currentTimeMillis());
                mFs.write((date+"   "+tag+"   "+content).getBytes());
                String newLine = System.getProperty("line.separator");
                mFs.write(newLine.getBytes());
                mFs.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
            Log.w(tag,content);
        }
    }


    public static void logt(String tag, String conent){
        if(on){
            try {
//                String date = CommonUtils.ms2Date(System.currentTimeMillis());
                mFs.write((conent).getBytes());
                String newLine = System.getProperty("line.separator");
                mFs.write(newLine.getBytes());
                mFs.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
            Log.w(tag,conent);
        }
    }


    public static void loge(String tag, String conent){
        if(on){
            Log.e(tag,conent);
        }
    }
    public static void logw(String tag, String conent){
        if(on){
            Log.w(tag,conent);
        }
    }



}
