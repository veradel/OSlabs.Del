#include <linux/kernel.h>
#include <linux/module.h> 
#include <linux/printk.h>
#include <linux/init.h>

static int __init my_init(void) {
    pr_info("Welcome to the Tomsk State University\n");
    return 0;
}

static void __exit my_exit(void) {
    pr_info("Tomsk State University forever!\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
