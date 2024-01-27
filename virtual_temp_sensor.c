#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <linux/device.h>

#define mDeviceName                 "virtual_temp_sensor"
#define mCommandFahrenheit          _IOR('k', 1, int)

static int gMajorNumber;
int gCelsiusTemp;
static struct cdev sVirtualCharDevice;
static struct class *sVirtualCharDeviceClass;
static struct device *sVirtualCharDeviceDev;

static int fDeviceOpen(struct inode *, struct file *);
static ssize_t fDeviceRead(struct file *, char *, size_t, loff_t *);
static int fDeviceRelease(struct inode *, struct file *);
static long fDeviceIoctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);

static struct file_operations sFileOperation = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = fDeviceIoctl,
    .open  = fDeviceOpen,
    .read  = fDeviceRead,
    .release = fDeviceRelease
};

static int __init fVirtualSensorInit(void) {
    printk(KERN_INFO "VirtualTempSensor: Initializing the VirtualTempSensor\n");

    // Allocate a major number dynamically
    gMajorNumber = register_chrdev(0, mDeviceName, &sFileOperation);
    if (gMajorNumber < 0) 
    {
        printk(KERN_ALERT "VirtualTempSensor failed to register a major number\n");
        return gMajorNumber;
    }
    printk(KERN_INFO "VirtualTempSensor: registered correctly with major number %d\n", gMajorNumber);

    // Create the device class
    sVirtualCharDeviceClass = class_create(mDeviceName);

    if (IS_ERR(sVirtualCharDeviceClass)) 
    {
        unregister_chrdev(gMajorNumber, mDeviceName);
        printk(KERN_ALERT "Failed to create the device class\n");
        return PTR_ERR(sVirtualCharDeviceClass);
    }

    // Create the device
    sVirtualCharDeviceDev = device_create(sVirtualCharDeviceClass, NULL, MKDEV(gMajorNumber, 0), NULL, mDeviceName);

    if (IS_ERR(sVirtualCharDeviceDev)) 
    {
        class_destroy(sVirtualCharDeviceClass);
        unregister_chrdev(gMajorNumber, mDeviceName);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(sVirtualCharDeviceDev);
    }

    cdev_init(&sVirtualCharDevice, &sFileOperation);
    sVirtualCharDevice.owner = THIS_MODULE;

    int err = cdev_add(&sVirtualCharDevice, MKDEV(gMajorNumber, 0), 1);

    if (err < 0) 
    {
        device_destroy(sVirtualCharDeviceClass, MKDEV(gMajorNumber, 0));
        class_destroy(sVirtualCharDeviceClass);
        unregister_chrdev(gMajorNumber, mDeviceName);
        printk(KERN_ALERT "VirtualTempSensor: Failed to add cdev\n");
        return err;
    }

    printk(KERN_INFO "VirtualTempSensor: device class created correctly\n");
    return 0;
}

static void __exit fVirtualSensorExit(void) 
{
    cdev_del(&sVirtualCharDevice);

    device_destroy(sVirtualCharDeviceClass, MKDEV(gMajorNumber, 0));

    class_destroy(sVirtualCharDeviceClass);

    unregister_chrdev(gMajorNumber, mDeviceName);

    printk(KERN_INFO "VirtualTempSensor: Removed VirtualTempSensor\n");
}

static int fDeviceOpen(struct inode *inode, struct file *file) 
{
    printk(KERN_INFO "VirtualTempSensor: Device has been opened\n");
    return 0;
}

static ssize_t fDeviceRead(struct file *filp, char *buffer, size_t len, loff_t *offset) 
{
    get_random_bytes(&gCelsiusTemp, sizeof(gCelsiusTemp));
    gCelsiusTemp = abs(gCelsiusTemp) % 41;  // Generate a random temperature between 0 and 40

    printk(KERN_INFO "VirtualTempSensor: Random temperature is %d\n", gCelsiusTemp);

    int error_count = 0;
    error_count = copy_to_user(buffer, &gCelsiusTemp, sizeof(gCelsiusTemp));

    if (error_count == 0) 
    {
        return sizeof(gCelsiusTemp);
    } 
    else 
    {
        return -EFAULT;
    }
}

static long fDeviceIoctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    int fahrenheitTemp;

    switch ( ioctl_num )
    {
        case mCommandFahrenheit:
            
            fahrenheitTemp = (gCelsiusTemp * 9 / 5) + 32;

            copy_to_user((int *)ioctl_param, &fahrenheitTemp, sizeof(fahrenheitTemp));
            break;
        
        default:
            break;
    }

    return 0;
}

static int fDeviceRelease(struct inode *inode, struct file *file) 
{
    printk(KERN_INFO "VirtualTempSensor: Device successfully closed\n");
    return 0;
}

module_init(fVirtualSensorInit);
module_exit(fVirtualSensorExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oguzhan Caglar");
MODULE_DESCRIPTION("A virtual temperature sensor Linux kernel module");
MODULE_VERSION("0.1");