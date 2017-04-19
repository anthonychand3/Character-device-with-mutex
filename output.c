
/*
character-mode linux device driver Assignment for COP 4600
Based off the tutorial by Derek Molloy http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
by
Anthony Chand
Mauricio Mendez
James Harrison
*/

// Loading the kernel libraries
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>

// the name of out device
#define DEVICE_NAME "Outputamj"
#define CLASS_NAME "OutputClass"
#define MESSAGE_LIMIT 255

// GPL type of license
MODULE_LICENSE("GPL");

// Authors of this driver
MODULE_AUTHOR("Anthony Chand, Mauricio Mendez, James Harrison");

// Description of driver
MODULE_DESCRIPTION("Programming Assignment 3");

// Version of driver
MODULE_VERSION("2.1");

// Define and Initialize mutex
DEFINE_MUTEX(mutexAMJ);

// Struct pointer for device class
static struct class* charClass = NULL;

// Struct pointer for device class
static struct device* OutputDeviceamj = NULL;

//used to store messgaes
static char message[256];

// used to store the major number of the device
static int majorNumber;
static short messageSize;
static int openedDevices = 0;

// Indexes for the buffer
int start = 0, length = 0;

// Buffer variable
signed char bufferAMJ[MESSAGE_LIMIT];

// Exports symbols for dynamic linking
EXPORT_SYMBOL(mutexAMJ);
EXPORT_SYMBOL(length);
EXPORT_SYMBOL(start);
EXPORT_SYMBOL(bufferAMJ);


static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);


// basic set up for file operations
// not need if we were working on device drivers for example a graphics card
static struct file_operations fops = {
    .open = dev_open,
    .write = dev_write,
    .release = dev_release,
};


static int __init dev_init(void)
{
    printk(KERN_INFO "OutputDeviceamj: Initializing module\n");

    // 0 is used to dynamically assign a major number to the device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

    // major number is not allowed to be negative
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "OutputDeviceamj: Failed to register a major number\n");

        return majorNumber;
    }

    printk(KERN_INFO "OutputDeviceamj: registered correctly with major number: %d\n", majorNumber);

    // Register the class for the device
    charClass = class_create(THIS_MODULE, CLASS_NAME);

    // If there is an error
    if (IS_ERR(charClass))
    {
        // The major number is unregistered
        unregister_chrdev(majorNumber, DEVICE_NAME);

        printk(KERN_ALERT "Failed to register device class\n");

        // Return error on pointer
        return PTR_ERR(charClass);
    }

    printk(KERN_INFO "OutputDeviceamj: device class registered correctly\n");

    // Device_create is used to register the device driver
    OutputDeviceamj = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

    // If there is an error
    if (IS_ERR(OutputDeviceamj))
    {
        // The class is destroyed
        class_destroy(charClass);

        // The major number is unregistered
        unregister_chrdev(majorNumber, DEVICE_NAME);

        // Device could not be created
        printk(KERN_ALERT "Failed to create device\n");

        return PTR_ERR(OutputDeviceamj);
    }

    // Initialize the mutex
    mutex_init(&mutexAMJ);

    // The device was created properly
    printk(KERN_INFO "OutputDeviceamj: device class created correctly\n");

    return 0;
}

// Code to remove device driver, class, and unregister major number
static void __exit dev_exit(void)
{
    // Destroy the mutex
    mutex_destroy(&mutexAMJ);

    // The device is removed
    device_destroy(charClass, MKDEV(majorNumber, 0));

    // The class is unregistered
    class_unregister(charClass);

    // The clas is destroyed
    class_destroy(charClass);

    // The major number is unregistered
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_INFO "OutputDeviceamj: GoodBye\n");
}

// Function called when the device is opened
static int dev_open(struct inode *inodep, struct file *fp)
{
    // if the mutex
    if (mutex_trylock(&mutexAMJ) == false)
    {
        printk(KERN_ALERT "OutputDeviceamj: Device cannot be used");

        return -EBUSY;
    }

    // Increment counter for open devices
    openedDevices++;

    // Device can be used
    printk(KERN_INFO "OutputDeviceamj: This device has been opened %d times\n", openedDevices);

    return 0;
}

// Function called when the device is being read
// static ssize_t dev_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
// {
//     // Counter storing the number of errors
//     int errorCounter = 0;
//
//     // Send buffer string to a user
//     errorCounter = copy_to_user(buffer, message, messageSize);
//
//     if (errorCounter == 0)
//     {
//         printk(KERN_INFO "OutputDeviceamj: Sent %d characters\n", messageSize);
//
//         messageSize = 0;
//
//         return 0;
//     }
//     else
//     {
//         printk(KERN_INFO "OutputDeviceamj: Failed to send %d characters\n", errorCounter);
//
//         return -EFAULT;
//     }
// }

static ssize_t dev_write(struct file *fp, const char *buffer, size_t len, loff_t *offset)
{

    ssize_t i, characters = 0;

    for (i = 0; i < len; i++)
    {
        if (length < MESSAGE_LIMIT)
        {
            bufferAMJ[(start + length) % MESSAGE_LIMIT] = buffer[i];
            length++;
            characters++;
        }

        printk(KERN_INFO "OutputDeviceamj: Received %zu characters from user\n", characters);

        return 0;

    }
}

// Function called when the device is ready to be closed
static int dev_release(struct inode *inodep, struct file *fp)
{
    // Unlock the mutex
    mutex_unlock(&mutexAMJ);

    // Close Device
    printk(KERN_INFO "OutputDeviceamj: Device closed\n");

    return 0;
}

// Init functions and clean up functions
module_init(dev_init);
module_exit(dev_exit);
