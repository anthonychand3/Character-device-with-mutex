
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
#define DEVICE_NAME "Inputamj"
#define CLASS_NAME "InputClass"
#define MESSAGE_LIMIT 255

// GPL type of license
MODULE_LICENSE("GPL");

// Authors of this driver
MODULE_AUTHOR("Anthony Chand, Mauricio Mendez, James Harrison");

// Description of driver
MODULE_DESCRIPTION("Programming Assignment 3");

// Version of driver
MODULE_VERSION("2.1");

// struct for the mutex
extern struct mutex mutexAMJ;

// Struct pointer for device class
static struct class* charClass = NULL;

// Struct pointer for device class
static struct device* InputDeviceamj = NULL;

//used to store messgaes
static char message[256];

// Initialize the number of open devices to 0
static int openedDevices = 0;

// used to store the major number of the device
static int majorNumber;
static short messageSize;

// Start and length variable for the buffer
extern int start, length;

// Buffer for driver
extern signed char bufferAMJ[MESSAGE_LIMIT];



static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

// basic set up for file operations
// not need if we were working on device drivers for example a graphics card
static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init dev_init(void)
{
    printk(KERN_INFO "InputDeviceamj: Initializing module\n");

    // 0 is used to dynamically assign a major number to the device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

    // major number is not allowed to be negative
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "InputDeviceamj: Failed to register a major number\n");

        return majorNumber;
    }

    printk(KERN_INFO "InputDeviceamj: registered correctly with major number: %d\n", majorNumber);

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

    printk(KERN_INFO "InputDeviceamj: device class registered correctly\n");

    // Device_create is used to register the device driver
    InputDeviceamj = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

    // If there is an error
    if (IS_ERR(InputDeviceamj))
    {
        // The class is destroyed
        class_destroy(charClass);

        // The major number is unregistered
        unregister_chrdev(majorNumber, DEVICE_NAME);

        // Device could not be created
        printk(KERN_ALERT "Failed to create device\n");

        return PTR_ERR(InputDeviceamj);
    }

    // The device was created properly
    printk(KERN_INFO "InputDeviceamj: device class created correctly\n");

    return 0;
}

// Code to remove device driver, class, and unregister major number
static void __exit dev_exit(void)
{
    // The device is removed
    device_destroy(charClass, MKDEV(majorNumber, 0));

    // The class is unregistered
    class_unregister(charClass);

    // The clas is destroyed
    class_destroy(charClass);

    // The major number is unregistered
    unregister_chrdev(majorNumber, DEVICE_NAME);


    printk(KERN_INFO "InputDeviceamj: GoodBye\n");
}

// Function called when the device is opened
static int dev_open(struct inode *inodep, struct file *fp)
{
    printk(KERN_INFO "InputDeviceamj: Device has been opened\n");

    return 0;
}

// Function called when the device is being read
static ssize_t dev_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    // Counter storing the number of errors
    int errorCounter = 0;

    // Send buffer string to a user
    errorCounter = copy_to_user(buffer, message, messageSize);

    if (errorCounter == 0)
    {
        printk(KERN_INFO "InputDeviceamj: Sent %d characters\n", messageSize);

        messageSize = 0;

        return 0;
    }
    else
    {
        printk(KERN_INFO "InputDeviceamj: Failed to send %d characters\n", errorCounter);

        return -EFAULT;
    }
}

// static ssize_t dev_write(struct file *fp, const char *buffer, size_t len, loff_t *offset)
// {
//     int errorCounter = 0;
//
//     // There is more room left in the buffer
//     // So we can add more, we dont know how many more at this step yet
//     if (messageSize < MESSAGE_LIMIT)
//     {
//
//         // If the message size + the len is less than the message limit
//         // Then the buffer has room for an additional message
//         if (messageSize + len < MESSAGE_LIMIT)
//         {
//             errorCounter = copy_from_user(message, buffer, len);
//
//             // Sending was succesful
//             if (errorCounter == 0)
//             {
//                 printk(KERN_INFO "InputDeviceamj: Received %zu characters from user\n", len);
//
//                 // Update the size of the message
//                 messageSize = strlen(message);
//
//                 return 0;
//             }
//
//             // Failed to send
//             else
//             {
//                 printk(KERN_INFO "InputDeviceamj: Failed to send %d characters\n", errorCounter);
//
//                 return -EFAULT;
//             }
//         }
//
//         // There isnt enough room for another message
//         // copy as many characters from the user as possible
//         else
//         {
//             errorCounter = copy_from_user(message, buffer, MESSAGE_LIMIT - messageSize);
//
//             // Sending was succesful
//             if (errorCounter == 0)
//             {
//                 printk(KERN_INFO "InputDeviceamj: Buffer limit reached. Received only %d characters from user\n", MESSAGE_LIMIT - messageSize);
//
//                 // Update the size of the message
//                 messageSize = strlen(message);
//
//                 return 0;
//             }
//
//             // Failed to send
//             else
//             {
//                 printk(KERN_INFO "InputDeviceamj: Failed to send %d characters\n", errorCounter);
//
//                 return -EFAULT;
//             }
//         }
//     }
//
//     // No characters can be recieved since the buffer is full
//     else
//     {
//         printk(KERN_INFO "InputDeviceamj: Buffer is full. No characters written.\n");
//
//         return 0;
//     }
// }

// Function called when the device is ready to be closed
static int dev_release(struct inode *inodep, struct file *fp)
{
    // Close Device
    printk(KERN_INFO "InputDeviceamj: Device closed\n");

    return 0;
}

// Init functions and clean up functions
module_init(dev_init);
module_exit(dev_exit);
