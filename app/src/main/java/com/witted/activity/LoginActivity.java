package com.witted.activity;

import android.os.Handler;

import com.witted.R;
import com.witted.service.model.BaseRequest;
import com.witted.service.model.Register;
import com.witted.service.remote.GeneralCallback;
import com.witted.service.remote.RemoteManager;
import com.witted.utils.LogUtils;
import com.witted.utils.ThreadPoolUtils;

public class LoginActivity extends BaseActivity {
    @Override
    protected int contentLayout() {
        return R.layout.activity_login;
    }

    @Override
    protected void afterViews() {
        super.afterViews();

        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                RemoteManager.INST.connectTcp("172.16.2.209", 9999, new GeneralCallback() {
                    @Override
                    public void onSuccess(String success) {
                        LogUtils.i(TAG, "success");
                        BaseRequest<Register> request = new BaseRequest<>(0, 101, "201141FF", new Register(0, "201141"));
                        ThreadPoolUtils.getInstance().execute(new Runnable() {
                            @Override
                            public void run() {
                                while (true) {
                                    BaseRequest<Register> request = new BaseRequest<>(1, 101, "201141FF", new Register(0, "201141"));
                                    try {
                                        boolean sendMsg = RemoteManager.INST.sendMsg(request);
                                        LogUtils.i(TAG, "sendMsg" + sendMsg);
                                        Thread.sleep(10 * 1000);
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }

                                }

//
                            }


                        });


                    }

                    @Override
                    public void onFail(int errCode) {
                        LogUtils.i(TAG, "fail");


                    }
                });
            }
        }, 2000);

    }

    //9999  172.16.2.209
}
