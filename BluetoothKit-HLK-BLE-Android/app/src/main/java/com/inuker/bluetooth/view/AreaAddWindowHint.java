package com.inuker.bluetooth.view;


import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.inuker.bluetooth.R;


public class AreaAddWindowHint extends Dialog implements View.OnClickListener {
	private Context context;
	private Button confirmBtn;
	private Button cancelBtn;
	private TextView tvContent;
	private boolean isShowTost;
	
	private TextView titleTv;
	private String period = "";
	private PeriodListener listener;
	private String defaultName = "",title;
	private String strConfirm = "";
	public AreaAddWindowHint(Context context) {
		super(context);
		this.context = context;
	}

	public AreaAddWindowHint(Context context, int theme, String titleName, PeriodListener listener, String defaultName) {
		super(context, theme);
		this.context = context;
		this.listener = listener;
		this.defaultName = defaultName;
		this.title = titleName;
	}
	
	public AreaAddWindowHint(Context context, int theme, String titleName, PeriodListener listener, String defaultName, boolean isTost) {
		super(context, theme);
		this.context = context;
		this.listener = listener;
		this.defaultName = defaultName;
		this.title = titleName;
		this.isShowTost = isTost;
	}

	/** 更新显示内容 */
	public void updateContent(String strNewContent) {
		this.defaultName = strNewContent;
		tvContent.setText(defaultName);
	}
	
	/****
	 * 
	 * @author mqw
	 *
	 */
	public interface PeriodListener {
		public void refreshListener(String string);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.setContentView(R.layout.window_area_hint);
		confirmBtn = (Button) findViewById(R.id.confirm_btn);
		cancelBtn = (Button) findViewById(R.id.cancel_btn);
		tvContent = (TextView) findViewById(R.id.areaName);
		titleTv = (TextView) findViewById(R.id.dialog_title);
		titleTv.setText(title);
		
		if (isShowTost) {
			findViewById(R.id.view1).setVisibility(View.GONE);
			cancelBtn.setVisibility(View.GONE);
		}
		
		
		confirmBtn.setOnClickListener(this);
		cancelBtn.setOnClickListener(this);
		tvContent.setText(defaultName);
		
		if (!strConfirm.equals("")) {
			confirmBtn.setText(strConfirm);
		}
		
		
	}
	
	public void setConfirmName(String strConfirm) {
		this.strConfirm = strConfirm;
	}
		 
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		int id = v.getId();
		switch (id) {
		case R.id.cancel_btn:
			dismiss();
			break;
		case R.id.confirm_btn: 
				dismiss();
				listener.refreshListener(period);
			break;

		default:
			break;
		}
	}
}