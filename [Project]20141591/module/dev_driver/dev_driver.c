#include "dev_driver.h"
#define SET_OPTION _IOW(MAJOR_NUM,0,unsigned int) //Encode SET_OPTION by _IOW MACRO
#define COMMAND _IO(MAJOR_NUM,1) //Encode COMMAND by _IO MACRO
/*
 * to match
 * my file operation with original
 */


/*
 * wait queue For sleep thread program
 * when call write (in wait_queue)
 * and alarm time expired, will wake up
 */
wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write); 

// matching file operation function
static struct file_operations dev_fops =
{
	.open = dev_open,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
	.release = dev_release,
};
/*
 * device open
 */
static int dev_open(struct inode *inode, struct file * file){
    int ret;
	int irq;

	printk(KERN_ALERT "Open Module\n");
    init_timer(&timer);
	return 0;
}
/*
 * device realease
 */
static int dev_release(struct inode * inode, struct file * file){
    printk(KERN_ALERT "Release Module\n");

    return 0;
}
static void dev_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos){
    printk("Sleep On\n");
    //thread program will be slept(in wait queue)
    interruptible_sleep_on(&wq_write);
}
/*
 * Alram Timer function(Using project3)
 */
void stopwatch_function(unsigned long timeout){
    fnd_execute();
    //add timer every sec
    timer.expires = get_jiffies_64() + HZ;
    timer.function = stopwatch_function;
    add_timer(&timer);
    if(fnd_min == minute){
        del_timer(&timer);
        outw(0,(unsigned int)iom_fpga_fnd_addr);
		__wake_up(&wq_write,1,1,NULL);
		fnd_min =0;
		fnd_sec = 0;
    }

}

/*
 * write FND Device function
 */ 
void fnd_execute(void){
    unsigned short int fnd_value= 0;
    //stop watch second +1
    fnd_sec++;
    if(fnd_sec == 60){
        fnd_min++;
        fnd_sec = 0;
    }
    if(fnd_min == 60)
        fnd_min = 0;
    fnd_value = fnd_value | ((fnd_min/10) << 12);
    fnd_value = fnd_value | ((fnd_min%10) << 8);
    fnd_value = fnd_value | ((fnd_sec/10) << 4);
    fnd_value = fnd_value | ((fnd_sec%10));

    outw(fnd_value,(unsigned int)iom_fpga_fnd_addr);
    return;
}
/*
 * Alram Timer will start here
 * receive time setting information from jni
 */
long dev_ioctl(struct file * file, unsigned int cmd, unsigned long arg){
    unsigned int byte_stream;
    
    //execute by COMMAND
    switch(cmd){
        //first ioctl (SET_OPTION)
        case SET_OPTION:
            //Receive Argument(byte_stream in jni)
            if (copy_from_user(&byte_stream,(unsigned int*)arg,sizeof(unsigned int))){
                return -1;
            }
            //Setting input time
            minute = byte_stream & 0x000000FF;
            break;
        //SECOND IOCTL (COMMAND)
        case COMMAND:
            //setting timer device environment & add_timer
            timer.expires = get_jiffies_64() + HZ;
            timer.function = stopwatch_function;
            add_timer(&timer);
            break;
        default:
            break;
    }
    return 0;
}

/*
 * character device driver register
 * Using Assignment #7 code  
 */ 
static int sw_register_cdev(void)
{
	int error;
	stopwatch_num = MKDEV(MAJOR_NUM, 0); // return dev_t using major num,minor num
	error = register_chrdev_region(stopwatch_num,1,DEVICE_NAME); //register character device driver
	if(error<0) {
		printk(KERN_WARNING "dev: can't get major %d\n", MAJOR_NUM);
		return 0;
	}
	printk(KERN_ALERT "major number = %d\n",MAJOR_NUM);
	cdev_init(&stopwatch_cdev, &dev_fops); //init character device driver connect file_operation
	stopwatch_cdev.owner = THIS_MODULE;
	stopwatch_cdev.ops = &dev_fops;
	error = cdev_add(&stopwatch_cdev, stopwatch_num, 1); //register in kernel
	if(error)
	{
		printk(KERN_NOTICE "dev Register Error %d\n", error);
	}
	return 0;
}
/*
 * When insmod stopwatch.ko
 * register & init module in kernel
 */ 
static int __init sw_init(void) {
	int result;
	if((result = sw_register_cdev()) < 0 )
		return result;
	printk(KERN_ALERT "Init Module Success \n");
	printk(KERN_ALERT "Device : /dev/dev_driver, Major Num : 242 \n");
    // set virtual fpga address using physical address
    // for writing device
    iom_fpga_fnd_addr = ioremap(FND_PHY_ADDRESS, 0x04);
	return 0;
}
/*
 * When rmmod stopwatch.ko
 * unregister & delete timer 
 */ 
static void __exit sw_exit(void) {
	cdev_del(&stopwatch_cdev);
	unregister_chrdev_region(stopwatch_num, 1);
    iounmap(iom_fpga_fnd_addr);
    del_timer_sync(&timer);
    //del_timer_sync(&e_timer);
	printk(KERN_ALERT "Remove Module Success \n");
}

module_init(sw_init);
module_exit(sw_exit);
	MODULE_LICENSE("GPL");
