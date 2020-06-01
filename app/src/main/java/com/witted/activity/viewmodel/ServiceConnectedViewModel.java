package com.witted.activity.viewmodel;

import com.witted.service.remote.OnServiceConnectionListener;
import com.witted.service.remote.RemoteManager;
import com.witted.utils.LogUtils;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class ServiceConnectedViewModel extends ViewModel implements OnServiceConnectionListener {


    private final String TAG=this.getClass().getName();


    private MutableLiveData<Boolean> mServiceConnectLiveData=new MutableLiveData<>();


    public ServiceConnectedViewModel() {
        RemoteManager.INST.addOnServiceConnectionListener(this);
    }


    public MutableLiveData<Boolean> getServiceConnectLivedata(){

        boolean serviceConnectSuccess = RemoteManager.INST.isServiceConnectSuccess();

        mServiceConnectLiveData.setValue(serviceConnectSuccess);

        return mServiceConnectLiveData;
    }

    @Override
    protected void onCleared() {
        super.onCleared();
        LogUtils.i(TAG,"cleared");
        RemoteManager.INST.removeOnServiceConnectionListener(this);
    }

    @Override
    public void onServiceConnected() {
        mServiceConnectLiveData.postValue(true);

    }

    @Override
    public void onServiceDisconnected() {
        mServiceConnectLiveData.postValue(false);

    }
}
