#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

static struct proc_dir_entry* proc_file;

static ssize_t proc_read(struct file* file, char* buf, size_t count, loff_t* pos) {
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


static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .read = proc_read,
};


static int __init my_init(void) {
    pr_info("Добро пожаловать в Томский государственный университет\n");
    proc_file = proc_create("tsu_crab_nebula", 0, NULL, &proc_fops);
    if (proc_file == NULL) {
        pr_err("Ошибка при создании /proc/tsu_crab_nebula\n");
        return -ENOMEM;
    }
    return 0;
}


static void __exit my_exit(void) {
    pr_info("Томский государственный университет навсегда!\n");
    proc_remove(proc_file);
}

module_init(my_init);
module_exit(my_exit);