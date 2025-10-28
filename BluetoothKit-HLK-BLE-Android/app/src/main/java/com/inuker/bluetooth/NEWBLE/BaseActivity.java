package com.inuker.bluetooth.NEWBLE;

import android.content.Context;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;
import android.widget.Toast;

public class BaseActivity extends AppCompatActivity {

    protected Context mContext;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        this.mContext = this;

    }

    protected void showToast(String str) {
        Toast.makeText(getApplicationContext(), str, Toast.LENGTH_SHORT).show();
    }


    public void onResume() {
        super.onResume();
    }

    public void onPause() {
        super.onPause();
    }



}
