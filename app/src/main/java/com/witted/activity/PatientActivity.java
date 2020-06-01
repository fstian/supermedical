package com.witted.activity;

import android.os.Handler;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.TextClock;
import android.widget.TextView;

import com.witted.R;

import androidx.annotation.LayoutRes;
import butterknife.BindView;
import q.rorbin.badgeview.QBadgeView;

/**
 * @author YT
 * 未输液界面
 */
public class PatientActivity extends BaseActivity {

    @BindView(R.id.patient_fl_container)
    FrameLayout mContainer;
    @BindView(R.id.patient_tv_nurseLevel)
    TextView mNurseLevel;


    @BindView(R.id.patient_tv_notice)
    TextView mTvNotice;
    private TextView mTvInHospitalCode;
    private TextView mTvInHospitalTime;
    private TextView mTvDoctor;
    private TextView mTvNurse;
    private TextView mTvAllergy;
    private TextView mTvTips;

    @Override
    protected int contentLayout() {
        return R.layout.activity_patient;
    }

    @Override
    protected void beforeViews() {
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

    }

    @Override
    protected void afterViews() {

        show(R.layout.patient_layout_expand);
//        new Handler().postDelayed(new Runnable() {
//            @Override
//            public void run() {
//                show(R.layout.patient_layout_contract);
//            }
//        },5*1000);

        new QBadgeView(this).bindTarget(mTvNotice).setBadgeNumber(5);
    }


    private void show(@LayoutRes int res){
        View view = View.inflate(this,res, null);
        mContainer.removeAllViews();
        mContainer.addView(view);

        mTvInHospitalCode = (TextView) view.findViewById(R.id.layout_tv_hospital_code);
        mTvInHospitalTime = (TextView) view.findViewById(R.id.layout_tv_hospital_time);
        mTvDoctor = (TextView) view.findViewById(R.id.layout_tv_hospital_doctor);
        mTvNurse = (TextView) view.findViewById(R.id.layout_tv_hospital_nurse);

        if(res==R.layout.patient_layout_contract){
            mTvAllergy = (TextView) view.findViewById(R.id.layout_tv_allergy);
            mTvTips = (TextView) view.findViewById(R.id.layout_tv_tips);
        }

    }


    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }



}
