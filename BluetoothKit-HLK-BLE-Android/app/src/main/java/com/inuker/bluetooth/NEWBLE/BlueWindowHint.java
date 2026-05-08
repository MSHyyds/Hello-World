package com.inuker.bluetooth.NEWBLE;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.inuker.bluetooth.R;


public class BlueWindowHint extends  Dialog implements View.OnClickListener {
	private Context context;
	private Button confirmBtn;
	private Button cancelBtn;
	private TextView tvContent;
	private boolean isShowTost;
	
	private TextView titleTv;
	private String period = "";
	private PeriodListener listener;
	private String defaultName = "",title,strLeft="取消",strRight="确定";
	public BlueWindowHint(Context context) {
		super(context);
		this.context = context;
	}

	public BlueWindowHint(Context context, int theme, String titleName, PeriodListener listener, String defaultName, String strLeft, String strRight) {
		super(context, theme);
		this.context = context;
		this.listener = listener;
		this.defaultName = defaultName;
		this.title = titleName;
		this.strLeft = strLeft;
		this.strRight = strRight;
	}
	
	public BlueWindowHint(Context context, int theme, String titleName, PeriodListener listener, String defaultName, boolean isTost) {
		super(context, theme);
		this.context = context;
		this.listener = listener;
		this.defaultName = defaultName;
		this.title = titleName;
		this.isShowTost = isTost;
	}

	
	/****
	 * 
	 * @author mqw
	 *
	 */
	public interface PeriodListener {
		public void confirmListener();
		public void cancelListener();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.setContentView(R.layout.window_ble_hint);
		confirmBtn = (Button) findViewById(R.id.confirm_btn);
		cancelBtn = (Button) findViewById(R.id.cancel_btn);
		tvContent = (TextView) findViewById(R.id.areaName);
		titleTv = (TextView) findViewById(R.id.dialog_title);
		titleTv.setText(title);
		setCancelable(false);
		
		if (isShowTost) {
			findViewById(R.id.view1).setVisibility(View.GONE);
			cancelBtn.setVisibility(View.GONE);
		}

		cancelBtn.setText(strLeft);
		confirmBtn.setText(strRight);
		
		confirmBtn.setOnClickListener(this);
		cancelBtn.setOnClickListener(this);
		tvContent.setText(defaultName);
		
		
	}
	
	/** 更新显示内容 */
	public void updateContent(String strNewContent) {
		this.defaultName = strNewContent;
		tvContent.setText(defaultName);
	}
		 
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		int id = v.getId();
		switch (id) {
		case R.id.cancel_btn:
			dismiss();
			listener.cancelListener();
			break;
		case R.id.confirm_btn:
				dismiss();
				listener.confirmListener();
			break;

		default:
			break;
		}
	}
}