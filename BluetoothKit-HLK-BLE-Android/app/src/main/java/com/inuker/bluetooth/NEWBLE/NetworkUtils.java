package com.inuker.bluetooth.NEWBLE;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.text.InputFilter;
import android.text.Spanned;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.NumberPicker;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


@SuppressLint("DefaultLocale")
public class NetworkUtils {
	// ʮ����ת��Ϊʮ����ƣ����ΪC8��
    //Integer.toHexString(200);
	// ʮ�����ת��Ϊʮ���ƣ����140��
	//Integer.parseInt("8C",16);

	  /*
     * Convert byte[] to hex string.�������ǿ��Խ�byteת����int��Ȼ������Integer.toHexString(int)��ת����16�����ַ�  
     * @param src byte[] data  
     * @return hex string  
     */     
    public static String bytesToHexString(byte[] src){  
        StringBuilder stringBuilder = new StringBuilder("");  
        if (src == null || src.length <= 0) {  
            return null;  
        }  
        for (int i = 0; i < src.length; i++) {  
            int v = src[i] & 0xFF;  
            String hv = Integer.toHexString(v);  
            if (hv.length() < 2) {  
                stringBuilder.append(0);  
            }  
            stringBuilder.append(hv);  
        }  
        return stringBuilder.toString();  
    }  
    
    /** 
     * Convert hex string to byte[] 
     * @param hexString the hex string 
     * @return byte[] 
     */  
    public static byte[] hexStringToBytes(String hexString) {  
        if (hexString == null || hexString.equals("")) {  
            return null;  
        }  
        hexString = hexString.toUpperCase();  
        int length = hexString.length() / 2;  
        char[] hexChars = hexString.toCharArray();  
        byte[] d = new byte[length];  
        for (int i = 0; i < length; i++) {  
            int pos = i * 2;  
            d[i] = (byte) (charToByte(hexChars[pos]) << 4 | charToByte(hexChars[pos + 1]));  
        }  
        return d;  
    }  
    
    /** 
     * Convert char to byte 
     * @param c char 
     * @return byte 
     */  
    public static byte charToByte(char c) {  
        return (byte) "0123456789ABCDEF".indexOf(c);  
    } 
    
    /**
     * @param Cmd
     * @return  ��ȡ����У��
     */
    public static byte GetXOR(byte[] Cmd)  
    {  
        byte check = (byte)(Cmd[0] ^ Cmd[1]);  
       for (int i = 2; i < Cmd.length; i++)  
       {  
            check = (byte)(check ^ Cmd[i]);  
        }  
        return check;  
    }  
    
    private static String toHexUtil(int n){
        String rt="";
        switch(n){
        case 10:rt+="A";break;
        case 11:rt+="B";break;
        case 12:rt+="C";break;
        case 13:rt+="D";break;
        case 14:rt+="E";break;
        case 15:rt+="F";break;
        default:
            rt+=n;
        }
        return rt;
    }
    
    
    public static String toHex(int n){
        StringBuilder sb=new StringBuilder();
        if(n/16==0){
            return toHexUtil(n);
        }else{
            String t=toHex(n/16);
            int nn=n%16;
            sb.append(t).append(toHexUtil(nn));
        }
        if (sb.length() == 1) {
        	return "0"+sb;
		}
        return sb.toString();
    }
    
    /**
	 * 获取当前时间与星期
	 * @return
	 */
	public static String[] nowData(){  
		String mYear;  
	    String mMonth;  
	    String mDay;  
	    String mHour;
	    String mMinute;
	    String mSecond;
	    String mWay; 
        final Calendar c = Calendar.getInstance();  
        c.setTimeZone(TimeZone.getTimeZone("GMT+8:00"));  
        mYear = String.valueOf(c.get(Calendar.YEAR)); // 获取当前年份  
        mMonth = String.valueOf(c.get(Calendar.MONTH) + 1);// 获取当前月份  
        if (mMonth.length() == 1) 
        	mMonth = "0"+mMonth;
        mDay = String.valueOf(c.get(Calendar.DAY_OF_MONTH));// 获取当前月份的日期号码  
        mHour = String.valueOf(c.get(Calendar.HOUR_OF_DAY));// 小时
        if (mHour.length() == 1) 
        	mHour = "0"+mHour;
        mMinute = String.valueOf(c.get(Calendar.MINUTE));// 分钟
        if (mMinute.length() == 1) 
        	mMinute = "0"+mMinute;
        mSecond = String.valueOf(c.get(Calendar.SECOND));// 秒
        if (mSecond.length() == 1) 
        	mSecond = "0"+mSecond;
        mWay = String.valueOf(c.get(Calendar.DAY_OF_WEEK));  
        if("1".equals(mWay)){  
            mWay ="7";  
        }else if("2".equals(mWay)){  
            mWay ="1";  
        }else if("3".equals(mWay)){  
            mWay ="2";  
        }else if("4".equals(mWay)){  
            mWay ="3";  
        }else if("5".equals(mWay)){  
            mWay ="4";  
        }else if("6".equals(mWay)){  
            mWay ="5";  
        }else if("7".equals(mWay)){  
            mWay ="6";  
        }
        
        return new String[]{mYear,mMonth,mDay,mHour,mMinute,mSecond,mWay};
    }
    
    
    /**
	 * 获取版本号
	 * @return 当前应用的版本号
	 */
	public static String getVersion(Context con) {
	    try {
	        PackageManager manager = con.getPackageManager();
	        PackageInfo info = manager.getPackageInfo(con.getPackageName(), 0);
	        String version = info.versionName;
	        return  version;
	    } catch (Exception e) {
	        e.printStackTrace();
	        return "0.0";
	    }
	}
	
	/**
     * 字符串转ASCII码再转16进制
     * @param str
     * @return
     */
    public static StringBuilder parseAscii(String str){
        StringBuilder sb=new StringBuilder();
        byte[] bs=str.getBytes();
        for(int i=0;i<bs.length;i++)
            sb.append(toHex(bs[i]));
        return sb;
    }
    
    /**
	 * 密码强度
	 * 
	 * @return Z = 字母 S = 数字 T = 特殊字符
	 */
	public static int checkPassword(String passwordStr) {
		String regexZ = "\\d*";
		String regexS = "[a-zA-Z]+";
		String regexT = "\\W+$";
		String regexZT = "\\D*";
		String regexST = "[\\d\\W]*";
		String regexZS = "\\w*";
		String regexZST = "[\\w\\W]*";

		if (passwordStr.matches(regexZ)) {
			return 0;// 弱
		}
		if (passwordStr.matches(regexS)) {
			return 0;// 弱
		}
		if (passwordStr.matches(regexT)) {
			return 0;// 弱
		}
		if (passwordStr.matches(regexZT)) {
			return 1;// 中
		}
		if (passwordStr.matches(regexST)) {
			return 1;// 中
		}
		if (passwordStr.matches(regexZS)) {
			return 1;// 中
		}
		if (passwordStr.matches(regexZST)) {
			return 2;// 强
		}
		return 0;

	}
	
	/**
	  * unicode 转字符串
	 */
	public static String unicode2String(String unicode) {
	  
	     StringBuffer string = new StringBuffer();
	  
	     String[] hex = unicode.split("\\\\u");
	  
	     for (int i = 1; i < hex.length; i++) {
	  
	         // 转换出每一个代码点
	        int data = Integer.parseInt(hex[i], 16);
	  
	         // 追加成string
	         string.append((char) data);
	     }
	  
	     return string.toString();
	 }
	
	/**
	  * 字符串转换unicode
	  */
	 public static String string2Unicode(String string) {
	  
	     StringBuffer unicode = new StringBuffer();
	  
	     for (int i = 0; i < string.length(); i++) {
	  
	         // 取出每一个字符
	        char c = string.charAt(i);
	  
	         // 转换为unicode
	         unicode.append("\\u" + Integer.toHexString(c));
	     }
	  
	     return unicode.toString();
	 }
	 
	 /**
	  * 二进制转16进制
	  * @param bString
	  * @return
	  */
	 public static String binaryString2hexString(String bString)  
	    {  
	        if (bString == null || bString.equals("") || bString.length() % 8 != 0)  
	            return null;  
	        StringBuffer tmp = new StringBuffer();  
	        int iTmp = 0;  
	        for (int i = 0; i < bString.length(); i += 4)  
	        {  
	            iTmp = 0;  
	            for (int j = 0; j < 4; j++)  
	            {  
	                iTmp += Integer.parseInt(bString.substring(i + j, i + j + 1)) << (4 - j - 1);  
	            }  
	            tmp.append(Integer.toHexString(iTmp));  
	        }  
	        
	        String str = tmp.toString();
	        if (str.length() < 2) {
	        	str = "0" + str;
	        }
	        return str;  
	    }
	 
	
	 
	/**
	  * 判断当前日期是星期几
	  * 
	  * @param  pTime     设置的需要判断的时间  //格式如2012-09-08
	  * @return dayForWeek 判断结果
	  * @Exception 发生异常
	  */

	//  String pTime = "2012-03-12";
	 public static String getNowWeek(String pTime) {

	  
	  String Week = "";

	  SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd");
	  Calendar c = Calendar.getInstance();
	  try {

	   c.setTime(format.parse(pTime));

	  } catch (ParseException e) {
	   // TODO Auto-generated catch block
	   e.printStackTrace();
	  }
	  if (c.get(Calendar.DAY_OF_WEEK) == 1) {
	   Week += "7";
	  }
	  if (c.get(Calendar.DAY_OF_WEEK) == 2) {
	   Week += "1";
	  }
	  if (c.get(Calendar.DAY_OF_WEEK) == 3) {
	   Week += "2";
	  }
	  if (c.get(Calendar.DAY_OF_WEEK) == 4) {
	   Week += "3";
	  }
	  if (c.get(Calendar.DAY_OF_WEEK) == 5) {
	   Week += "4";
	  }
	  if (c.get(Calendar.DAY_OF_WEEK) == 6) {
	   Week += "5";
	  }
	  if (c.get(Calendar.DAY_OF_WEEK) == 7) {
	   Week += "6";
	  }

	  return Week;
	 }
	 
	 /**
		 * 设置选择器控件的大小
		 * @param d
		 * @param t
		 */
	 public static void setPickerSize(ViewGroup dp) {
			List<NumberPicker> npList = findNumberPicker(dp);
			for(NumberPicker np:npList){
				resizeNumberPicker(np);
			}
		}
		
		/**
		 * 得到viewGroup里面的numberpicker组件
		 * @param viewGroup
		 * @return
		 */
		private static List<NumberPicker> findNumberPicker(ViewGroup viewGroup){
			List<NumberPicker> npList = new ArrayList<NumberPicker>();
			View child = null;
			if(null != viewGroup){
				for(int i = 0;i<viewGroup.getChildCount();i++){
					child = viewGroup.getChildAt(i);
					if(child instanceof NumberPicker){
						npList.add((NumberPicker)child);
					}
					else if(child instanceof LinearLayout){
						List<NumberPicker> result = findNumberPicker((ViewGroup)child);
						if(result.size()>0){
							return result;
						}
					}
				}
			}
			return npList;
		}
		
		/**
		 * 调整numberpicker大小
		 */
		@SuppressLint("NewApi")
		private static void resizeNumberPicker(NumberPicker np){
			LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(200, LayoutParams.WRAP_CONTENT);
			params.setMargins(0, 0, 0, 0);
			np.setLayoutParams(params);
		}
		
		/**
		  * 修改当前datepicker的字体大小
		  * 
		  * @param 
		  * @return
		  */
		public static void SetNumberPickerTxt(ViewGroup dp) {
			List<NumberPicker> nbList = new ArrayList<NumberPicker>();
			nbList = findNumberPicker(dp);
			if (nbList != null) {
				for (int i = 0; i < nbList.size(); i++) {
					NumberPicker nbTmp = (NumberPicker) nbList.get(i);
					EditText edTxt = findEditText(nbTmp);
					edTxt.setFocusable(false);
					edTxt.setGravity(Gravity.CENTER);
					edTxt.setTextSize(15);
				}
			}

		}
		
	/**
	 * 从当前NumberPicker中查找EditText子控件
	 * 
	 * @param group
	 * @return
	 */
	@SuppressLint("NewApi")
	private static EditText findEditText(NumberPicker np) {
		if (np != null) {
			for (int i = 0; i < np.getChildCount(); i++) {
				View child = np.getChildAt(i);
				if (child instanceof EditText) {
					return (EditText) child;
				}
			}
		}
		return null;
	}
    
	/** ascii码转16进制 */
	 public static String convertStringToHex(String str){

		  char[] chars = str.toCharArray();

		  StringBuffer hex = new StringBuffer();
		  for(int i = 0; i < chars.length; i++){
		    hex.append(Integer.toHexString((int)chars[i]));
		  }

		  return hex.toString();
		  }
	 
	 /** 判断是否为中文 */
	 public static boolean isZh(Context con) {
	        Locale locale = con.getResources().getConfiguration().locale;
	        String language = locale.getLanguage();
	        if (language.endsWith("zh"))
	            return true;
	        else
	            return false;
	    }
	 
	 public static String getMonth(String timeMonth) {
		 int iMonth = Integer.parseInt(timeMonth.trim());
		 if (iMonth == 1) {
			 return "Jan";
		 }
		 else if (iMonth == 2) {
			 return "Feb";
		 }
		 else if (iMonth == 3) {
			 return "Mar";
		 }
		 else if (iMonth == 4) {
			 return "Apr";
		 }
		 else if (iMonth == 5) {
			 return "May";
		 }
		 else if (iMonth == 6) {
			 return "Jun";
		 }
		 else if (iMonth == 7) {
			 return "Jul";
		 }
		 else if (iMonth == 8) {
			 return "Aug";
		 }
		 else if (iMonth == 9) {
			 return "Sep";
		 }
		 else if (iMonth == 10) {
			 return "Oct";
		 }
		 else if (iMonth == 11) {
			 return "Nov";
		 }
		 else {
			 return "Dec";
		 }
		 
	 }
	 
	    public static boolean isContainChinese(String str) {

	        Pattern p = Pattern.compile("[\u4e00-\u9fa5]");
	        if (p == null) {
	        	return false;
	        }
	        else {
	        	Matcher m = p.matcher(str);
	        	if (m.find()) {
	        		return true;
	        	}
	        	return false;
	        }
	        
	    }
	    
	    /** 两个长度为一个字节 */
		 public static String hexString2binaryString(String hexString)  
		    {  
		        if (hexString == null || hexString.length() % 2 != 0)  
		            return null;  
		        String bString = "", tmp;  
		        for (int i = 0; i < hexString.length(); i++)  
		        {  
		            tmp = "0000"  
		                    + Integer.toBinaryString(Integer.parseInt(hexString  
		                            .substring(i, i + 1), 16));  
		            bString += tmp.substring(tmp.length() - 4)+"";  
		        }  
		        return bString;  
		    }  
	    
	    /** 异或得到校验值 */
	    public static String getXORCheck(String strHexData) {
	    	byte[] args = hexStringToBytes(strHexData);
	    	int result = 0;  
	    	 for(int i = 0 ; i < args.length; i++){  
	    		 result=result^args[i];  
	         }  
	    	String strHex = Integer.toHexString(result);
//	    	String strV =String.format("%02d" ,5);
	    	String strV =strHex.length() < 2 ? "0"+strHex : strHex;
	    	strV = strV.substring(strV.length() - 2);
	    	
//	    	Log.e("NetworkUtils", "getXORCheck，异或，校验结："+strV);
	    	return strV;
	    }
	    
	    
	    /** 异或得到校验值 */
	    public static String getXORCheck(int[] args) {
	    	int result = 0;  
	    	 for(int i = 0 ; i < args.length; i++){  
	    		 result=result^args[i];  
	    		 Log.e("NetworkUtils", "getXORCheck，异或结果："+result);
	         }  
	    	String strV =String.format("%02" , Integer.toHexString(result));
	    	return strV;
	    }
	    
	    /** 十六进制转Ascii */
	    public static String convertHexToString(String hex){  
	    	  
	        StringBuilder sb = new StringBuilder();  
	        StringBuilder temp = new StringBuilder();  
	    
	        //49204c6f7665204a617661 split into two characters 49, 20, 4c...  
	        for( int i=0; i<hex.length()-1; i+=2 ){  
	    
	            //grab the hex in pairs  
	            String output = hex.substring(i, (i + 2));  
	            //convert hex to decimal  
	            int decimal = Integer.parseInt(output, 16);  
	            //convert the decimal to character  
	            sb.append((char)decimal);  
	    
	            temp.append(decimal);  
	        }  
	    
	        return sb.toString();  
	        } 

	    /**
         * 限制输入字符长度   中文算3字符，英文算1字符
         */
	    public static InputFilter getTextFilter(final int maxLength) {
	        InputFilter filter = new InputFilter() {
	            @Override
	            public CharSequence filter(CharSequence source, int start, int end, Spanned dest, int dstart, int dend) {
	                // TODO Auto-generated method stub
	                int dindex = 0;
	                int count = 0;
	                while (count <= maxLength && dindex < dest.length()) {
	                    char c = dest.charAt(dindex++);
	                    if (c < 128) {
	                        count = count + 1;
	                    } else {
	                        count = count + 3;
	                    }
	                }
	                if (count > maxLength) {
	                    return dest.subSequence(0, dindex - 1);
	                }
	                int sindex = 0;
	                while (count <= maxLength && sindex < source.length()) {
	                    char c = source.charAt(sindex++);
	                    if (c < 128) {
	                        count = count + 1;
	                    } else {
	                        count = count + 3;
	                    }
	                }
	                if (count > maxLength) {
	                    sindex--;
	                }
	                return source.subSequence(0, sindex);
	            }
	        };
	        
	        return filter;
	    }


	/** 计算校验和 ，返回十六进制校验位 */
	public static String makeCheckSum(String data) {
		int dSum = 0;
		int length = data.length();
		int index = 0;
		// 遍历十六进制，并计算总和
		while (index < length) {
			String s = data.substring(index, index + 2); // 截取2位字符
			dSum += Integer.parseInt(s, 16); // 十六进制转成十进制 , 并计算十进制的总和
			index = index + 2;
		}

		int mod = dSum % 256; // 用256取余，十六进制最大是FF，FF的十进制是255
		String checkSumHex = Integer.toHexString(mod); // 余数转成十六进制
		length = checkSumHex.length();
		if (length < 2) {
			checkSumHex = "0" + checkSumHex;  // 校验位不足两位的，在前面补0
		}
		return checkSumHex;
	}
	
}
