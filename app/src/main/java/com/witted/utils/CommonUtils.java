package com.witted.utils;

import android.app.ActivityManager;
import android.app.Service;
import android.content.Context;
import android.media.MediaPlayer;
import android.os.Vibrator;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;

/**
 * Created by YT on 2017.11.20.
 */

public class CommonUtils {


    public static int getScreenWidthsPx(Context context){
        return context.getResources().getDisplayMetrics().widthPixels;

    }

    public static int getScreenHeightsPx(Context context){
        return context.getResources().getDisplayMetrics().heightPixels;

    }

    public static int getScreenWidthsDp(Context context){
        return (int) (context.getResources().getDisplayMetrics().widthPixels/getScreenDensity(context));

    }

    public static int getScreenHeightsDp(Context context){
        return (int) (context.getResources().getDisplayMetrics().heightPixels/getScreenDensity(context));

    }

    public static float getScreenDensity(Context context){
        return context.getResources().getDisplayMetrics().density;

    }

    public static void setVibrator(Context context){
        Vibrator vibrator = (Vibrator) context.getSystemService(Service.VIBRATOR_SERVICE);
        vibrator.vibrate(1000);
    }

    protected void showSound(Context context, int raw) {
        MediaPlayer mediaPlayer = null;
        mediaPlayer = MediaPlayer.create(context, raw);
        mediaPlayer.setVolume(0.05f, 0.05f);
        mediaPlayer.start();
    }



    /*
* 判断服务是否启动,context上下文对象 ，className服务的name
*/
    public static boolean isServiceRunning(Context mContext, String className) {

        boolean isRunning = false;
        ActivityManager activityManager = (ActivityManager) mContext
                .getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> serviceList = activityManager
                .getRunningServices(200);

        if (!(serviceList.size() > 0)) {
            return false;
        }

        for (int i = 0; i < serviceList.size(); i++) {
            if (serviceList.get(i).service.getClassName().equals(className) == true) {
                isRunning = true;
                break;
            }
        }
        return isRunning;
    }

    public static String ms2Date(long _ms) {
        Date date = new Date(_ms);
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault());
        return format.format(date);
    }


    public static long getLongDate(String time){
        SimpleDateFormat sdf =new SimpleDateFormat("yyyy-mm-dd HH:mm:ss");
        try {
            Date date = sdf.parse(time);
            return date.getTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return 0;
    }

}
