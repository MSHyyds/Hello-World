package com.inuker.bluetooth.NEWBLE;


public class BaseVolume {

	public static final String BROADCAST_RECVPIPE = "BROADCAST_RECVPIPE";
	public static final String BROADCAST_SEND_TIMEOUT = "BROADCAST_SEND_TIMEOUT";
	public static final String BROADCAST_SEND_RESULT = "BROADCAST_SEND_RESULT";
	public static final String BROADCAST_SEND_LENGTH = "BROADCAST_SEND_LENGTH";
	public static final String BROADCAST_SEND_CODE = "BROADCAST_SEND_CODE";
	/** 包头 */
	public static final String COMMAND_HEAD = "a5aaac";
	/** 包尾 */
	public static final String COMMAND_END = "c5ccca";

	
	/** 设备开关机（左上角键） */
	public static final String COMMAND_DEVICE_SWITCH = "68FFFFFF01011178";
	/** 设备返回键（右上角键） */
	public static final String COMMAND_DEVICE_BACK = "68FFFFFF01012289";
	/** 设备确认键（下部分中间键） */
	public static final String COMMAND_DEVICE_CENTRE = "68FFFFFF0101339A";
	/** 设备左键（下部分左边键） */
	public static final String COMMAND_DEVICE_LEFT = "68FFFFFF010144AB";
	/** 设备右键（下部分右边键） */
	public static final String COMMAND_DEVICE_RIGHT = "68FFFFFF010155BC";
	/** 设备上键（下部分上边键） */
	public static final String COMMAND_DEVICE_TOP = "68FFFFFF010166CD";
	/** 设备下键（下部分下边键） */
	public static final String COMMAND_DEVICE_DOWN= "68FFFFFF010177DE";


	
	
	
	
}
