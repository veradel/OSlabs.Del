#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

MODULE_DESCRIPTION("Tomsk State University Kernel Module");
MODULE_AUTHOR("Vera"); 

#define PROCFS_NAME "tsu_nebula"

static struct proc_dir_entry* proc_file = NULL;

static ssize_t proc_read(struct file *file, char  __user *buffer, size_t count, loff_t* pos) {
    char* msg = "Приблизительный возраст Крабовидной туманности (с 4 июля 1054 года):\n";
    static char output[256];
    if (*pos > 0) {
        return 0;
    }

    // Приближённый расчёт (игнорируем високосные года для простоты)
    long seconds_per_year = 365 × 24 × 60 * 60;
    long years = 2024 - 1054; // Приблизительный текущий год. Замените 2024 на функцию получения текущего года, если нужно.
    long total_seconds = years * seconds_per_year;
    long hours = total_seconds / (60 * 60);

    snprintf(output, sizeof(output), "%s%ld часов\n", msg, hours);
    *pos += strlen(output);

    return copy_to_user(buf, output, strlen(output)) ? -EFAULT : strlen(output);
}

// Определение структуры proc_ops или file_operations в зависимости от версии ядра
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read, // Устанавливаем функцию чтения
};
#else
static const struct file_operations proc_file_ops = {
    .read = proc_read, // Устанавливаем функцию чтения
};
#endif

static int __init my_init(void) {
    proc_file = proc_create("tsu_crab_nebula", 0, NULL, &proc_fops);
    if (proc_file == NULL) {
        pr_err("Ошибка при создании /proc/tsu_crab_nebula\n");
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}


static void __exit my_exit(void) {
    proc_remove(proc_file); 
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
