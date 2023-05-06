#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>
#include <linux/types.h>
// #include "logger.h"
// #include "interceptor.h"

#include "../headers/interceptor.h"
#include "../headers/utils.h"
//#include "../headers/connectivity_thread.h"

static int __init initialize(void)
{
    if(initialise_interceptor()) clear_interceptor();
    return 0;
cleanup:
    
    return 0;
}

static void __exit uninstall(void)
{
    clear_interceptor();
    
    printk(KERN_INFO "Module extracted!\n");
}

module_init(initialize);
module_exit(uninstall);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Infectivity Test");
MODULE_DESCRIPTION("Infectivity Test");