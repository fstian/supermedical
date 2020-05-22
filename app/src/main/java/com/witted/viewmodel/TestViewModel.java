package com.witted.viewmodel;

import com.witted.net.RetrofitManager;
import com.witted.net.ServiceApi;
import com.witted.service.remote.RemoteManager;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class TestViewModel extends ViewModel {


    private MutableLiveData<String> testLiveDate=new MutableLiveData<>();

    private LiveData<String> mLiveData=new MutableLiveData<>();


    public LiveData<BaseResult<String>> getLoginResult(User user){

        return  RetrofitManager.getInstance().create(ServiceApi.class).getLoginResult(user);
    }


    @Override
    protected void onCleared() {
        super.onCleared();
    }
}
