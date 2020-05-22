package com.witted.net;

import org.jetbrains.annotations.NotNull;

import java.io.IOException;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;

import okhttp3.Interceptor;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.logging.HttpLoggingInterceptor;
import retrofit2.Retrofit;
import retrofit2.converter.gson.GsonConverterFactory;

public class RetrofitManager {

    public static Retrofit instance;

    public static Retrofit getInstance() {
        if (instance == null) {
            new RetrofitManager();
        }
        return instance;
    }

    /**
     * HttpLoggingInterceptor loggingInterceptor = new HttpLoggingInterceptor("tang");
     * loggingInterceptor.setPrintLevel(HttpLoggingInterceptor.Level.BODY);
     * loggingInterceptor.setColorLevel(Level.INFO);
     * OkHttpClient.Builder builder = new OkHttpClient.Builder();
     * builder.connectTimeout(5, TimeUnit.SECONDS);
     * builder.readTimeout(10, TimeUnit.SECONDS);
     * builder.writeTimeout(10, TimeUnit.SECONDS);
     * builder.retryOnConnectionFailure(true);
     * builder.addInterceptor(loggingInterceptor);
     * OkHttpClient client = builder.build();
     */


    RetrofitManager() {

        OkHttpClient.Builder builder = new OkHttpClient.Builder();
        HttpLoggingInterceptor loggingInterceptor = new HttpLoggingInterceptor();
        builder.addInterceptor(loggingInterceptor);
        builder.connectTimeout(5, TimeUnit.SECONDS);
        builder.readTimeout(10, TimeUnit.SECONDS);
        builder.writeTimeout(10, TimeUnit.SECONDS);
        builder.retryOnConnectionFailure(true);

//        builder.addInterceptor(new Interceptor() {
//            @NotNull
//            @Override
//            public Response intercept(@NotNull Chain chain) throws IOException {
//                Request request = chain.request();
//                return chain.proceed(request);
//            }
//        });

        instance = new Retrofit.Builder().baseUrl("http://172.16.1.89:8080")
                .client(builder.build())
                .addConverterFactory(GsonConverterFactory.create())
                .addCallAdapterFactory(new LiveDataCallAdapterFactory())
                .build();


    }


}
