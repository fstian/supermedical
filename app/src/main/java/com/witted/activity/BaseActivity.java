package com.witted.activity;

import android.os.Bundle;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import butterknife.ButterKnife;

public abstract class BaseActivity extends AppCompatActivity {
    public final String TAG=BaseActivity.class.getName();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        beforeViews();
        setContentView(contentLayout());
        ButterKnife.bind(this);
        afterViews();
    }

    private void beforeViews() {
    }

    protected  void afterViews(){

    };

    protected abstract int contentLayout();
}
