#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/smp_lock.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>

#define MODULE_NAME     "niemand"
#define MAX_PROCESS     500
#define PROCFS_NAME     MODULE_NAME

static struct proc_dir_entry *niemand_procfs;

int strtoint(unsigned char *str){
    int ret;
    if(0 != strict_strtoul(str, 0, &ret))
      return -1;
    return ret;
}

int procfs_read( char *buffer, \
                 char **buffer_location, \
                 off_t offset, int buffer_length, \
                 int *eof, void *data)
{
  int ret;
  struct task_struct *g, *t;
  int pids_in_kernel[MAX_PROCESS];
  int pidnumber, i, length = 0;
  struct dentry *cur_dentry, * parent_dentry;
  unsigned char *dname, buffer_temp[20], *buffer_res, *buffer_concat;
  bool hidden;
  struct file *proc_filp;

  if(offset > 0)
    return 0;

  lock_kernel();
  
  do_each_thread(g,t){
    if(length > MAX_PROCESS){
      printk(KERN_NOTICE "Max number of proccess reach. ");
      break;
    }
        pids_in_kernel[length] = t->pid;
        length++;
  } while_each_thread(g,t);
  
  unlock_kernel();
   
  proc_filp = filp_open( "/proc", O_RDONLY, 0);  
  parent_dentry = (proc_filp->f_path).dentry;

  sprintf(buffer, ""); 

  list_for_each_entry(cur_dentry, &parent_dentry->d_subdirs, \
                       d_u.d_child){
        dname = cur_dentry->d_name.name;
        pidnumber = strtoint(dname);
        if(pidnumber > 0){
          hidden = true;
          for(i = 0; i < length && hidden; i++){
            if(pids_in_kernel[i] == pidnumber)
            {
              hidden = false;
            }
          }
          if(hidden){
            sprintf(buffer_temp, "Hidden: %d\n", pidnumber);
            buffer_concat = kmalloc( strlen(buffer) + strlen(buffer_temp) + 1, GFP_USER);
            strcpy(buffer_concat, buffer);
            strcat(buffer_concat, buffer_temp);
            memcpy(buffer, buffer_concat, strlen(buffer_concat));
            kfree(buffer_concat);
            printk(KERN_DEBUG "Procces hidden PID: %d", pidnumber);
          }
        }
  }
  return strlen(buffer);
}


int niemand_init(void)
{
  niemand_procfs = create_proc_entry(PROCFS_NAME, 0644, NULL);
  if(niemand_procfs == NULL){
    remove_proc_entry(PROCFS_NAME, NULL);
    printk(KERN_ALERT "Error: Could no initialize /proc/%s", PROCFS_NAME);
    return -ENOMEM;
  }

  niemand_procfs->read_proc = procfs_read;
  niemand_procfs->mode = S_IFREG | S_IRUGO;
  niemand_procfs->uid = 0;

  return 0;
}

void niemand_cleanup(void)
{
  remove_proc_entry(PROCFS_NAME, NULL);
}

module_init(niemand_init);
module_exit(niemand_cleanup);
