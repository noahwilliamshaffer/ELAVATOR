#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

// EXAMPLE
MODULE_LICENSE("Dual BSD/GPL");

#define BUF_LEN 100

static struct proc_dir_entry* proc_entry;

static int procfs_buf_len;

int isFirst = 1;

struct timespec curr_timespec = {0,0};
struct timespec prev_timespec;

static ssize_t procfile_read(struct file* file, char * ubuf, size_t count, loff_t *ppos)
{
        long int secElapsed, nanoElapsed;
        char currentTime[BUF_LEN] = "current time: ", seconds[11], nSeconds[10], period[] = {'.', '\0'}, endline[] = {'\n', '\0'};

        if(isFirst == 1){
                curr_timespec = current_kernel_time();

                printk("Original time: %ld.%ld\n", curr_timespec.tv_sec, curr_timespec.tv_nsec);

                sprintf(seconds, "%d",curr_timespec.tv_sec);
                sprintf(nSeconds, "%d",curr_timespec.tv_nsec);

                printk("current time: %s.%s\n", seconds, nSeconds);

                strcat(currentTime, seconds);
                strcat(currentTime, period);
                strcat(currentTime, nSeconds);
                strcat(currentTime, endline);

                isFirst = 0;


        }
        else{
                char elapsedTime[]= "elapsed time: ";

                prev_timespec = curr_timespec;
                curr_timespec = current_kernel_time();

                printk("Original time: %ld.%ld\n", curr_timespec.tv_sec, curr_timespec.tv_nsec);

                sprintf(seconds, "%d",curr_timespec.tv_sec);
                sprintf(nSeconds, "%d",curr_timespec.tv_nsec);

                printk("current time: %s.%s\n", seconds, nSeconds);

                strcat(currentTime, seconds);
                strcat(currentTime, period);
                strcat(currentTime, nSeconds);
                strcat(currentTime, endline);
                strcat(currentTime, elapsedTime);


                secElapsed = curr_timespec.tv_sec - prev_timespec.tv_sec - 1;
                nanoElapsed = curr_timespec.tv_nsec - prev_timespec.tv_nsec + 1000000000;

                if(nanoElapsed >= 1000000000){
                        secElapsed += 1;
                        nanoElapsed -= 1000000000;
                }

                sprintf(seconds, "%d",secElapsed);
                sprintf(nSeconds, "%d",nanoElapsed);


                char zero[] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '\0'};

                int i;

                for(i = strlen(nSeconds)-1; i >= 0; --i){
                        zero[i] = nSeconds[i] | zero[i];
                }

                strcat(currentTime, seconds);
                strcat(currentTime, period);
                strcat(currentTime, zero);
                strcat(currentTime, endline);

                printk("current time: %ld.%ld\n", curr_timespec.tv_sec, curr_timespec.tv_nsec);
                printk("elapsed time: %s.%s\n", seconds, nSeconds);

        }

        procfs_buf_len = strlen(currentTime);

        if (*ppos > 0 || count < procfs_buf_len)
                return 0;

        if (copy_to_user(ubuf, currentTime, procfs_buf_len))

                return -EFAULT;

        *ppos = procfs_buf_len;

        return procfs_buf_len;
}




static ssize_t procfile_write(struct file* file, const char * ubuf, size_t count, loff_t* ppos)
{
        return -1;
}


static struct file_operations procfile_fops = {
        .owner = THIS_MODULE,
        .read = procfile_read,
        .write = procfile_write,
};

static int timer_init(void)
{
        proc_entry = proc_create("timer", 0666, NULL, &procfile_fops);

        if (proc_entry == NULL)
                return -ENOMEM;

        return 0;
}

static void timer_exit(void)
{
        proc_remove(proc_entry);
        return;
}



module_init(timer_init);
module_exit(timer_exit);
