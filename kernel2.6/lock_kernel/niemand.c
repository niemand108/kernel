#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/smp_lock.h>

#define MODULE_NAME "niemand"

MODULE_LICENSE("GPL");

static int niemand_major = 0;

ssize_t niemand_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
  unsigned int i= 0;
  printk(KERN_DEBUG "locking kernel...");
  
  lock_kernel();
  
  /*In 1.5Mhz cpu frecuency, more than 5 seconds */
  do {
    i++;
    i--;
    i++;
  } while (i < 1500000000 );

  unlock_kernel();

  printk(KERN_WARNING "unlocked kernel");
 
  return count;
}


struct file_operations niemand_fops = {
  .owner = THIS_MODULE,
  .read = niemand_read,
};

int niemand_init(void)
{
  int dev = 0;
  dev = register_chrdev(niemand_major, MODULE_NAME, &niemand_fops);

  if(dev <0)
    return dev;
  if(niemand_major == 0)
    niemand_major = dev;

  return 0;
}

void niemand_cleanup(void)
{
  unregister_chrdev(niemand_major, MODULE_NAME);
}

module_init(niemand_init);
module_exit(niemand_cleanup);
