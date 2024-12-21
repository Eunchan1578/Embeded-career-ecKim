#include <linux/kernel.h>
#include <linux/gpio.h>
#define OFF 0
#define ON 1
#define GPIOCNT 8

int gpioLedInit(void);
void gpioLedSet(int val);
void gpioLedFree(void);
int gpioKeyInit(void);
long gpioKeyGet(void);
void gpioKeyFree(void);

int gpioLed[GPIOCNT] = {518,519,520,521,522,523,524,525};
int gpioKey[GPIOCNT] = {528,529,530,531,532,533,534,535};
long sys_ecKimsyscall(long val)
{
	int ret = 0;
	ret = gpioLedInit();
	if(ret<0)
		return ret;
	gpioLedSet(val);
	gpioLedFree();

	ret = gpioKeyInit();
	if(ret<0)
		return ret;
	val = gpioKeyGet();
	gpioKeyFree();
	return val;
}

long gpioKeyGet(void)
{
	long retVal=0;
	for(int i=0;i<GPIOCNT;i++)
	{
		retVal = retVal+(((long)gpio_get_value(gpioKey[i]))<<i);
	}
	return retVal;
}
int gpioLedInit(void)
{
	char gpioName[10];
	int i = 0;
	int ret = 0;
	for(i=0;i<GPIOCNT;i++)
	{
		sprintf(gpioName,"led%d",i);
		ret = gpio_request(gpioLed[i],gpioName);
		if(ret<0)
		{
			printk("Failed Request gpio%d error\n",gpioLed[i]);
			return ret;
		}
	}
	for(i=0;i<GPIOCNT;i++)
	{
		ret = gpio_direction_output(gpioLed[i],OFF);
		if(ret<0)
		{
			printk("Failed Direction_output gpio%d error\n",gpioLed[i]);
			return ret;
		}
	}
	return ret;
}
int gpioKeyInit(void)
{
	char gpioName[10];
	int i = 0;
	int ret = 0;
	for(i=0;i<GPIOCNT;i++)
	{
		sprintf(gpioName,"key%d",i);
		ret = gpio_request(gpioKey[i],gpioName);
		if(ret<0)
		{
			printk("Failed Request gpio%d error\n",gpioKey[i]);
			return ret;
		}
	}
	for(i=0;i<GPIOCNT;i++)
	{
		ret = gpio_direction_input(gpioKey[i]);
		if(ret<0)
		{
			printk("Failed Direction_output gpio%d error\n",gpioKey[i]);
			return ret;
		}
	}
	return ret;
}
void gpioLedSet(int val)
{

	int i = 0;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_set_value(gpioLed[i],(val & (0x01 << i)));
	}
}
void gpioLedFree(void)
{
	int i = 0;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_free(gpioLed[i]);
	}
}

void gpioKeyFree(void)
{
	int i = 0;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_free(gpioKey[i]);
	}
}
