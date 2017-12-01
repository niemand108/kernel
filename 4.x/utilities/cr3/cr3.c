// task_cr3 based on: https://carteryagemann.com/pid-to-cr3.html
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/sched/signal.h>

unsigned long task_cr3(struct task_struct *task)
{
    struct mm_struct *mm;
    void *cr3_virt;
    unsigned long cr3_phys;

    if (task == NULL)
        return -1; // 0xFFFFFFFFF

    mm = task->mm;

    // mm can be NULL in some rare cases (e.g. kthreads)
    // when this happens, we should check active_mm
    if (mm == NULL) {
        mm = task->active_mm;
    }

    if (mm == NULL){
      	printk( KERN_DEBUG "PID %d: null in mm_struct", task->pid);
        return 0; // this shouldn't happen, but just in case
    }

    cr3_virt = (void *) mm->pgd;
    cr3_phys = virt_to_phys(cr3_virt);

    return cr3_phys;
}

ssize_t read_proc(struct file *filp, char *buf, size_t length, loff_t *offset)
{
  if(*offset > 0){
    return 0;
  }

  long cr3;
  char buf_temp[ 2 + 16 + 2 + 10 + 1]; // "0xXXXXXXXX[16 -> 64bits]: pid[10] \n"
  struct task_struct *t;

  for_each_process(t) {
    
    cr3 = task_cr3(t);
    sprintf(buf_temp, "0x%x: %ld \n", cr3, t->pid);  
    
    if(strlen(buf) + strlen(buf_temp) >= length)
      goto end_read;
    
    strcat(buf, buf_temp);
  }

  end_read:
  *offset = 1; 
  return strlen(buf);
}

struct file_operations proc_fops = {
read: read_proc
};

int cr3_init(void) 
{
  proc_create("cr3", 0600, NULL,  &proc_fops);
  return 0;
}

void cr3_cleanup(void)
{
  remove_proc_entry("cr3", NULL);
}

MODULE_LICENSE("GPL");
module_init(cr3_init);
module_exit(cr3_cleanup);
