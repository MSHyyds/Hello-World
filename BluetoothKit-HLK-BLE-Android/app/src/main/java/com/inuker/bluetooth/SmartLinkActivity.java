package com.inuker.bluetooth;


import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.text.Html;
import android.text.method.HideReturnsTransformationMethod;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.inuker.bluetooth.NEWBLE.BaseVolume;
import com.inuker.bluetooth.NEWBLE.MainBLEActivity;
import com.inuker.bluetooth.library.Code;
import com.inuker.bluetooth.library.connect.response.BleNotifyResponse;
import com.inuker.bluetooth.library.connect.response.BleWriteResponse;
import com.inuker.bluetooth.library.utils.ByteUtils;
import com.inuker.bluetooth.view.AreaAddWindowHint;
import com.inuker.bluetooth.view.LoadingDialog;

import java.util.UUID;

import static com.inuker.bluetooth.library.Constants.REQUEST_SUCCESS;


public class SmartLinkActivity extends AppCompatActivity {

	private TextView m_tvChangeWifi;
	private TextView m_btnConfig;
	private EditText m_tvSSID;
	private EditText m_edPwd;
	private String SSID,PWD,SendData;
	private LinearLayout m_llParent;
	private boolean showPwd = false;
	private LoadingDialog loadingDialog = null;
	private ImageView m_imgPwd;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_airkiss);

		loadingDialog = new LoadingDialog(this, R.style.LoadingDialogStyle);
		loadingDialog.setCancelable(false);
		
        m_imgPwd = (ImageView) findViewById(R.id.imgPwdState);
		m_tvSSID = (EditText) findViewById(R.id.tvSSID);
		m_edPwd = (EditText) findViewById(R.id.edPwd);
		m_btnConfig = (TextView) findViewById(R.id.tvConfig);
		m_llParent = (LinearLayout) findViewById(R.id.llParent);
		
		WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
		WifiInfo wifiInfo = wifiManager.getConnectionInfo();
		String strSSID = wifiInfo.getSSID();
		Log.d("wifiInfo", wifiInfo.toString());
		String ssid = "";
		ssid = getWIFISSIDNew(this);
		m_tvSSID.setText(ssid);
		
		m_btnConfig.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				SSID = m_tvSSID.getText().toString();
				PWD = m_edPwd.getText().toString();

				SendData = "!%!%:ssid:"+SSID+",pwd:"+PWD+",";
				if (PWD.equals("")) {
					hintRegister("系统提示","密码为空，是否继续配置？");
				}
				else {
					Log.e("AKeyConfigActivity","开始配置！");
					loadingDialog.show();
					MainBLEActivity.getInstance().startSendData(SendData.getBytes());
				}

			}
		});

		
		findViewById(R.id.llParent).setOnClickListener(new OnClickListener() {
			@SuppressLint("NewApi")
			public void onClick(View v) {
				InputMethodManager imm = (InputMethodManager)SmartLinkActivity.this.getSystemService(Context.INPUT_METHOD_SERVICE);
				imm.hideSoftInputFromWindow(v.getWindowToken(), 0);  
			}
		});

		findViewById(R.id.imgPwdState).setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				if (!showPwd) {
					m_edPwd.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
					showPwd = true;
					m_imgPwd.setImageResource(R.drawable.img_pwd_show);
				}
				else {
					m_edPwd.setTransformationMethod(PasswordTransformationMethod.getInstance());
					showPwd = false;
					m_imgPwd.setImageResource(R.drawable.img_pwd_hide);
				}
			}
		});
		
		m_tvChangeWifi = (TextView) findViewById(R.id.tvChangeWifi);
		m_tvChangeWifi.setText(Html.fromHtml("<u>"+"更 换 wifi"+"</u>"));
		m_tvChangeWifi.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				//直接进入手机中的wifi网络设置界面
				startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));
			}
		});
		
		// 添加网络监听
		IntentFilter mFilter = new IntentFilter();
		mFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
		mFilter.addAction(BaseVolume.BROADCAST_RECVPIPE);
		mFilter.addAction(BaseVolume.BROADCAST_SEND_TIMEOUT);
		mFilter.addAction(BaseVolume.BROADCAST_SEND_RESULT);
		registerReceiver(myNetReceiver, mFilter);


	}

	private void hintRegister(String title, String content) {
		AreaAddWindowHint pwdDialog = new AreaAddWindowHint(SmartLinkActivity.this,
				R.style.dialog_style,title,
				new AreaAddWindowHint.PeriodListener() {
					public void refreshListener(String string) {
						Log.e("AKeyConfigActivity","开始配置！");
						loadingDialog.show();
						MainBLEActivity.getInstance().startSendData(SendData.getBytes());
					}
				},content);
		pwdDialog.show();
	}

	/** 配网信息发送成功，设备需要重启 */
	private void hintRegisterSuccess(String title, String content) {
		AreaAddWindowHint pwdDialog = new AreaAddWindowHint(SmartLinkActivity.this,
				R.style.dialog_style,title,
				new AreaAddWindowHint.PeriodListener() {
					public void refreshListener(String string) {
						finish();
						// 发送广播，让其他界面自己关闭。
						sendBroadcast(new Intent("AutoDisconnectDevice"));
					}
				},content,true);
		pwdDialog.setCanceledOnTouchOutside(false);
		pwdDialog.show();
	}
	
	


	protected void onDestroy() {
		super.onDestroy();
		unregisterReceiver(myNetReceiver);
	}
	
	/****
	 * 网络监听
	 */
	private BroadcastReceiver myNetReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
				ConnectivityManager mConnectivityManager = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
				NetworkInfo netInfo = mConnectivityManager
						.getActiveNetworkInfo();
				if (netInfo != null && netInfo.isAvailable()) {
					// wifi网络
					if (netInfo.getType() == ConnectivityManager.TYPE_WIFI) {
						WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
						WifiInfo wifiInfo = wifiManager.getConnectionInfo();
						String ssid = "";
						ssid = getWIFISSIDNew(SmartLinkActivity.this);
						Log.e("网络监测", "当前网络为wifi，ssid="+ssid);
						m_tvSSID.setText(ssid);
						
					} 
					// 2G网络
					else if (netInfo.getType() == ConnectivityManager.TYPE_ETHERNET) {
						Log.e("网络监测", "当前网络2G");
					} 
					// 3G网络
					else if (netInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
						Log.e("网络监测", "当前网络3G");
					}
				} else {
					
				}
			}
			// 接收数据
			else if (intent.getAction().equalsIgnoreCase(BaseVolume.BROADCAST_RECVPIPE)) {
				byte[] value = intent.getByteArrayExtra(BaseVolume.BROADCAST_RECVPIPE);
				String strData = new String(value);
				Log.e("mNotifyRsp", "接收数据："+strData);
				// 配网成功
				if (strData.indexOf("BLE smtcn:ok") >= 0) {
					loadingDialog.dismiss();
					hintRegisterSuccess("系统提示","配网信息发送成功，设备需要重启！");
				}
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
				} else {
					CommonUtils.toast("Send Error:"+ Code.toString(iCode));
					loadingDialog.dismiss();
				}
			}
			else if (intent.getAction().equalsIgnoreCase("AutoDisconnectDevice")) {
				finish();
			}
		}
	};

    /**
     * 获取SSID
     * @param activity 上下文
     * @return  WIFI 的SSID
     */
    public String getWIFISSIDNew(Activity activity) {
        String ssid="unknown id";

        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.O||Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {

            WifiManager mWifiManager = (WifiManager) activity.getApplicationContext().getSystemService(Context.WIFI_SERVICE);

            assert mWifiManager != null;
            WifiInfo info = mWifiManager.getConnectionInfo();

            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
                return info.getSSID();
            } else {
                return info.getSSID().replace("\"", "");
            }
        } else if (Build.VERSION.SDK_INT==Build.VERSION_CODES.O_MR1){

            ConnectivityManager connManager = (ConnectivityManager) activity.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
            assert connManager != null;
            NetworkInfo networkInfo = connManager.getActiveNetworkInfo();
            if (networkInfo.isConnected()) {
                if (networkInfo.getExtraInfo()!=null){
                    return networkInfo.getExtraInfo().replace("\"","");
                }
            }
        }
        return ssid;
    }





}
