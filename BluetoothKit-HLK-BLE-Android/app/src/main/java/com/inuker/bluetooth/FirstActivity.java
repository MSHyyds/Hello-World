package com.inuker.bluetooth;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import com.inuker.bluetooth.NEWBLE.MainBLEActivity;


public class FirstActivity extends Activity {

    private static final int REQUEST_CODE = 0; // 请求�?

    // �?�?的全部权�?
    @SuppressLint("InlinedApi")
    static final String[] PERMISSIONS = new String[]{

            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.CHANGE_WIFI_STATE,
            Manifest.permission.CHANGE_WIFI_MULTICAST_STATE,
            Manifest.permission.ACCESS_WIFI_STATE,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.ACCESS_NETWORK_STATE,
            Manifest.permission.CHANGE_NETWORK_STATE,

            Manifest.permission.REQUEST_INSTALL_PACKAGES,
            Manifest.permission.ACCESS_COARSE_LOCATION,// 位置权限
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_BACKGROUND_LOCATION, //全新的定位权限

    };

    private PermissionsChecker mPermissionsChecker; // 权限�?测器

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mPermissionsChecker = new PermissionsChecker(this);

    }

    public void onResume() {
        super.onResume();
        // 缺少权限�?, 进入权限配置页面
        if (mPermissionsChecker.lacksPermissions(PERMISSIONS)) {
            startPermissionsActivity();
        } else {
            startActivity(new Intent(this, MainBLEActivity.class));
            finish();
        }

    }

    private void startPermissionsActivity() {
        PermissionsActivity.startActivityForResult(this, REQUEST_CODE, PERMISSIONS);
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        // 拒绝�?, 关闭页面, 缺少主要权限, 无法运行
        if (requestCode == REQUEST_CODE && resultCode == PermissionsActivity.PERMISSIONS_DENIED) {
            finish();
        } else {
            startActivity(new Intent(this, MainBLEActivity.class));
            finish();
        }
    }


}
