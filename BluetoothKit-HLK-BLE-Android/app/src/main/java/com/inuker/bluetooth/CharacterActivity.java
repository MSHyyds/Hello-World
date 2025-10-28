package com.inuker.bluetooth;

import android.app.Activity;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.inuker.bluetooth.library.Code;
import com.inuker.bluetooth.library.connect.listener.BleConnectStatusListener;
import com.inuker.bluetooth.library.connect.response.BleMtuResponse;
import com.inuker.bluetooth.library.connect.response.BleNotifyResponse;
import com.inuker.bluetooth.library.connect.response.BleReadResponse;
import com.inuker.bluetooth.library.connect.response.BleUnnotifyResponse;
import com.inuker.bluetooth.library.connect.response.BleWriteResponse;
import com.inuker.bluetooth.library.model.BleGattCharacter;
import com.inuker.bluetooth.library.utils.BluetoothLog;
import com.inuker.bluetooth.library.utils.ByteUtils;

import static com.inuker.bluetooth.library.Constants.*;

import java.util.Calendar;
import java.util.List;
import java.util.UUID;

/**
 * Created by dingjikerbo on 2016/9/6.
 */
public class CharacterActivity extends Activity implements View.OnClickListener {

    private String mMac;
    private UUID mService;
    private UUID mCharacterRead;
    private UUID mCharacterWrite;
    private TextView mTvTitle;
    private Button mBtnRead;
    private Button mBtnWrite;
    private EditText mEtInput;
    private Button mBtnNotify;
    private Button mBtnUnnotify;
    private EditText mEtInputMtu;
    private Button mBtnRequestMtu;
    private EditText edSendView,edTime;
    private ScrollView scrollView;
    private TextView tvLogView,tvSendCount,tvSendSuccess,tvSendError,tvReceiveCountByNotify;
    private CheckBox cbAutoSend;
    private Switch swNotify;
    private int iSendCount = 0;
    private int iSendSuccess = 0;
    private int iSendError = 0;
    private int iReceiveCountByNotify = 0;

    private RadioButton rbZiFu,rbShiLiu;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.character_activity);

        Intent intent = getIntent();
        mMac = intent.getStringExtra("mac");
        mService = (UUID) intent.getSerializableExtra("service");
        String strUUIDSer = mService.toString();

        mCharacterRead = (UUID) intent.getSerializableExtra("character");
        String strUUIDRead = mCharacterRead.toString();


        mCharacterWrite = (UUID) intent.getSerializableExtra("characterWrite");
        String strUUIDWrite = mCharacterWrite.toString();

        mTvTitle = (TextView) findViewById(R.id.title);
        mTvTitle.setText(String.format("%s", mMac));

        mBtnRead = (Button) findViewById(R.id.read);

        mBtnWrite = (Button) findViewById(R.id.write);
        mEtInput = (EditText) findViewById(R.id.input);

        mBtnNotify = (Button) findViewById(R.id.notify);
        mBtnUnnotify = (Button) findViewById(R.id.unnotify);

        mEtInputMtu = (EditText) findViewById(R.id.et_input_mtu);
        mBtnRequestMtu = (Button) findViewById(R.id.btn_request_mtu);

        rbZiFu = findViewById(R.id.rbZiFu);
        rbShiLiu = findViewById(R.id.rbShiLiu);

        rbZiFu.setChecked(true);
        rbShiLiu.setChecked(false);

        mBtnRead.setOnClickListener(this);
        mBtnWrite.setOnClickListener(this);

        mBtnNotify.setOnClickListener(this);
        mBtnNotify.setEnabled(true);

        mBtnUnnotify.setOnClickListener(this);
        mBtnUnnotify.setEnabled(false);

        mBtnRequestMtu.setOnClickListener(this);

        initNewUI();

        // 设置蓝牙的MTU值，决定每次收发数据的长度，默认MTU=23，即20个字节。蓝牙5.0的最大MTU=512，即509个字节。
        ClientManager.getClient().requestMtu(mMac, 503, mMtuResponse);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("AutoDisconnectDevice");
        registerReceiver(myNetReceiver, intentFilter);

    }

    /****
     * 网络监听
     */
    private BroadcastReceiver myNetReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            // 配网信息发送成功，设备需要重启
            if (action.equals("AutoDisconnectDevice")) {
                finish();
            }
        }
    };

    private void initNewUI() {
        edSendView = (EditText) findViewById(R.id.send_value);
        edTime = (EditText) findViewById(R.id.edTime);
        scrollView = (ScrollView) findViewById(R.id.scrollView);
        tvLogView = (TextView) findViewById(R.id.tvLog);
        tvSendCount = (TextView) findViewById(R.id.tvSendCount);
        tvSendSuccess = (TextView) findViewById(R.id.tvSendSuccess);
        tvSendError = (TextView) findViewById(R.id.tvSendError);
        tvReceiveCountByNotify = (TextView) findViewById(R.id.tvReceiveCountByNotify);

        findViewById(R.id.button_Send).setOnClickListener(this);
        findViewById(R.id.button_ClearLog).setOnClickListener(this);
        findViewById(R.id.tvPeiWang).setOnClickListener(this);

        cbAutoSend = (CheckBox) findViewById(R.id.cbAutoSend);
        swNotify = (Switch) findViewById(R.id.swNOTIFY);

        // Notify通知
        swNotify.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                openNotify(isChecked);
            }
        });

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

    private final BleReadResponse mReadRsp = new BleReadResponse() {
        @Override
        public void onResponse(int code, byte[] data) {
            if (code == REQUEST_SUCCESS) {
                mBtnRead.setText(String.format("read: %s", ByteUtils.byteToString(data)));
                CommonUtils.toast("success");
            } else {
                CommonUtils.toast("failed");
                mBtnRead.setText("read");
            }
        }
    };

    private final BleWriteResponse mWriteRsp = new BleWriteResponse() {
        @Override
        public void onResponse(final int code) {
            String hex = edSendView.getText().toString();
            if (code == REQUEST_SUCCESS) {
                iSendSuccess = iSendSuccess + hex.length();
                tvSendSuccess.setText("成功："+iSendSuccess);
            } else {
                iSendError = iSendError + hex.length();
                tvSendError.setText("失败："+iSendError);

                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        addText("Send Error:"+ Code.toString(code));
                    }
                });
            }
        }
    };

    private final BleNotifyResponse mNotifyRsp = new BleNotifyResponse() {
        @Override
        public void onNotify(UUID service, UUID character,final byte[] value) {
            if (service.equals(mService) && character.equals(mCharacterRead)) {
                mBtnNotify.setText(String.format("%s", ByteUtils.byteToString(value)));
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        String strNewData = "";
                        // 字符
                        if (rbZiFu.isChecked()) {
                            strNewData= new String(value);
                        }
                        // 十六进制
                        else {
                            strNewData = ByteUtils.byteToString(value);
                        }
                        addText("Receive by Notify:"+ strNewData+"，Length："+strNewData.length());
                        iReceiveCountByNotify = iReceiveCountByNotify + strNewData.length();
                        tvReceiveCountByNotify.setText("接收总长度："+ iReceiveCountByNotify);
                        Log.e("测试","Receive by Notify:"+ strNewData+"，Length："+strNewData.length());
                    }
                });
            }
        }

        @Override
        public void onResponse(int code) {
            if (code == REQUEST_SUCCESS) {
                mBtnNotify.setEnabled(false);
                mBtnUnnotify.setEnabled(true);
                CommonUtils.toast("success");
            } else {
                CommonUtils.toast("failed");
            }
        }
    };

    private final BleUnnotifyResponse mUnnotifyRsp = new BleUnnotifyResponse() {
        @Override
        public void onResponse(int code) {
            if (code == REQUEST_SUCCESS) {
                CommonUtils.toast("success");
                mBtnNotify.setEnabled(true);
                mBtnUnnotify.setEnabled(false);
            } else {
                CommonUtils.toast("failed");
            }
        }
    };

    private final BleMtuResponse mMtuResponse = new BleMtuResponse() {
        @Override
        public void onResponse(int code, Integer data) {
            if (code == REQUEST_SUCCESS) {
                CommonUtils.toast("request mtu success,mtu = " + data);
            } else {
                CommonUtils.toast("request mtu failed");
            }
        }
    };

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.read:
                ClientManager.getClient().read(mMac, mService, mCharacterRead, mReadRsp);
                break;
            case R.id.write:
                ClientManager.getClient().write(mMac, mService, mCharacterWrite,
                        ByteUtils.stringToBytes(mEtInput.getText().toString()), mWriteRsp);
                break;
            case R.id.notify:
                ClientManager.getClient().notify(mMac, mService, mCharacterRead, mNotifyRsp);
                break;
            case R.id.unnotify:
                ClientManager.getClient().unnotify(mMac, mService, mCharacterRead, mUnnotifyRsp);
                break;
            case R.id.btn_request_mtu:
                String mtuStr = mEtInputMtu.getText().toString();
                if (TextUtils.isEmpty(mtuStr)) {
                    CommonUtils.toast("MTU不能为空");
                    return;
                }
                int mtu = Integer.parseInt(mtuStr);
                if (mtu < GATT_DEF_BLE_MTU_SIZE || mtu > GATT_MAX_MTU_SIZE) {
                    CommonUtils.toast("MTU不不在范围内");
                    return;
                }
                ClientManager.getClient().requestMtu(mMac, mtu, mMtuResponse);
                break;
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

                Intent intent = new Intent(CharacterActivity.this, SmartLinkActivity.class);
                intent.putExtra("mac", mMac);
                intent.putExtra("service", mService);
                intent.putExtra("characterWrite", mCharacterWrite);
                intent.putExtra("character", mCharacterRead);
                startActivity(intent);

                break;
        }
    }

    private final BleConnectStatusListener mConnectStatusListener = new BleConnectStatusListener() {
        @Override
        public void onConnectStatusChanged(String mac, int status) {
            BluetoothLog.v(String.format("CharacterActivity.onConnectStatusChanged status = %d", status));

            if (status == STATUS_DISCONNECTED) {
                CommonUtils.toast("disconnected");
                mBtnRead.setEnabled(false);
                mBtnWrite.setEnabled(false);
                mBtnNotify.setEnabled(false);
                mBtnUnnotify.setEnabled(false);

                mTvTitle.postDelayed(new Runnable() {

                    @Override
                    public void run() {
                        finish();
                    }
                }, 300);
            }
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
        ClientManager.getClient().registerConnectStatusListener(mMac, mConnectStatusListener);
    }

    @Override
    protected void onPause() {
        super.onPause();
        ClientManager.getClient().unregisterConnectStatusListener(mMac, mConnectStatusListener);
    }

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

    Handler handler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
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
        ClientManager.getClient().write(mMac, mService, mCharacterWrite,sendByte, mWriteRsp);

    }

    private void addText( String content) {
        Calendar c = Calendar.getInstance();
        int HH = c.get(Calendar.HOUR_OF_DAY);
        int MM = c.get(Calendar.MINUTE);
        int SS = c.get(Calendar.SECOND);
        int MS = c.get(Calendar.MILLISECOND);
        String strTime = String.format("[%02d:%02d:%02d:%02d] %s\n",HH,MM,SS,MS,content);
        tvLogView.append(strTime);
        tvLogView.append("\n");

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

    private void openNotify(boolean isOpen) {
        if (isOpen) {
            ClientManager.getClient().notify(mMac, mService, mCharacterRead, mNotifyRsp);
        }
        else {
            ClientManager.getClient().unnotify(mMac, mService, mCharacterRead, mUnnotifyRsp);
        }
    }

    public boolean onCreateOptionsMenu(Menu menu){
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_right, menu);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item){
        switch (item.getItemId()) {
            case R.id.action_cart://监听配置按钮

                break;
        }
        return super.onOptionsItemSelected(item);
    }




    public void onDestroy() {
        super.onDestroy();
        handler.removeCallbacks(runnableSend);
    }



}
