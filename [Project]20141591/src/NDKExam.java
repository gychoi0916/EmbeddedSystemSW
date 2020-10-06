package org.example.ndk;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class NDKExam extends Activity {
	//public native int add(int x, int y);
	//public native void testString(String str);
	public native int devopen();
	public native void devioctl(int fd,int minute);
	public native void devclose(int fd);
	public native void devwrite(int fd, char [] buf);
	public native int buzopen();
	public native void buzwrite(int fd);
	public native void buzclose(int fd);
	char [] buf = {0,0};
	int flag;
	/*
	 * Thread Handler
	 * to use TextView Setting
	 */
	Handler mHandler = new Handler(){
		public void handleMessage(Message msg){
			if(msg.what ==0){
				tv.setText("Wake Up!!");
			}
		}
	};
	/*
	 * Thread
	 * Using dev_write
	 * When Alarm driver executing, this Thread will sleep
	 * And, wake_up later setting time, then buzzer driver will start
	 */
	class BackThread extends Thread{
		int mBackValue = 0;
		Handler sHandler;
		BackThread(Handler handler){
			sHandler = handler;
		}
		public void run(){
			//tv.setText("Sleep During "+ minute+ "minutes..");
			try{
				devwrite(fd,buf);
				Message msg = Message.obtain();
				msg.what=0;
				sHandler.sendMessage(msg);
				buzfd = buzopen();
	        	buzwrite(buzfd);
	        	buzclose(buzfd);
	        	
	        	try{
	        		total_sleep2 += minute;
	        		BufferedWriter bw = new BufferedWriter(new FileWriter(getFilesDir().toString()+"/minutes.txt",false));
	        		bw.write(Integer.toString(total_sleep2));
	        		bw.close();
	        	}catch(IOException e){
	        		e.printStackTrace();
	        	}
			}catch(Exception e){
				;
			}
			//tv.setText("Wake Up!!");
		}
	}
	TextView tv;
	EditText data;
	Button btn,btn2;
	OnClickListener ltn;
	OnClickListener ltn2;
	BackThread mThread;
	int fd;
	int buzfd;
	int minute;
	int total_sleep,total_sleep2;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        tv = (TextView)findViewById(R.id.textView1);
        data = (EditText)findViewById(R.id.editText1);
        btn = (Button)findViewById(R.id.button1);
        btn2 = (Button)findViewById(R.id.button2);
        tv.setText("00minutes later Alarms");       
        
        try{
    		BufferedReader br = new BufferedReader(new FileReader(getFilesDir().toString()+"/minutes.txt"));
    		String readStr ="";
    		String str = null;
    		str = br.readLine();
    		readStr += str;
    		total_sleep2 = Integer.parseInt(readStr);
    		br.close();
    	}catch(FileNotFoundException e){
    		total_sleep2 =0;
    		e.printStackTrace();
    	}catch(IOException e){
    		e.printStackTrace();
    	}
		/*
		 * Input Alarm time then, click "Setting" button
		 * then alarm device open, and ioctl communication with input minute
		 */
        ltn = new OnClickListener(){
        	public void onClick(View v){
        		String tmp = data.getText().toString();
        		minute = Integer.parseInt(tmp); // receive editText content
        		fd = devopen();
        		devioctl(fd,minute); // ioctl communication
        		mThread = new BackThread(mHandler);
                mThread.setDaemon(true);
        		mThread.start(); //thread run
        		tv.setText("Sleep During "+ minute+ "minutes..");
        		devclose(fd);
        	}
        };

		/*
		 * If click this button,
		 * User will check How long alarm using
		 */
        btn.setOnClickListener(ltn);
        ltn2 = new OnClickListener(){
        	public void onClick(View v){
        		tv.setText("You sleep today " + total_sleep+" minutes!"+"total "+ total_sleep2+"!");
        	}
        };
        btn2.setOnClickListener(ltn2);
        System.loadLibrary("ndk-exam");
        
        
        
        //tv.setText("The multiply of " + x + " and " + y + " is " + z);
        //setContentView(tv);
	
       
        //testString("test");
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
