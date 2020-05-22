package com.witted.net;

import com.witted.viewmodel.BaseResult;
import com.witted.viewmodel.User;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import retrofit2.http.Body;
import retrofit2.http.Headers;
import retrofit2.http.POST;

public interface ServiceApi {

    @Headers("Content-Type: application/json;charset=UTF-8")
    @POST("register/")
    LiveData<BaseResult<String>>  getLoginResult(@Body User user);


}
