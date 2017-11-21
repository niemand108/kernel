#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/types.h>

void (*my_dereference_funptr)(void);

ssize_t write_proc(struct file *filp,const char *buf,\
                   size_t count,loff_t *offp)
{
  my_dereference_funptr();
  return count;
}

struct file_operations proc_fops = {
  write: write_proc
};

int proc_init (void) {
 proc_create("nullderef", 0666, NULL,&proc_fops);
 return 0;
}

void proc_cleanup(void) {
 remove_proc_entry("nullderef",NULL);
}

MODULE_LICENSE("GPL"); 
module_init(proc_init);
module_exit(proc_cleanup);

