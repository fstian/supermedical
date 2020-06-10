package com.witted.activity;

import com.witted.R;
import com.witted.activity.viewmodel.ServiceConnectedViewModel;
import com.witted.activity.viewmodel.TcpConnectedViewModel;
import com.witted.service.model.BaseRequest;
import com.witted.service.model.Register;
import com.witted.service.remote.GeneralCallback;
import com.witted.service.remote.RemoteManager;
import com.witted.utils.LogUtils;

import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;

public class LoginActivity extends BaseActivity {


    public final String TAG = LoginActivity.class.getName();

    @Override
    protected int contentLayout() {
        return R.layout.activity_login;
    }

    @Override
    protected void afterViews() {

        ViewModelProvider viewModelProvider = new ViewModelProvider(this, new ViewModelProvider.NewInstanceFactory());

        ServiceConnectedViewModel serviceConnectedViewModel = viewModelProvider.get(ServiceConnectedViewModel.class);

        TcpConnectedViewModel tcpConnectedViewModel = viewModelProvider.get(TcpConnectedViewModel.class);

        serviceConnectedViewModel.getServiceConnectLivedata().observe(this, new Observer<Boolean>() {
            @Override
            public void onChanged(Boolean serviceConnected) {
                LogUtils.i(TAG, "ServiceConnect   " + serviceConnected);
                if (serviceConnected) {
                    RemoteManager.INST.connectTcp("172.16.2.209", 9999);
                }
            }
        });

        tcpConnectedViewModel.getTcpConnectedViewModel().observe(this, new Observer<String>() {
            @Override
            public void onChanged(String s) {
                LogUtils.i(TAG, "TcpConnected   " + s);

                BaseRequest<Register> request = new BaseRequest<>(1, 101, "201141FF", new Register(0, "201141"));
                RemoteManager.INST.sendMsg(request, new GeneralCallback() {
                    @Override
                    public void onSuccess() {
                        LogUtils.i(TAG,"sendMsg"+"   success");
                    }

                    @Override
                    public void onFail(String err) {
                        LogUtils.i(TAG,"sendMsg"+"   =rr");
                    }
                });

            }
        });


    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        RemoteManager.INST.shutdown();

        System.exit(0);
    }

    //9999  172.16.2.209
}
