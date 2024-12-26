#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/version.h>

MODULE_DESCRIPTION("Tomsk State University Kernel Module");
MODULE_AUTHOR("veradel");
MODULE_LICENSE("GPL");

#define PROCFS_NAME "tsulab"

static struct proc_dir_entry *proc_file = NULL;
static struct timespec64 start_time;

// Функция для вычисления количества часов
static long long hours_since_1054(void) {
    struct timespec64 current_time;
    long long diff_sec;
    long long hours;

    ktime_get_real_ts64(&current_time);

    diff_sec = current_time.tv_sec - start_time.tv_sec;
    hours = diff_sec / (60 * 60);

    return hours;
}

static ssize_t proc_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    char output[64];
    size_t output_len;
    long long hours;

    if (*offset > 0)
        return 0;

    hours = hours_since_1054();

    output_len = snprintf(output, sizeof(output), "%lld\n", hours);
    if (copy_to_user(buffer, output, output_len))
        return -EFAULT;

    *offset += output_len;
    return output_len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
};
#else
static const struct file_operations proc_file_ops = {
    .read = proc_read,
};
#endif

static int __init my_init(void) {
    struct tm start_tm;
    time64_t start_time_sec;

    pr_info("Welcome to the Tomsk State University\n");

    // Заполняем структуру tm для начальной даты
    start_tm.tm_year = 1054 - 1900;
    start_tm.tm_mon = 7 - 1; // Июль - 7й месяц
    start_tm.tm_mday = 4;
    start_tm.tm_hour = 0;
    start_tm.tm_min = 0;
    start_tm.tm_sec = 0;
    
    // Преобразуем tm в секунды
    start_time_sec = mktime64(start_tm.tm_year + 1900, start_tm.tm_mon + 1, start_tm.tm_mday,
                             start_tm.tm_hour, start_tm.tm_min, start_tm.tm_sec);

    start_time.tv_sec = start_time_sec;
    start_time.tv_nsec = 0;


    proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_ops);
    if (!proc_file) {
        pr_err("Failed to create /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);

    return 0;
}

static void __exit my_exit(void) {
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
    pr_info("Tomsk State University forever!\n");
}

module_init(my_init);
module_exit(my_exit);