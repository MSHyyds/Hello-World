package com.inuker.bluetooth;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

import com.inuker.bluetooth.library.Code;
import com.inuker.bluetooth.library.connect.listener.BleConnectStatusListener;
import com.inuker.bluetooth.library.connect.response.BleNotifyResponse;
import com.inuker.bluetooth.library.connect.response.BleWriteResponse;
import com.inuker.bluetooth.library.utils.BluetoothLog;
import com.inuker.bluetooth.library.utils.ByteUtils;

import java.util.UUID;

import static com.inuker.bluetooth.library.Constants.REQUEST_SUCCESS;
import static com.inuker.bluetooth.library.Constants.STATUS_DISCONNECTED;

public class TestDemoActivity extends Activity implements View.OnClickListener{
    private String mMac;
    private UUID mService;
    private UUID mCharacter;

    private TextView tvMac,tvServer,tvCharacter;
    private Switch swONOFF;
    private SeekBar sbProgress;
    private TextView tvProgress;

    /** 0x00：没发任何数据，0x01:发开，0x02:发关，0x03：进度条 */
    private int iNowSendState = 0x00;
    private final String COMMAND_SWITCH_ON = "UK001!";
    private final String COMMAND_SWITCH_OFF = "UK004!";
    private final String COMMAND_SEEKBAR_VALUE = "UT";


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_demo);

        Intent intent = getIntent();
        mMac = intent.getStringExtra("mac");
        mService = (UUID) intent.getSerializableExtra("service");
        mCharacter = (UUID) intent.getSerializableExtra("character");

        initUI();

        handler.sendEmptyMessageDelayed(0,1000);

    }

    private void initUI() {

        tvMac = (TextView) findViewById(R.id.tvMac);
        tvServer = (TextView) findViewById(R.id.tvServer);
        tvCharacter = (TextView) findViewById(R.id.tvCharacter);
        tvProgress = (TextView) findViewById(R.id.tvProgress);

        tvMac.setText(""+mMac);
        String strServer = mService.toString();
        tvServer.setText(""+strServer.substring(0,strServer.indexOf("-")));
        String strCharacter = mCharacter.toString();
        tvCharacter.setText(""+strCharacter.substring(0,strCharacter.indexOf("-")));

        sbProgress = (SeekBar) findViewById(R.id.sbProgress);
        sbProgress.setEnabled(false);

        tvProgress = (TextView) findViewById(R.id.tvProgress);

        findViewById(R.id.tvBack).setOnClickListener(this);

        swONOFF.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                // 打开
                if (b) {
//                    swONOFF.setSwitchTextAppearance(DeviceControlActivity.this,R.style.s_true);
                    iNowSendState = 0x01;
                    sendData(COMMAND_SWITCH_ON);
                }
                // 关闭
                else {
//                    swONOFF.setSwitchTextAppearance(DeviceControlActivity.this,R.style.x1);
                    iNowSendState = 0x02;
                    sendData(COMMAND_SWITCH_OFF);
                }
            }
        });
        sbProgress.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                tvProgress.setText("Now Temp:"+(150+seekBar.getProgress()));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                String strValue = (150+seekBar.getProgress())+"";
                String strData = COMMAND_SEEKBAR_VALUE + strValue + "!";
                sendData(strData);
            }
        });
    }


    private void sendData(String strData) {
        Log.e("TestDemoActivity", "sendData: "+strData,null );
        byte[] sendByte = strData.getBytes();
        ClientManager.getClient().write(mMac, mService, mCharacter,sendByte, mWriteRsp);

    }

    Handler handler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            ClientManager.getClient().notify(mMac, mService, mCharacter, mNotifyRsp);

        }
    };

    /** 接收数据 */
    private final BleNotifyResponse mNotifyRsp = new BleNotifyResponse() {
        @Override
        public void onNotify(UUID service, UUID character,final byte[] value) {
            if (service.equals(mService) && character.equals(mCharacter)) {
                String receiveCmd = new String(value);
                Log.e("TestDemoActivity", "onNotify: "+receiveCmd,null );
                // 回复的OK
                if (receiveCmd.indexOf("OK") >= 0) {
                    // 当前发的开
                    if (iNowSendState == 0x01) {
                        sbProgress.setEnabled(true);
                    }
                    // 当前发的关
                    else if (iNowSendState == 0x02) {
                        sbProgress.setEnabled(false);
                    }
                }
            }
        }

        @Override
        public void onResponse(int code) {
            if (code == REQUEST_SUCCESS) {
//                CommonUtils.toast("success");
            } else {
                CommonUtils.toast("failed");
            }
        }
    };

    private final BleWriteResponse mWriteRsp = new BleWriteResponse() {
        @Override
        public void onResponse(final int code) {
            if (code == REQUEST_SUCCESS) {
//                CommonUtils.toast("success");
            } else {
                CommonUtils.toast(Code.toString(code));
            }
        }
    };

    protected void onResume() {
        super.onResume();
        ClientManager.getClient().registerConnectStatusListener(mMac, mConnectStatusListener);
    }

    @Override
    protected void onPause() {
        super.onPause();
        ClientManager.getClient().unregisterConnectStatusListener(mMac, mConnectStatusListener);
    }

    private final BleConnectStatusListener mConnectStatusListener = new BleConnectStatusListener() {
        @Override
        public void onConnectStatusChanged(String mac, int status) {
            BluetoothLog.v(String.format("CharacterActivity.onConnectStatusChanged status = %d", status));

            if (status == STATUS_DISCONNECTED) {
                CommonUtils.toast("disconnected");
                finish();
            }
        }
    };


    @Override
    public void onClick(View view) {

        switch (view.getId()) {

            case R.id.tvBack:

                finish();

                break;

        }


    }
}
