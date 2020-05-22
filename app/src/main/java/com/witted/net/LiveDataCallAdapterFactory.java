package com.witted.net;

import android.util.Log;

import java.lang.annotation.Annotation;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.concurrent.atomic.AtomicBoolean;

import androidx.lifecycle.LiveData;
import retrofit2.Call;
import retrofit2.CallAdapter;
import retrofit2.Callback;
import retrofit2.Response;
import retrofit2.Retrofit;

public class LiveDataCallAdapterFactory extends CallAdapter.Factory {

    private String TAG="LiveDataCallAdapterFactory";

    @Override
    public CallAdapter<?, ?> get(Type returnType, Annotation[] annotations, Retrofit retrofit) {
        //returnType是 请求时的返回类型
        //如LiveData<User>
        //getRawType返回的是LiveData的class
        Log.i(TAG, "get: "+returnType);
        if (getRawType(returnType)!= LiveData.class) {
            return null;
        }

        //getParameterUpperBound 获取泛型的类型
        //如LiveData<User,T> index指泛型的位置,
        Type bound = getParameterUpperBound(0, (ParameterizedType) returnType);

        Log.i(TAG, "get: "+bound.toString());

        return new LiveDataCallAdapter(bound);
    }



    class  LiveDataCallAdapter<T> implements  CallAdapter{

        private  Type mReturnType;

        public LiveDataCallAdapter(Type returnType) {

            mReturnType = returnType;
        }

        @Override
        public Type responseType() {
            return mReturnType;
        }

        @Override
        public LiveData<T> adapt(Call call) {
            AtomicBoolean atomicBoolean = new AtomicBoolean(false);
            return new LiveData<T>() {
                @Override
                protected void onActive() {
                    super.onActive();
                    if(atomicBoolean.compareAndSet(false,true)){
                        call.enqueue(new Callback() {
                            @Override
                            public void onResponse(Call call, Response response) {

                                Log.i(TAG, "onResponse: "+response.body().toString());
                                postValue((T) response.body());
                            }

                            @Override
                            public void onFailure(Call call, Throwable t) {
                                Log.i(TAG, "onFailure: "+t.getMessage());
                                postValue(null);
                            }
                        });
                    }
                }
            };
        }
    }
}
