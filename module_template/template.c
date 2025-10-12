// SPDX-License-Identifier: GPL-2.0
/*
 * Generic Linux Kernel Module Template
 * 
 * This file serves as a template for creating Linux kernel modules.
 * Replace {{MODULE_NAME}} with your actual module name.
 * 
 * Author: Your Name
 * Email: your.email@example.com
 * Date: $(shell date)
 * Version: 1.0
 * License: GPL v2
 * 
 * Description: Brief description of what this module does
 * 
 * Compilation: make
 * Loading: sudo insmod {{MODULE_NAME}}.ko
 * Unloading: sudo rmmod {{MODULE_NAME}}
 * 
 * Parameters:
 *   debug - Enable debug output (default: 0)
 */

/* Define log format - must be before any includes */
#define pr_fmt(fmt) "{{MODULE_NAME}}: " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

/*
 * Module Information
 * These macros provide information about the module
 */
MODULE_LICENSE("GPL v2");                           /* License type */
MODULE_AUTHOR("Your Name <your.email@example.com>"); /* Module author */
MODULE_DESCRIPTION("{{MODULE_DESCRIPTION}}");       /* Brief description */
MODULE_VERSION("1.0");                             /* Module version */
MODULE_ALIAS("{{MODULE_NAME}}_alias");              /* Module alias */

/*
 * Module Parameters
 * These can be set when loading the module: insmod module.ko param=value
 */
static int debug = 0;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debug output (0=off, 1=on)");

/*
 * Module Global Variables
 */
static struct proc_dir_entry *proc_entry;
static char proc_buffer[1024];

/* 
 * Using standard kernel pr_*() functions with pr_fmt() for consistent logging
 * This approach integrates with Dynamic Debug system automatically
 * 
 * Available functions:
 * - pr_debug()  : Debug messages (controlled by Dynamic Debug)
 * - pr_info()   : Informational messages  
 * - pr_warn()   : Warning messages
 * - pr_err()    : Error messages
 * - pr_crit()   : Critical errors
 */

/*
 * Proc filesystem operations
 * These functions handle reads and writes to /proc/{{MODULE_NAME}}
 */

/* Called when someone reads from /proc/{{MODULE_NAME}} */
static ssize_t {{MODULE_NAME}}_proc_read(struct file *file, char __user *buffer, size_t count, loff_t *pos)
{
    int len;
    
    if (*pos > 0) {
        return 0; /* End of file */
    }
    
    len = snprintf(proc_buffer, sizeof(proc_buffer),
                   "{{MODULE_NAME}} module information:\n"
                   "Version: %s\n"
                   "Debug: %s\n",
                   "1.0",
                   debug ? "enabled" : "disabled");
    
    if (len > count) {
        len = count;
    }
    
    if (copy_to_user(buffer, proc_buffer, len)) {
        return -EFAULT;
    }
    
    *pos += len;
    pr_debug("proc_read: %d bytes read\n", len);
    
    return len;
}

/* Called when someone writes to /proc/{{MODULE_NAME}} */
static ssize_t {{MODULE_NAME}}_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char input[256];
    int len = count;
    
    if (len >= sizeof(input)) {
        len = sizeof(input) - 1;
    }
    
    if (copy_from_user(input, buffer, len)) {
        return -EFAULT;
    }
    
    input[len] = '\0';
    
    pr_info("Received write: %s", input);
    pr_debug("proc_write: %d bytes written\n", len);
    
    return count;
}

/* Proc file operations structure */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_fops = {
    .proc_read = {{MODULE_NAME}}_proc_read,
    .proc_write = {{MODULE_NAME}}_proc_write,
};
#else
static const struct file_operations proc_fops = {
    .read = {{MODULE_NAME}}_proc_read,
    .write = {{MODULE_NAME}}_proc_write,
};
#endif

/*
 * Helper Functions
 */


/*
 * Module Initialization Function
 * This function is called when the module is loaded into the kernel
 */
static int __init {{MODULE_NAME}}_init(void)
{   
    /* Print module loading message */
    pr_info("Loading module (version: 1.0)\n");
    pr_debug("Debug parameter: %d\n", debug);
    
    if (debug)
        pr_info("Debug mode enabled via module parameter\n");

    /* Create proc entry */
    proc_entry = proc_create("{{MODULE_NAME}}", 0666, NULL, &proc_fops);
    if (!proc_entry) {
        pr_err("Failed to create /proc/{{MODULE_NAME}}\n");
        return -ENOMEM;
    }
    
    pr_info("Created /proc/{{MODULE_NAME}} interface\n");
    pr_info("Module loaded successfully!\n");
    
    return 0; /* Success */
}

/*
 * Module Cleanup Function
 * This function is called when the module is removed from the kernel
 */
static void __exit {{MODULE_NAME}}_exit(void)
{
    pr_info("Unloading module...\n");
    
    /* Remove proc entry */
    if (proc_entry) {
        proc_remove(proc_entry);
        pr_debug("Removed /proc/{{MODULE_NAME}}\n");
    }
    
    pr_info("Module unloaded successfully!\n");
}

/*
 * Register module init and exit functions
 */
module_init({{MODULE_NAME}}_init);
module_exit({{MODULE_NAME}}_exit);

/*
 * Additional module metadata
 */

/* End of file */