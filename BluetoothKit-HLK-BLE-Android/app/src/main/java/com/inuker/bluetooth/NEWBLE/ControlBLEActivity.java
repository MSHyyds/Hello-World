package com.inuker.bluetooth.NEWBLE;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;

import com.inuker.bluetooth.CharacterActivity;
import com.inuker.bluetooth.ClientManager;
import com.inuker.bluetooth.CommonUtils;
import com.inuker.bluetooth.R;
import com.inuker.bluetooth.SmartLinkActivity;
import com.inuker.bluetooth.library.Code;
import com.inuker.bluetooth.library.connect.listener.BleConnectStatusListener;
import com.inuker.bluetooth.library.utils.ByteUtils;

import java.util.Calendar;

import static com.inuker.bluetooth.library.Constants.GATT_DEF_BLE_MTU_SIZE;
import static com.inuker.bluetooth.library.Constants.GATT_MAX_MTU_SIZE;
import static com.inuker.bluetooth.library.Constants.STATUS_DISCONNECTED;

public class ControlBLEActivity extends BaseActivity implements View.OnClickListener {


    private EditText edSendView,edTime,edClearLength;
    private ScrollView scrollView;
    private TextView tvLogView,tvSendCount,tvSendSuccess,tvSendError,tvReceiveCountByNotify;
    private CheckBox cbAutoSend;
    private int iSendCount = 0;
    private int iSendSuccess = 0;
    private int iSendError = 0;
    private int iReceiveCountByNotify = 0;

    private RadioButton rbZiFu,rbShiLiu;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_control_ble);

        // 校验和
        String strCheck = NetworkUtils.makeCheckSum("1234567890");

        initUI();
        bindReceiver();

    }

    private void initUI() {

        edClearLength = findViewById(R.id.edClearLength);
        edSendView = (EditText) findViewById(R.id.send_value);
        edTime = (EditText) findViewById(R.id.edTime);
        scrollView = (ScrollView) findViewById(R.id.scrollView);
        tvLogView = (TextView) findViewById(R.id.tvLog);
        tvSendCount = (TextView) findViewById(R.id.tvSendCount);
        tvSendSuccess = (TextView) findViewById(R.id.tvSendSuccess);
        tvSendError = (TextView) findViewById(R.id.tvSendError);
        tvReceiveCountByNotify = (TextView) findViewById(R.id.tvReceiveCountByNotify);

        rbZiFu = findViewById(R.id.rbZiFu);
        rbShiLiu = findViewById(R.id.rbShiLiu);

        rbZiFu.setChecked(true);
        rbShiLiu.setChecked(false);

        findViewById(R.id.button_Send).setOnClickListener(this);
        findViewById(R.id.button_ClearLog).setOnClickListener(this);
        findViewById(R.id.tvPeiWang).setOnClickListener(this);

        cbAutoSend = (CheckBox) findViewById(R.id.cbAutoSend);

        cbAutoSend.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    if (edSendView.getText().equals("") && edTime.getText().equals("")) {
                        cbAutoSend.setChecked(false);
                        return;
                    }
                    edSendView.setEnabled(false);
                    edTime.setEnabled(false);
                    handler.post(runnableSend);
                }
                else {
                    edSendView.setEnabled(true);
                    edTime.setEnabled(true);
                    handler.removeCallbacks(runnableSend);
                }
            }
        });

    }

    private void bindReceiver() {
        // 注册连接成功的回调
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BaseVolume.BROADCAST_RECVPIPE);
        intentFilter.addAction(BaseVolume.BROADCAST_SEND_TIMEOUT);
        intentFilter.addAction(BaseVolume.BROADCAST_SEND_RESULT);
        intentFilter.addAction("AutoDisconnectDevice");
        mContext.registerReceiver(broadcastReceiver,intentFilter);
    }

    Handler handler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
        }
    };

    Runnable runnableSend = new Runnable() {
        @Override
        public void run() {
            sendData();
            if (edTime.getText().toString().equals("")) {
                return;
            }
            int iTime = Integer.parseInt(edTime.getText().toString());
            handler.postDelayed(this,iTime);
        }
    };

    private String hex = "";
    private byte[] sendByte = null;
    private void sendData() {
        hex = edSendView.getText().toString();
        if (TextUtils.isEmpty(hex)) {
            return;
        }
        // 字符
        if (rbZiFu.isChecked()) {
            sendByte = hex.getBytes();
        }
        // 十六进制
        else {
            sendByte = ByteUtils.toByteArray(hex);
        }

        iSendCount = iSendCount + sendByte.length;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                tvSendCount.setText("发送总长度："+iSendCount);
                addText( "Send success"
                        + "，Write: " + hex
                        + "，Length："+sendByte.length);
            }
        });

        MainBLEActivity.getInstance().startSendData(sendByte);

    }

    private void addText( String content) {
        int iMaxLength = 10;
        Editable editable = edClearLength.getText();
        if (editable != null && !editable.equals("")) {
            iMaxLength = Integer.parseInt(editable.toString());
        }

        Calendar c = Calendar.getInstance();
        int HH = c.get(Calendar.HOUR_OF_DAY);
        int MM = c.get(Calendar.MINUTE);
        int SS = c.get(Calendar.SECOND);
        int MS = c.get(Calendar.MILLISECOND);
        String strData = String.format("[%02d:%02d:%02d:%02d] %s\n",HH,MM,SS,MS,content);
        int iLineCount = tvLogView.getLineCount();
        if (iLineCount <= iMaxLength) {
            tvLogView.append(strData);
        }
        else {
            tvLogView.setText(strData);
        }


        scrollView.post(new Runnable() {
            @Override
            public void run() {
                int offset = tvLogView.getMeasuredHeight()-scrollView.getMeasuredHeight();
                if (offset > 0) {
                    scrollView.scrollTo(0, offset);
                }
            }
        });

    }

    BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // 接收数据
            if (intent.getAction().equalsIgnoreCase(BaseVolume.BROADCAST_RECVPIPE)) {
                byte[] value = intent.getByteArrayExtra(BaseVolume.BROADCAST_RECVPIPE);
                String strNewData = "";
                // 字符
                if (rbZiFu.isChecked()) {
                    strNewData= new String(value);
                }
                // 十六进制
                else {
                    strNewData = ByteUtils.byteToString(value);
                }
                addText("Receive by Notify:"+ strNewData+"，Length："+value.length);
                iReceiveCountByNotify = iReceiveCountByNotify + value.length;
                tvReceiveCountByNotify.setText("接收总长度："+ iReceiveCountByNotify);
            }
            // 发送超时
            else if (intent.getAction().equalsIgnoreCase(BaseVolume.BROADCAST_SEND_TIMEOUT)) {

            }
            // 发送结果
            else if (intent.getAction().equalsIgnoreCase(BaseVolume.BROADCAST_SEND_RESULT)) {
                boolean isSuccess = intent.getBooleanExtra(BaseVolume.BROADCAST_SEND_RESULT,false);
                int iSendLength = intent.getIntExtra(BaseVolume.BROADCAST_SEND_LENGTH,0);
                final int iCode = intent.getIntExtra(BaseVolume.BROADCAST_SEND_CODE,0);
                if (isSuccess) {
                    iSendSuccess = iSendSuccess + iSendLength;
                    tvSendSuccess.setText("成功："+iSendSuccess);
                } else {
                    iSendError = iSendError + iSendLength;
                    tvSendError.setText("失败："+iSendError);
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            addText("Send Error:"+ Code.toString(iCode));
                        }
                    });
                }
            }
            else if (intent.getAction().equalsIgnoreCase("AutoDisconnectDevice")) {
                finish();
            }
        }
    };

    public void onResume() {
        super.onResume();
        // 注册蓝牙连接状态监听
        ClientManager.getClient().registerConnectStatusListener(MainBLEActivity.getInstance().nowSelectDevice.getAddress(), mConnectStatusListener);

    }

    public void onPause() {
        super.onPause();
        ClientManager.getClient().unregisterConnectStatusListener(MainBLEActivity.getInstance().nowSelectDevice.getAddress(), mConnectStatusListener);
    }

    private final BleConnectStatusListener mConnectStatusListener = new BleConnectStatusListener() {
        @Override
        public void onConnectStatusChanged(String mac, int status) {
            Log.e("BlueDeviceListActivity", String.format("CharacterActivity.onConnectStatusChanged status = %d", status));
            if (status == STATUS_DISCONNECTED) {

                CommonUtils.toast("disconnected");
                BlueWindowHint hintDialog = new BlueWindowHint(ControlBLEActivity.this, R.style.dialog_style, "系统提示", new BlueWindowHint.PeriodListener() {
                    public void confirmListener() {
                        onBackPressed();
                    }
                    public void cancelListener() {
                        onBackPressed();
                    }
                }, "连接异常，请重试！",true);
                hintDialog.show();
            }
        }
    };

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button_ClearLog:
                tvLogView.setText("");
                tvSendCount.setText("发送总长度：0");
                tvSendSuccess.setText("成功：0");
                tvSendError.setText("失败：0");
                tvReceiveCountByNotify.setText("接收总长度：0");
                iSendCount = 0;
                iReceiveCountByNotify = 0;
                iSendSuccess = 0;
                iSendError = 0;
                break;
            case R.id.button_Send:
                if (edSendView.getText().toString().equals("")) {
                    return;
                }
                sendData();
                break;
            case R.id.tvPeiWang:
                Intent intent = new Intent(ControlBLEActivity.this, SmartLinkActivity.class);
                startActivity(intent);
                break;
        }
    }

    public void onDestroy() {
        super.onDestroy();
        handler.removeCallbacks(runnableSend);
        mContext.unregisterReceiver(broadcastReceiver);
        ClientManager.getClient().unregisterConnectStatusListener(MainBLEActivity.getInstance().nowSelectDevice.getAddress(), mConnectStatusListener);
        ClientManager.getClient().disconnect(MainBLEActivity.getInstance().nowSelectDevice.getAddress());
    }



}
