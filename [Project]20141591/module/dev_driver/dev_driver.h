#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>

#define DEVICE_NAME "dev_driver"
#define MAJOR_NUM 242 //Major number
/*
 * FND PHYSICAL ADDRESS
 */
#define FND_PHY_ADDRESS 0x08000004

static struct cdev stopwatch_cdev;
struct timer_list timer;
//DEVICE EXECUTE OPTION VARIABLE
// unsigned int fnd_idx,fnd_val,t_cnt,t_interval,counter=0;
static dev_t stopwatch_num;
/*
 * Virtual device address
 * For control device
 */
static unsigned char* iom_fpga_fnd_addr;

//fnd_number information var
unsigned short int fnd_min = 0;
unsigned short int fnd_sec = 0;

//for store left time when stopwatch paused
unsigned long paused_time = 0;
unsigned char minute;

/*
 * function definition
 */
static int dev_open(struct inode *inode, struct file * file);
static void dev_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static long dev_ioctl(struct file *file,unsigned int cmd, unsigned long arg);
static int dev_release(struct inode * inode, struct file * file);
void stopwatch_function(unsigned long timeout);
void end_function(unsigned long timeout);
void fnd_execute(void);
static int sw_register_cdev(void);
static int __init sw_init(void);
static void __exit sw_exit(void);
