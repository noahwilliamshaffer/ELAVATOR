#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

/* System call stub */
long (*STUB_issue_request)(int , int , int) = NULL;
EXPORT_SYMBOL(STUB_issue_request);

/* System call wrapper */
SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type) {
   printk(KERN_NOTICE "Inside SYSCALL_DEFINE3 block. %s: Your start_floor is %d, Your destination_floor is %d, " ,"issue_request", start_floor, destination_floor);
   printk(KERN_NOTICE "Your type is: %d%n", type);
    if (STUB_issue_request != NULL)
        return STUB_issue_request(start_floor, destination_floor, type);
    else
        return -ENOSYS;
}
