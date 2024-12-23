#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>          
#include <linux/errno.h>       
#include <linux/types.h>       
#include <linux/fcntl.h>       

#include <linux/gpio.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include "ioctl.h"
#define DEBUG 1

#define   ECKIM_DEV_NAME            "ecKim"
#define   ECKIM_DEV_MAJOR            230      

#define OFF 0
#define ON 1
#define GPIOCNT 8

static int openFlag = 0;
static int ledVal = 0;
static int gpioLedInit(void);
static void gpioLedSet(long val);
static void gpioLedFree(void);
static int gpioKeyInit(void);
static void gpioKeyFree(void);
static int gpioLed[GPIOCNT] = {518,519,520,521,522,523,524,525};
static int gpioKey[GPIOCNT] = {528,529,530,531,532,533,534,535};
/////////////////////////////////////////
typedef struct {
	int keyNum;
	int irqKey[GPIOCNT];
} keyDataStruct;
static int irqKeyInit(keyDataStruct* pkeyData);
static void irqKeyFree(keyDataStruct* pKeyData);
static DEFINE_MUTEX(keyMutex);
static DECLARE_WAIT_QUEUE_HEAD(waitQueueRead);

/////////////////////////////////////////
timerVal_data driver_info;
struct timer_list timerLed;
static int timerVal = 100;
void kerneltimer_registertimer(unsigned long timeover);
void kerneltimer_func(struct timer_list* t);
/////////////////////////////////////////




void kerneltimer_registertimer(unsigned long timeover)
{
    if(!timer_pending(&timerLed))
	{
		timerLed.expires = get_jiffies_64() + timeover; 
		timer_setup(&timerLed, kerneltimer_func, 0);
		add_timer(&timerLed);
	}
}
void kerneltimer_func(struct timer_list* t)
{
    gpioLedSet(ledVal);
    ledVal = ~ledVal & 0xff;
	gpioLedSet(ledVal);
    mod_timer(t, get_jiffies_64() + timerVal);
}



irqreturn_t keyIsr(int irq, void* data)
{
	int i;
	keyDataStruct* pKeyData = (keyDataStruct*)data;
	for(i=0;i<GPIOCNT;i++)
	{
		if(irq == pKeyData->irqKey[i])
		{
			if(mutex_trylock(&keyMutex) != 0)
			{
				pKeyData->keyNum = i + 1;
				mutex_unlock(&keyMutex);
				break;
			}
		}
	}
	wake_up_interruptible(&waitQueueRead);
	return IRQ_HANDLED;
}
static long ledkey_ioctl (struct file* filp, unsigned int cmd, unsigned long arg)
{

	int err=0;
	int size=0;

	if(_IOC_TYPE(cmd) != IOCTL_TIMER_MAGIC) return -EINVAL;
	if(_IOC_NR(cmd) >= IOCTL_TIMER_MAXNR) return -EINVAL;
	size = _IOC_SIZE(cmd);
	if(size)
	{
		if((_IOC_DIR(cmd) & _IOC_READ) || (_IOC_DIR(cmd) & _IOC_WRITE) )
		{
			err = access_ok((void*)arg,size);
			if(!err) return err;
		}
	}
	switch(cmd)
	{
		case TIMER_START:
			kerneltimer_registertimer(timerVal);
			break;
		case TIMER_STOP:
    		if (timer_pending(&timerLed))
			{
        		del_timer(&timerLed);
			}
			break;
		case TIMER_VALUE:
			{
				err = copy_from_user((void*)(&(driver_info.timer_val)),(void*)arg,size);
				timerVal=driver_info.timer_val;
				if(err != 0) 
				{
					return -EFAULT;
				}
				break;
			}
        default:
            err =-E2BIG;
            break;



	}
    return err;
}

static int ledkey_open (struct inode* inode, struct file* filp)
{

	int result=0;
	keyDataStruct* pKeyData;
	char * irqName[GPIOCNT] = {"irqKey0","irqKey1","irqKey2","irqKey3","irqKey4","irqKey5","irqKey6","irqKey7",};
#if DEBUG
    int num0 = MAJOR(inode->i_rdev); 
    int num1 = MINOR(inode->i_rdev); 
    printk( "call open -> major : %d\n", num0 );
    printk( "call open -> minor : %d\n", num1 );
#endif

	pKeyData = (keyDataStruct*)kmalloc(sizeof(keyDataStruct),GFP_KERNEL);
	if(!pKeyData)
		return -ENOMEM;

	memset(pKeyData,0,sizeof(keyDataStruct));
	filp->private_data = pKeyData;
	
	memset(&driver_info,0,sizeof(timerVal_data));

	result = irqKeyInit(pKeyData);
	if(result < 0)
		return result;

	for(int i=0;i<GPIOCNT;i++)
	{
		result = request_irq(pKeyData->irqKey[i],keyIsr,IRQF_TRIGGER_RISING,irqName[i],pKeyData);
		if(result < 0 )
			return result;
	}

	if(openFlag)
		return -EBUSY;
	else
		openFlag = 1;

	if(!try_module_get(THIS_MODULE))
		return -ENODEV;

    return 0;
}

static ssize_t ledkey_read(struct file* filp, char* buf, size_t count, loff_t* f_pos)
{
	keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;


	if(!(filp->f_flags & O_NONBLOCK))
	{
		wait_event_interruptible(waitQueueRead, pKeyData->keyNum);	
	}

	put_user(pKeyData->keyNum,buf);

	if(mutex_trylock(&keyMutex) != 0)
	{
		if(pKeyData->keyNum != 0)
		{
			pKeyData->keyNum = 0;
		}
	}
	mutex_unlock(&keyMutex);

    return sizeof(pKeyData->keyNum);
}

static ssize_t ledkey_write \
	(struct file* filp, const char* buf, size_t count, loff_t* f_pos)
{
	char kbuf;
	get_user(kbuf,buf);
	ledVal=kbuf;
	return sizeof(kbuf);
}
static __poll_t ledkey_poll\
	(struct file* filp, struct poll_table_struct* wait)
{
    unsigned int mask=0;
    keyDataStruct* pKeyData = (keyDataStruct*)filp->private_data;

    if(wait->_key & POLLIN)
        poll_wait(filp, &waitQueueRead, wait);
    if(pKeyData->keyNum > 0)
        mask = POLLIN;

    return mask;

}
static int ledkey_release (struct inode* inode, struct file* filp)
{
	keyDataStruct* pKeyData = (keyDataStruct*)filp->private_data;
    printk( "call release \n" );

	irqKeyFree(pKeyData);

	module_put(THIS_MODULE);
	openFlag = 0;
	if (timer_pending(&timerLed))
	{
		del_timer(&timerLed);
	}
	gpioLedSet(0x00);
	if(filp->private_data)
		kfree(filp->private_data);
    return 0;
}

static int gpioLedInit(void)
{
	int i;
	int ret=0;
	char gpioName[10];
	for(i=0;i<GPIOCNT;i++)
	{
		sprintf(gpioName,"led%d",i);
		ret = gpio_request(gpioLed[i], gpioName);
		if(ret < 0)
		{
			printk("Failed request gpio%d error\n",gpioLed[i]);
			return ret;
		}
		ret = gpio_direction_output(gpioLed[i],OFF);
		if(ret < 0)
		{
			printk("Failed directon_output gpio%d error\n",gpioLed[i]);
			return ret;
		}
	}
	return ret;
}
static void gpioLedSet(long val)
{
	int i;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_set_value(gpioLed[i],(val & (0x01 << i)));
	}
}
static void gpioLedFree(void)
{
	int i;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_free(gpioLed[i]);
	}
}
static int gpioKeyInit(void)
{
	int i;
	int ret=0;
	char gpioName[10];
	for(i=0;i<GPIOCNT;i++)
	{
		sprintf(gpioName,"key%d",i);
		ret = gpio_request(gpioKey[i], gpioName);
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", gpioKey[i]);
			return ret;
		}
		ret = gpio_direction_input(gpioKey[i]);
		if(ret < 0) {
			printk("Failed direction_output gpio%d error\n", gpioKey[i]);
       	 	return ret;
		}
	}
	return ret;
}
static int irqKeyInit(keyDataStruct* pkeyData)
{
	int i;
	int ret=0;
	for(i=0;i<GPIOCNT;i++)
	{
		pkeyData->irqKey[i] = gpio_to_irq(gpioKey[i]);
		if(pkeyData->irqKey[i] < 0)
		{
			printk("Failed gpio_to_irq() gpio%d error\n",gpioKey[i]);
			return pkeyData->irqKey[i];
		}
	}
	return ret;
}

static void irqKeyFree(keyDataStruct* pKeyData)
{
	int i;
	for(i=0;i<GPIOCNT;i++)
	{
		free_irq(pKeyData->irqKey[i], pKeyData);
	}
}

static void gpioKeyFree(void)
{
	int i;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_free(gpioKey[i]);
	}
}

struct file_operations ledkey_fops =
{
    .open     = ledkey_open,     
    .read     = ledkey_read,     
    .write    = ledkey_write,  
	.unlocked_ioctl = ledkey_ioctl,  
	.poll	  = ledkey_poll,
    .release  = ledkey_release,  
};

static int ledkey_init(void)
{
    int result;

    printk( "call ledkey_init \n" );    

	mutex_init(&keyMutex);

	result = gpioLedInit();
	if(result < 0)
		return result;

	result = gpioKeyInit();
	if(result < 0)
		return result;


    result = register_chrdev( ECKIM_DEV_MAJOR, ECKIM_DEV_NAME, &ledkey_fops);
    if (result < 0) return result;

    return 0;
}

static void ledkey_exit(void)
{
    printk( "call ledkey_exit \n" );    

    unregister_chrdev( ECKIM_DEV_MAJOR, ECKIM_DEV_NAME );

	gpioLedFree();
	gpioKeyFree();
	mutex_destroy(&keyMutex);
}

module_init(ledkey_init);
module_exit(ledkey_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ecKim");
MODULE_DESCRIPTION("led key test module");
