package com.inuker.bluetooth;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.inuker.bluetooth.library.connect.listener.BleConnectStatusListener;
import com.inuker.bluetooth.library.connect.options.BleConnectOptions;
import com.inuker.bluetooth.library.connect.response.BleConnectResponse;
import com.inuker.bluetooth.library.model.BleGattCharacter;
import com.inuker.bluetooth.library.model.BleGattProfile;
import com.inuker.bluetooth.library.model.BleGattService;
import com.inuker.bluetooth.library.search.SearchResult;
import com.inuker.bluetooth.library.utils.BluetoothLog;
import com.inuker.bluetooth.library.utils.BluetoothUtils;

import static com.inuker.bluetooth.library.Constants.*;

import java.util.List;
import java.util.UUID;

/**
 * Created by dingjikerbo on 2016/9/2.
 */
public class DeviceDetailActivity extends Activity {
    private TextView mTvTitle;
    private ProgressBar mPbar;

    private ListView mListView;
    private DeviceDetailAdapter mAdapter;

    private SearchResult mResult;

    private BluetoothDevice mDevice;

    private boolean mConnected;

    private final String UUID_BY_SERVER = "0000fff0-0000-1000-8000-00805f9b34fb";
    private final String UUID_BY_READ = "0000fff1-0000-1000-8000-00805f9b34fb";
    private final String UUID_BY_WRITE = "0000fff2-0000-1000-8000-00805f9b34fb";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.device_detail_activity);

        Intent intent = getIntent();
        String mac = intent.getStringExtra("mac");
        mResult = intent.getParcelableExtra("device");

        mDevice = BluetoothUtils.getRemoteDevice(mac);

        mTvTitle = (TextView) findViewById(R.id.title);
        mTvTitle.setText(mDevice.getAddress());

        mPbar = (ProgressBar) findViewById(R.id.pbar);

        mListView = (ListView) findViewById(R.id.listview);
        mAdapter = new DeviceDetailAdapter(this, mDevice);
        mListView.setAdapter(mAdapter);

        mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (!mConnected) {
                    return;
                }
                DetailItem item = (DetailItem) mAdapter.getItem(position);

                if (item.type == DetailItem.TYPE_SERVICE) {
                    // 透传的服务
                    if (item.uuid.toString().equalsIgnoreCase(UUID_BY_SERVER)) {
                        List<BleGattService> services = nowBleGattProfile.getServices();

                        for (BleGattService service : services) {
                            if (service.getUUID().toString().equalsIgnoreCase(UUID_BY_SERVER)) {
                                List<BleGattCharacter> characters = service.getCharacters();
                                UUID uuidService = item.uuid;
                                UUID uuidRead = null;
                                UUID uuidWrite = null;
                                for (BleGattCharacter character : characters) {
                                    if (character.getUuid().toString().equalsIgnoreCase(UUID_BY_READ))
                                        uuidRead = character.getUuid();
                                    else
                                        uuidWrite = character.getUuid();
                                }
                                Intent intent = new Intent(DeviceDetailActivity.this, CharacterActivity.class);
                                intent.putExtra("mac", mDevice.getAddress());
                                intent.putExtra("service", uuidService);
                                intent.putExtra("characterWrite", uuidWrite);
                                intent.putExtra("character", uuidRead);
                                startActivity(intent);
                                break;
                            }
                        }


                    }


                }


//                if (item.type == DetailItem.TYPE_CHARACTER) {
//                    BluetoothLog.v(String.format("click service = %s, character = %s", item.service, item.uuid));
//                    startCharacterActivity(item.service, item.uuid);
//                }
            }
        });

        ClientManager.getClient().registerConnectStatusListener(mDevice.getAddress(), mConnectStatusListener);

        connectDeviceIfNeeded();


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

    private final BleConnectStatusListener mConnectStatusListener = new BleConnectStatusListener() {
        @Override
        public void onConnectStatusChanged(String mac, int status) {
            BluetoothLog.v(String.format("DeviceDetailActivity onConnectStatusChanged %d in %s",
                    status, Thread.currentThread().getName()));

            mConnected = (status == STATUS_CONNECTED);
            connectDeviceIfNeeded();
        }
    };

    private void startCharacterActivity(UUID service, UUID character) {
        Intent intent = new Intent(this, CharacterActivity.class);
//        Intent intent = new Intent(this, TestDemoActivity.class);
        intent.putExtra("mac", mDevice.getAddress());
        intent.putExtra("service", service);
        intent.putExtra("character", character);
        startActivity(intent);
    }

    private void connectDevice() {
        mTvTitle.setText(String.format("%s%s", getString(R.string.connecting), mDevice.getAddress()));
        mPbar.setVisibility(View.VISIBLE);
        mListView.setVisibility(View.GONE);

        BleConnectOptions options = new BleConnectOptions.Builder()
                .setConnectRetry(3)
                .setConnectTimeout(20000)
                .setServiceDiscoverRetry(3)
                .setServiceDiscoverTimeout(10000)
                .build();

        ClientManager.getClient().connect(mDevice.getAddress(), options, new BleConnectResponse() {
            @Override
            public void onResponse(int code, BleGattProfile profile) {
                BluetoothLog.v(String.format("profile:\n%s", profile));
                mTvTitle.setText(String.format("%s", mDevice.getAddress()));
                mPbar.setVisibility(View.GONE);
                mListView.setVisibility(View.VISIBLE);

                if (code == REQUEST_SUCCESS) {
                    nowBleGattProfile = profile;
                    mAdapter.setGattProfile(profile);
                }
            }
        });
    }

    private BleGattProfile nowBleGattProfile;

    private void connectDeviceIfNeeded() {
        if (!mConnected) {
            connectDevice();
        }
    }

    @Override
    protected void onDestroy() {
        ClientManager.getClient().disconnect(mDevice.getAddress());
        ClientManager.getClient().unregisterConnectStatusListener(mDevice.getAddress(), mConnectStatusListener);
        super.onDestroy();
    }
}
