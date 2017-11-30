#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/completion.h>

#define MODULE_NAME "niemand"

MODULE_LICENSE("GPL");

static int niemand_major = 0;

/*
 * Declare "completion" ready to be used
 * */
DECLARE_COMPLETION(completion);

ssize_t niemand_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
  printk(KERN_DEBUG "procces %i-> %s sleeping until another process write",\
                             current->pid, current->comm);
  /* waits until another process call to "complete(&completion);"
   * */
  wait_for_completion(&completion);
  printk(KERN_DEBUG "process %i -> %s awoken",
                        current->pid, current->comm);
  return count;
}

ssize_t niemand_write(struct file *filp, const char __user *buf,\
                      size_t count, loff_t *pos)
{
  printk(KERN_DEBUG "process %i -> %s awakening one reader process", 
                        current->pid, current->comm);
  /* Communicates to kernel a "completion", 
   * which unlocks a wait_for_completion process */
  complete(&completion);
  return count;
}

struct file_operations niemand_fops = {
  .owner = THIS_MODULE,
  .read = niemand_read,
  .write = niemand_write
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
