#define pr_fmt(fmt) "[solution]: " fmt

#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h> /* for sprintf() */
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/uaccess.h> /* for get_user and put_user */
#include <linux/string.h>
#include <asm/errno.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EvMik205");
MODULE_DESCRIPTION("A sample Linux character driver");
MODULE_VERSION("0.1");

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t,
	loff_t *);

#define DEVICE_NAME "solution"
#define DEVICE_MAJOR 240
#define BUF_LEN 80

enum {
    DEV_FREE = 0,
    DEV_OPENED = 1,
};

static atomic_t already_open = ATOMIC_INIT(DEV_FREE);

static char msg[BUF_LEN + 1];

static struct class *cls;

static struct file_operations dev_fops = {
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .release = dev_release,
};

static int sum = 17;

/** @brief Init module & create character device
 *  @return Returns 0 on success, error code otherwise.
 */
static int __init dev_init(void)
{
    int rc;

    rc = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &dev_fops);

    if (rc < 0) {
	pr_alert("Registering char device failed with %d\n", rc);
	return rc;
    }

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(DEVICE_MAJOR, 0), NULL, DEVICE_NAME);

    pr_info("Created /dev/%s with major number = %d\n", DEVICE_NAME, DEVICE_MAJOR);

    return 0;
}

/** @brief Cleanup function.
 */
static void __exit dev_exit(void)
{
    device_destroy(cls, MKDEV(DEVICE_MAJOR, 0));
    class_destroy(cls);

    unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
}

/** @brief Called when a process tries to open the device file, like
 *  "sudo cat /dev/chardev". Trying to get lock on device.
 *  @return On success zero, otherwise -EBUSY
 *
 */
static int dev_open(struct inode *inode, struct file *file)
{
    if (atomic_cmpxchg(&already_open, DEV_FREE, DEV_OPENED))
	return -EBUSY;

    sprintf(msg, "%d", sum);
    try_module_get(THIS_MODULE);

    return 0;
}

/** @brief Called when a process closes the device file. Frees dev lock.
 *  @return 0
 */
static int dev_release(struct inode *inode, struct file *file)
{
    atomic_set(&already_open, DEV_FREE);
    module_put(THIS_MODULE);

    return 0;
}

/** @brief Called on reading from device
 *  @return 
 */
static ssize_t dev_read(struct file *filp,
			   char __user *buffer,
			   size_t length,
			   loff_t *offset)
{
    int rc = 0;

    rc = copy_to_user(buffer, msg, strlen(msg));

    if (rc != 0) 
    {
	pr_alert("Sent %d bytes failed\n", (int) strlen(msg));
	return -EFAULT;
    }

    pr_info("%s\n", msg);
    
    return 0;
}

static ssize_t dev_write(struct file *filp, const char __user *buffer,
			    size_t len, loff_t *off)
{
    int a = 0;
    int b = 0;
    int rc;

    rc = copy_from_user(msg, buffer, BUF_LEN);
    if (rc != 0) {
	pr_alert("Writing %d bytes failed\n", rc);
	return -EFAULT;
    }
  
    rc = sscanf(msg, "%d %d", &a, &b);
    if (rc != 2) {
	pr_alert("Error reading arguments %d\n", rc);
	return -EFAULT;
    }

    sum = a + b;
    sprintf(msg, "%d", sum);
    pr_info("Got numbers a=%d, b=%d, sum=%d, msg=%s\n", a, b, sum, msg);
    
    return len;
}

module_init(dev_init);
module_exit(dev_exit);

