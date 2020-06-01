package com.witted.activity.viewmodel;

import com.witted.service.remote.OnTcpConnectionListener;
import com.witted.service.remote.RemoteManager;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class TcpConnectedViewModel extends ViewModel implements OnTcpConnectionListener {

    private MutableLiveData<String> mTcpConnectedViewModel=new MutableLiveData<>();


    public TcpConnectedViewModel() {
        RemoteManager.INST.addOnTcpConnectionListener(this);
    }

    public MutableLiveData<String>  getTcpConnectedViewModel(){

        return mTcpConnectedViewModel;
    }

    @Override
    public void connectSuccess() {

        mTcpConnectedViewModel.postValue("success");

    }

    @Override
    public void connectFail(String err) {
        mTcpConnectedViewModel.postValue(err);
    }
}
