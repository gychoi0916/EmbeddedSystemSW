#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/ioctl.h>
#include "android/log.h"

#define DEV_DRIVER "/dev/dev_driver"
#define DOT_DRIVER "/dev/fpga_dot"
#define FND_DRIVER "/dev/fpga_fnd"
#define SWITCH_DRIVER "/dev/fpga_push_switch"
#define BUZ_DRIVER "/dev/fpga_buzzer"
#define MAX_BUTTON 9
#define MAJOR_NUM 242
#define SET_OPTION _IOW(MAJOR_NUM,0,unsigned int)
#define COMMAND _IO(MAJOR_NUM,1)
#define LOG_TAG "NATIVE"
#define LOGV(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


unsigned int bytestream = 0;
static int total_sleep = 0;
/*
 * Alarm driver Open
 */
jint JNICALL Java_org_example_ndk_NDKExam_devopen(JNIEnv * env,jobject this){
    int fd = open(DEV_DRIVER,O_RDWR);
    if (fd < 0){
        LOGV("/dev/dev_driver open Error");
        exit(-1);
    }

    return fd;
}
/*
 * dev_driver write
 * Thread will call this function
 * and thread will sleep until setting time.
 * when it wake up, java field variable update
 */
void JNICALL Java_org_example_ndk_NDKExam_devwrite(JNIEnv * env,jobject this,jint fd,jcharArray buf){
	LOGV("check !");
	write(fd,buf,2);
	LOGV("check 2 !");
	jclass thisClass = (*env)->GetObjectClass(env,this);
	jfieldID fidnumber = (*env)->GetFieldID(env,thisClass,"total_sleep","I");
	jint number = (*env) -> GetIntField(env,this,fidnumber);
	number = total_sleep;
	(*env)->SetIntField(env,this,fidnumber,number);
}

/*
 * ioctl communication with setting time(minute)
 * miute will pass ioctl by bytestream
 * using 2nd Project
 */
void JNICALL Java_org_example_ndk_NDKExam_devioctl(JNIEnv * env,jobject this,jint fd,jint minute){
    bytestream = 0;
    bytestream = bytestream | minute;
    total_sleep += minute;
    ioctl(fd,SET_OPTION,&bytestream);
    ioctl(fd,COMMAND);
}
//device close
void JNICALL Java_org_example_ndk_NDKExam_devclose(JNIEnv* env,jobject this,jint fd){
    close(fd);
}
/*
 * Buzzer driver open
 */
jint JNICALL Java_org_example_ndk_NDKExam_buzopen(JNIEnv* env, jobject this){
	int fd = open(BUZ_DRIVER,O_RDWR);
	if (fd<0){
		LOGV("Buzzer driver open error!");
		exit(-1);
	}
	else{
		LOGV("Buzzer driver open correct!");
	}
	return fd;
}
/*
 * buzzer driver write
 * will beep when time exipre
 */
void JNICALL Java_org_example_ndk_NDKExam_buzwrite(JNIEnv* env, jobject this,jint fd){
	int retval;
	int state =0;
	int data;
	LOGV("Buzzer write correct!");
	while(retval != 2){
		if(state!=0){
			state = 0;
			data =1;
			retval = write(fd,&data,1);
			if(retval<0){
				LOGV("Buzzer write error!");
				exit(-1);
			}
		}else{
			state =1;
			data =0;
			retval = write(fd,&data,1);
			if(retval <0){
				LOGV("Buzzer write error!");
				exit(-1);
			}
		}
		LOGV("check ret : ");
		sleep(1);
	}
}
//buzzer driver close
void JNICALL Java_org_example_ndk_NDKExam_buzclose(JNIEnv* env, jobject this,jint fd){
	close(fd);
}
