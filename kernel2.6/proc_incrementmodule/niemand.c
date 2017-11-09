#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>

#define MAX_SIZE         100
#define MODULE_NAME     "niemand"
#define PROC_NAME       MODULE_NAME

MODULE_LICENSE("GPL");


/* * * *
 * Global variables (scope: this module) 
 * * * */

int niemand_major;      // major number of this device
int niemand_minor = 0;  // minor number of this device

/* Our structure definition */
struct niemand_dev {
  unsigned long number;         //a simple  number which we access
  struct semaphore sem;         //semaphore for mutual exclusion
  struct cdev cdev;             //structure for one/this character device
};

/* Pointer to our structure, we will have to initialize it*/
struct niemand_dev *niemand_device;

/* Inputs params for this module:
 *      prompt$   insmod niemand.ko niemand_major=x niemand_minor=y
 * */
module_param(niemand_major, int, S_IRUGO);  
module_param(niemand_minor, int, S_IRUGO);


/* This structure hold information about our /proc file
 * */
static struct proc_dir_entry *niemand_proc_file;


/* This function will be called when de /proc file is read 
 * , return the intern number without exclusion mutual */
int niemand_procfs_read( char *buffer, char **buffer_location, \
                           off_t offset, int buffer_length, \
                           int *eof, void *data)
{
  unsigned long ret;
  
  /* Only allo offset 0 */
  if(offset > 0) 
    ret = 0; 
  else 
    ret = sprintf(buffer, "%lu\n", niemand_device->number);
  
  return ret;
}

/* This function will be called when the /proc file is written
 * , storing the value (only when it is a long) and return
 * the real size written
 * */
int niemand_procfs_write(struct file *file, const char *buffer, \
                        unsigned long count, void *data)
{
  unsigned long val = 0;

  /* Buffer to store info for this module of MAX_SIZE
   * * */
  char niemand_procfs_buffer[MAX_SIZE] = "";  
  if(count > MAX_SIZE){
    count = MAX_SIZE;
  }
  if(copy_from_user(niemand_procfs_buffer /*to*/,\
                    buffer /*from*/,\
                    count /*size*/))
  {
    return -EFAULT;
  }

  if(0 != strict_strtoul(niemand_procfs_buffer, 0, &val))
  {
    printk(KERN_INFO "writting failed %s", niemand_procfs_buffer);
    return -EINVAL;
  }
  
  /* Mutual exclusion */
  if(down_interruptible(&niemand_device->sem)){
    printk(KERN_WARNING "Fail doing semaphore-down");
    return -ERESTARTSYS;
  }
  if(val < ULONG_MAX) 
  {
    niemand_device->number = (long) val;
  } else {
    niemand_device->number = 0;
  }
  
  up(&niemand_device->sem);
 
  return count;
}


/* Definition of a open-function for the future fops
 * */
int niemand_open(struct inode *inode, struct file *filp)
{

  /* From the inode of this character device we retrive information about
   * the structure that contains it(to inode->i_cdev) and we asocciate it
   * to the open file ( /dev/niemand ), consequently we will be able to
   * access the structure of our device in the write/read functions of our
   * fops (kernel pass them through input parameters) .
   * */
  struct niemand_dev *dev;
  dev = container_of(inode->i_cdev, struct niemand_dev, cdev);
  filp->private_data = dev;
  return 0;
}

/* Definition of a read-function for the future fops
 * */
ssize_t niemand_read(struct file *filp, char __user *buf,\ 
                        size_t count, loff_t *f_pos)
{
  int ret = 0;                                 
  char stringint[MAX_SIZE];
  /* Retrieve our device struct from the descripter file */
  struct niemand_dev *dev = filp->private_data;
  
  /* Mutual exclusion */
  if(down_interruptible(&dev->sem)){
    printk(KERN_WARNING "Fail doing semaphore-down");
    return -ERESTARTSYS;
  }
  

  if(dev->number >= ULONG_MAX)
  {
    ret = 1;
    dev->number = 0;
  } 
  else 
  {
    dev->number++;
    ret = sprintf(stringint, "%lu", dev->number);
  }

  if(printk_ratelimit())
    printk(KERN_NOTICE "%lu++, size: %d", dev->number, ret);

  /* Copy to user memory(buf) a piece of
   * kernel memory(stringing with size=ret) */
  if(copy_to_user(buf, stringint , ret)){
    ret = -EFAULT;
  }
  up(&dev->sem);
  /* End of mutual exclusion */
  return ret;
}

/* Definition of a release-function for the future fops
 * */
int niemand_release(struct inode *inode, struct file *filp)
{
  /* Nothing to do here */
  return 0;
}


/* 
 * Initialize our fops associating theirs functions 
 * */
static struct file_operations niemand_fops = {
  .owner = THIS_MODULE,
  .read = niemand_read,
  .open = niemand_open,
  .release = niemand_release
};

/* EXIT MODULE
 * */
static void niemand_exit_module(void)
{
  dev_t devno = MKDEV(niemand_major, niemand_minor);

  remove_proc_entry(PROC_NAME, NULL /* main directory proc */);
  
  /* Release used memory and also registered devices
   * and its structures */
  cdev_del(&niemand_device->cdev); 
  kfree(niemand_device);
  unregister_chrdev_region(devno, 1);
}

/*
 * INIT MODULE
 * */
static int niemand_init_module(void)
{
  dev_t dev = 0;
  int err, ret;
  
  /* Create /proc file */
  niemand_proc_file = create_proc_entry(MODULE_NAME, 0644, NULL);
  if(niemand_proc_file == NULL) 
  {
    remove_proc_entry(MODULE_NAME, NULL /*main directory proc */);
    printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROC_NAME);
    return -ENOMEM;
  }

  /*We specify what read/write/etc functions will use and
   * also its permissions, etc. */
  niemand_proc_file->read_proc = niemand_procfs_read;
  niemand_proc_file->write_proc = niemand_procfs_write;
  niemand_proc_file->mode = S_IFREG | S_IRUGO;
  niemand_proc_file->uid = 0;

  /* only we are going to have defined scull_major if we have passed
   * it as module param, if not -> retrive automagicly */
  if(niemand_major) {
    dev = MKDEV(niemand_major, niemand_minor);
    ret = register_chrdev_region(dev, 1, MODULE_NAME);
  } else {
    ret = alloc_chrdev_region(&dev, niemand_minor, 1, MODULE_NAME);
    niemand_major = MAJOR(dev);
  }

  if(ret < 0) {
    printk(KERN_WARNING "cant get major %d number for device %s", niemand_major, MODULE_NAME);
    return ret;
  }
  
  /* Memory reserved for our structure */
  niemand_device = kmalloc(sizeof(struct niemand_dev), GFP_KERNEL);
  if(!niemand_device){
    goto fail;
  }
  /* Important: avoid leaks of kernel memory(passwords, stacks, variables, etc */
  memset(niemand_device, 0, sizeof(struct niemand_dev));

  /* Initialize variables of our structure */
  niemand_device->number = 0;
  sema_init(&niemand_device->sem, 1);
  /* ... also initialize/allocate the cdev structure */
  cdev_init(&niemand_device->cdev, &niemand_fops);
  niemand_device->cdev.owner = THIS_MODULE;
  // niemand_device->cdev.ops = &niemand_fops;
 
  /* with cdev_add the device is goint to be available for the kernel and
   * consequently for its users */
  err = cdev_add(&niemand_device->cdev, dev, 1);
  if(err){
    printk(KERN_NOTICE "error adding niemand0 (cdevadd): %d\n", err);
  }
  return 0;

fail: 
  niemand_exit_module();
  return ret; 
}

/* Say to the module where/how inits/exits */
module_init(niemand_init_module);
module_exit(niemand_exit_module);
