#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>

#define LOG_DEBUG(fmt, args...) my_debug_logger(__func__, fmt, ##args)
#define LOG_INFO(fmt, args...) my_info_logger(__func__, fmt, ##args)
#define LOG_WARNING(fmt, args...) my_warning_logger(__func__, fmt, ##args)
#define LOG_ERROR(fmt, args...) my_error_logger(__func__, fmt, ##args)

//#define LOG_FILE_PATH /var/log/module.log

void my_debug_logger(const char *func, const char *fmt, ...);

void my_info_logger(const char *func, const char *fmt, ...);

void my_warning_logger(const char *func, const char *fmt, ...);

void my_error_logger(const char *func, const char *fmt, ...);

// static int __init my_module_init(void);

// static void __exit my_module_exit(void);