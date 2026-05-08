package com.inuker.bluetooth.NEWBLE;

import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.TextView;

import com.inuker.bluetooth.AppConstants;
import com.inuker.bluetooth.ClientManager;
import com.inuker.bluetooth.DeviceListAdapter;
import com.inuker.bluetooth.R;
import com.inuker.bluetooth.library.Code;
import com.inuker.bluetooth.library.Constants;
import com.inuker.bluetooth.library.connect.listener.BleConnectStatusListener;
import com.inuker.bluetooth.library.connect.listener.BluetoothStateListener;
import com.inuker.bluetooth.library.connect.options.BleConnectOptions;
import com.inuker.bluetooth.library.connect.response.BleConnectResponse;
import com.inuker.bluetooth.library.connect.response.BleMtuResponse;
import com.inuker.bluetooth.library.connect.response.BleNotifyResponse;
import com.inuker.bluetooth.library.connect.response.BleWriteResponse;
import com.inuker.bluetooth.library.model.BleGattCharacter;
import com.inuker.bluetooth.library.model.BleGattProfile;
import com.inuker.bluetooth.library.model.BleGattService;
import com.inuker.bluetooth.library.search.SearchRequest;
import com.inuker.bluetooth.library.search.SearchResult;
import com.inuker.bluetooth.library.search.response.SearchResponse;
import com.inuker.bluetooth.library.utils.BluetoothLog;
import com.inuker.bluetooth.library.utils.ByteUtils;
import com.inuker.bluetooth.view.PullRefreshListView;
import com.inuker.bluetooth.view.PullToRefreshFrameLayout;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import static com.inuker.bluetooth.library.Constants.REQUEST_SUCCESS;

public class MainBLEActivity extends BaseActivity {

    private PullToRefreshFrameLayout mRefreshLayout;
    private PullRefreshListView mListView;
    private DeviceListAdapter mAdapter;
    private TextView mTvTitle;

    private List<SearchResult> mDevices;

    public SearchResult nowSelectDevice = null;
    public ProgressDialog mpDialog;
    private static MainBLEActivity mainBLEActivity;
    public static MainBLEActivity getInstance() {
        return mainBLEActivity;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mainBLEActivity = this;

        initView();
        initDialog();
        searchDevice();
        ClientManager.getClient().registerBluetoothStateListener(new BluetoothStateListener() {
            @Override
            public void onBluetoothStateChanged(boolean openOrClosed) {
                BluetoothLog.v(String.format("onBluetoothStateChanged %b", openOrClosed));
            }
        });
    }


    private void initView() {
        mDevices = new ArrayList<SearchResult>();
        mTvTitle = (TextView) findViewById(R.id.title);
        mRefreshLayout = (PullToRefreshFrameLayout) findViewById(R.id.pulllayout);
        mListView = mRefreshLayout.getPullToRefreshListView();
        mAdapter = new DeviceListAdapter(this);
        mListView.setAdapter(mAdapter);
        mListView.setOnRefreshListener(new PullRefreshListView.OnRefreshListener() {
            @Override
            public void onRefresh() {
                searchDevice();
            }
        });
    }

    protected void initDialog() {
        mpDialog = new ProgressDialog(this);
        mpDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mpDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface arg0) {
            }
        });
        mpDialog.setCancelable(true);
        mpDialog.setCanceledOnTouchOutside(false);
    }

    private void searchDevice() {
        SearchRequest request = new SearchRequest.Builder()
                .searchBluetoothLeDevice(5000, 2).build();

        ClientManager.getClient().search(request, mSearchResponse);
    }

    private final SearchResponse mSearchResponse = new SearchResponse() {
        @Override
        public void onSearchStarted() {
            BluetoothLog.e("MainActivity.onSearchStarted");
            mListView.onRefreshComplete(true);
            mRefreshLayout.showState(AppConstants.LIST);
            mTvTitle.setText(R.string.string_refreshing);
            mDevices.clear();
        }

        @Override
        public void onDeviceFounded(SearchResult device) {
            BluetoothLog.e("MainActivity.onDeviceFounded " + device.device.getAddress());
            if (!mDevices.contains(device)) {
                mDevices.add(device);
                mAdapter.setDataList(mDevices);
            }

            if (mDevices.size() > 0) {
                mRefreshLayout.showState(AppConstants.LIST);
            }
        }

        @Override
        public void onSearchStopped() {
            BluetoothLog.e("MainActivity.onSearchStopped");
            mListView.onRefreshComplete(true);
            mRefreshLayout.showState(AppConstants.LIST);

            mTvTitle.setText(R.string.devices);
        }

        @Override
        public void onSearchCanceled() {
            BluetoothLog.e("MainActivity.onSearchCanceled");

            mListView.onRefreshComplete(true);
            mRefreshLayout.showState(AppConstants.LIST);

            mTvTitle.setText(R.string.devices);
        }
    };

    /** 设备列表的点击事件 */
    public void onItemSelectClick(SearchResult searchResult) {
        Log.e("BlueDeviceListActivity", "onItemSelectClick，单击事件："+searchResult.getName()+"，"+searchResult.getAddress());
        ClientManager.getClient().stopSearch();
        // 单击连接
        nowSelectDevice = searchResult;
        showDialog("开始连接...");
        ClientManager.getClient().registerConnectStatusListener(nowSelectDevice.getAddress(), mConnectStatusListener);
        connectDevice();
    }

    public void showDialog(String strMsg) {
        mpDialog.setMessage(strMsg);
        mpDialog.show();
    }

    public void dismissDialog() {
        mpDialog.dismiss();
    }

    private final BleConnectStatusListener mConnectStatusListener = new BleConnectStatusListener() {
        @Override
        public void onConnectStatusChanged(String mac, int status) {
            Log.e("BlueDeviceListActivity", String.format("DeviceDetailActivity onConnectStatusChanged %d in %s",
                    status, Thread.currentThread().getName()));

        }
    };

    private void connectDevice() {
        BleConnectOptions options = new BleConnectOptions.Builder()
                .setConnectRetry(3)// 连接如果失败重试3次
                .setConnectTimeout(20000)// 连接超时20s
                .setServiceDiscoverRetry(3)// 发现服务如果失败重试3次
                .setServiceDiscoverTimeout(10000)// 发现服务超时10s
                .build();

        ClientManager.getClient().connect(nowSelectDevice.getAddress(), options, new BleConnectResponse() {
            @Override
            public void onResponse(int code, BleGattProfile profile) {
                BluetoothLog.v(String.format("profile:\n%s", profile));
                if (code == Constants.REQUEST_SUCCESS) {
                    Log.e("BlueDeviceListActivity", "连接成功，获取并绑定通道！");
                    mpDialog.setMessage("连接成功，获取通道...");
                    // 设置蓝牙的MTU值，决定每次收发数据的长度，默认MTU=23，即20个字节。蓝牙5.0的最大MTU=512，即509个字节。
                    ClientManager.getClient().requestMtu(nowSelectDevice.getAddress(), 512, mMtuResponse);
                    getProFileInfo(profile);

                }
                else {
                    Log.e("BlueDeviceListActivity", "连接失败，code："+ Code.toString(code));
                    showToast("连接失败，code："+ Code.toString(code));
                    dismissDialog();
                    ClientManager.getClient().disconnect(nowSelectDevice.getAddress());
                }
            }
        });
    }

    private final BleMtuResponse mMtuResponse = new BleMtuResponse() {
        @Override
        public void onResponse(int code, Integer data) {
            if (code == REQUEST_SUCCESS) {
                Log.e("BlueDeviceListActivity", "request mtu success,!!!!!!!!! MTU = " + data+"!!!!!!!!");
                showToast("request mtu success,MTU = " + data);
            } else {
                showToast("request mtu failed");
            }
        }
    };

    // 3432，名称是 HLK-B10/HLK-B20/HLK-B30时， 通讯 服务 和 UUID
    private final String UUID_BY_SERVER_3432 = "0000fff0-0000-1000-8000-00805f9b34fb";
    private final String UUID_BY_READ_3432 = "0000fff1-0000-1000-8000-00805f9b34fb";
    private final String UUID_BY_WRITE_3432 = "0000fff2-0000-1000-8000-00805f9b34fb";
//    private final String UUID_BY_READ_3432 = "0000fff3-0000-1000-8000-00805f9b34fb";
//    private final String UUID_BY_WRITE_3432 = "0000fff4-0000-1000-8000-00805f9b34fb";

    // 7697，名称是 HLK-BLE时， 通讯 服务 和 UUID
    private final String UUID_BY_SERVER_7697 = "000018aa-0000-1000-8000-00805f9b34fb";
    private final String UUID_BY_READ_7697 = "00002aaa-0000-1000-8000-00805f9b34fb";
    private final String UUID_BY_WRITE_7697 = "00002aaa-0000-1000-8000-00805f9b34fb";

    private UUID mService;
    private UUID mCharacterRead;
    private UUID mCharacterWrite;
    // 获取所有的Server 和 UDID
    private void getProFileInfo(BleGattProfile profile) {
        List<BleGattService> services = profile.getServices();
        boolean isServer = false;
        boolean isReadUUID = false;
        boolean isWriteUUID = false;

        for (BleGattService service : services) {
            // 3432的透传Server B10/B20/B30设备
            if (service.getUUID().toString().equalsIgnoreCase(UUID_BY_SERVER_3432)) {
                isServer = true;
                mService = service.getUUID();
                List<BleGattCharacter> characters = service.getCharacters();
                for (BleGattCharacter character : characters) {
                    if (character.getUuid().toString().equalsIgnoreCase(UUID_BY_READ_3432)) {
                        isReadUUID = true;
                        mCharacterRead = character.getUuid();
                        ClientManager.getClient().notify(nowSelectDevice.getAddress(), mService, mCharacterRead, mNotifyRsp);
                    }
                    else if (character.getUuid().toString().equalsIgnoreCase(UUID_BY_WRITE_3432)) {
                        isWriteUUID = true;
                        mCharacterWrite = character.getUuid();
                    }
                }
                break;
            }
            // 7697的透传Server,58S/58D设备
            else if (service.getUUID().toString().equalsIgnoreCase(UUID_BY_SERVER_7697)) {
                isServer = true;
                mService = service.getUUID();
                List<BleGattCharacter> characters = service.getCharacters();
                for (BleGattCharacter character : characters) {
                    if (character.getUuid().toString().equalsIgnoreCase(UUID_BY_READ_7697)) {
                        isReadUUID = true;
                        mCharacterRead = character.getUuid();
                        ClientManager.getClient().notify(nowSelectDevice.getAddress(), mService, mCharacterRead, mNotifyRsp);
                        isWriteUUID = true;
                        mCharacterWrite = character.getUuid();
                        break;
                    }
                }
                break;
            }
        }

        // 未找到透传Server
        if (!isServer) {
            dismissDialog();
            ClientManager.getClient().disconnect(nowSelectDevice.getAddress());
            showToast("未找到透传Server!");
        }
        // 未找到读取通道
        if (!isServer) {
            dismissDialog();
            ClientManager.getClient().disconnect(nowSelectDevice.getAddress());
            showToast("通道不完善！isReadUUID = "+isReadUUID);
        }
        // 未找到写入通道
        if (!isServer) {
            dismissDialog();
            ClientManager.getClient().disconnect(nowSelectDevice.getAddress());
            showToast("通道不完善！isWriteUUID = "+isWriteUUID);
        }

    }

    private final BleNotifyResponse mNotifyRsp = new BleNotifyResponse() {
        @Override
        public void onNotify(UUID service, UUID character,final byte[] value) {
            if (service.equals(mService) && character.equals(mCharacterRead)) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        String strNewData = ByteUtils.byteToString(value);
                        strNewData = strNewData.toLowerCase();
                        Log.e("BlueDeviceListActivity","接收蓝牙数据:"+ strNewData+"，Length："+strNewData.length());
                        // 将有效数据发送到其他界面
                        sendBroadcast(new Intent(BaseVolume.BROADCAST_RECVPIPE).putExtra(BaseVolume.BROADCAST_RECVPIPE,value));

                    }
                });
            }
        }
        @Override
        public void onResponse(int code) {
            if (code == Constants.REQUEST_SUCCESS) {
                Log.e("BlueDeviceListActivity","接收广播已开启！开始同步时间！");
                strOldBuffer = "";
                dismissDialog();
                // 跳转页面
                startActivity(new Intent(mContext,ControlBLEActivity.class));
            } else {
                Log.e("BlueDeviceListActivity","接收广播开启失败！code："+ Code.toString(code));
                showToast("接收广播开启失败！code："+Code.toString(code));
                dismissDialog();
                ClientManager.getClient().disconnect(nowSelectDevice.getAddress());
            }
        }
    };

    String strOldBuffer = "";
    //数据校验
    public void SyncData(String res) {
        strOldBuffer = strOldBuffer + res;
        // 做数据校验
        while (strOldBuffer.length() >= 18) {
            // a5aaac 包头
            boolean isHead = strOldBuffer.substring(0, 6).equalsIgnoreCase(BaseVolume.COMMAND_HEAD);
            if (isHead) {
                // c5ccca 包尾的位置
                int endIndex = strOldBuffer.indexOf(BaseVolume.COMMAND_END);
                // 大于0,说明该数据中有包尾
                if (endIndex > 0) {
                    // 数据中的校验
                    String strOldCheck = strOldBuffer.substring(6, 8);
                    // 从校验的下一位取到包尾的前一位
                    String strValue = strOldBuffer.substring(8, endIndex);
                    String strNewCheck = NetworkUtils.getXORCheck(strValue);
                    // 校验一致，则认为是一条有效的指令
                    if (strOldCheck.equalsIgnoreCase(strNewCheck)) {
                        String strData = strOldBuffer.substring(0, endIndex + 6);
                        // 将有效数据发送到其他界面
                        sendBroadcast(new Intent(BaseVolume.BROADCAST_RECVPIPE).putExtra(BaseVolume.BROADCAST_RECVPIPE,strData));

                        strOldBuffer = strOldBuffer.substring(endIndex + 6);
                    }
                    // 校验和计算的校验不匹配，说明数据有问题，则继续循环
                    else {
                        strOldBuffer = strOldBuffer.substring(2);
                    }
                }
                // 未发现包尾，则可能该数据还没接收完，等待下次接收
                else {
                    break;
                }
            } else {
                strOldBuffer = strOldBuffer.substring(2);
            }
        }
    }

    /** 发送次数 */
    private int iSendCount = 0;
    private byte[] sendByte = null;
    /** 开始发送数据 */
    public void startSendData(byte[] byteData) {
        handler.removeCallbacks(runnableSendTime);
        iSendCount = 0;
        sendByte = byteData;
        Log.e("BlueDeviceListActivity", "发送数据："+sendByte+",IotID："+nowSelectDevice.getAddress());
        ClientManager.getClient().write(nowSelectDevice.getAddress(), mService, mCharacterWrite,sendByte, mWriteRsp);
//        handler.post(runnableSendTime);
    }

    /** 停止发送 */
    public void stopSendData() {
        Log.e("BlueDeviceListActivity", "停止发送！");
        handler.removeCallbacks(runnableSendTime);
    }

    /** 定时发送数据 */
    private Runnable runnableSendTime = new Runnable() {
        @Override
        public void run() {
            if (iSendCount > 2) {
                // 连续发送3次都超时了，提示用户。
                Log.e("BlueDeviceListActivity", "连续发送3次都超时了，提示用户。"+nowSelectDevice.getAddress());
                sendBroadcast(new Intent(BaseVolume.BROADCAST_SEND_TIMEOUT));
                showToast("发送超时！");
                // 断开连接
                ClientManager.getClient().disconnect(nowSelectDevice.getAddress());
                dismissDialog();
                return;
            }
            ++iSendCount;
            Log.e("BlueDeviceListActivity", "发送数据："+sendByte+",IotID："+nowSelectDevice.getAddress());
            ClientManager.getClient().write(nowSelectDevice.getAddress(), mService, mCharacterWrite,sendByte, mWriteRsp);
//            handler.postDelayed(this,5000);// 延时5秒后，重新发送
        }
    };

    Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
        }
    };

    private final BleWriteResponse mWriteRsp = new BleWriteResponse() {
        @Override
        public void onResponse(final int code) {
            if (code == Constants.REQUEST_SUCCESS) {
                Log.e("BlueDeviceListActivity","数据发送成功！");
                mContext.sendBroadcast(new Intent(BaseVolume.BROADCAST_SEND_RESULT)
                        .putExtra(BaseVolume.BROADCAST_SEND_RESULT,true)
                        .putExtra(BaseVolume.BROADCAST_SEND_LENGTH,sendByte.length));
            } else {
                mContext.sendBroadcast(new Intent(BaseVolume.BROADCAST_SEND_RESULT)
                        .putExtra(BaseVolume.BROADCAST_SEND_RESULT,false)
                        .putExtra(BaseVolume.BROADCAST_SEND_CODE,code)
                        .putExtra(BaseVolume.BROADCAST_SEND_LENGTH,sendByte.length));
                Log.e("BlueDeviceListActivity","数据发送失败！code："+Code.toString(code));
            }
        }
    };

    @Override
    public void onPause() {
        super.onPause();
        ClientManager.getClient().stopSearch();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        ClientManager.getClient().stopSearch();
    }

}
